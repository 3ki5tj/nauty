/*****************************************************************************
*                                                                            *
*  Vertex-invariants source file for nauty 1.9.                              *
*                                                                            *
*   Copyright (1989-1993) Brendan McKay.  All rights reserved.               *
*   Subject to waivers and disclaimers in nauty.h.                           *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       13-Mar-90 : initial release for version 1.5                          *
*       10-Nov-90 : changes for version 1.6 :                                *
*                 - added dummy routine nautinv_null()                       *
*       27-Aug-92 : renamed to version 1.7, no changes to this file          *
*        5-Jun-93 : renamed to version 1.7+, no changes to this file         *
*       18-Aug-93 : renamed to version 1.8, no changes to this file          *
*       17-Sep-93 : changes for version 1.9 :                                *
*                 - added invariant routine adjacencies()                    *
*                                                                            *
*****************************************************************************/

#define  EXTDEFS 1
#include "naututil.h"

#if  MAXM==1
#define M 1
#else
#define M m
#endif

#define ACCUM(x,y)   x = (((x) + (y)) & 077777)

static int fuzz1[] = {037541,061532,005257,026416};
static int fuzz2[] = {006532,070236,035523,062437};

#define FUZZ1(x) ((x) ^ fuzz1[(x)&3])
#define FUZZ2(x) ((x) ^ fuzz2[(x)&3])

#define MAXCLIQUE 7     /* max clique size for cliques() */
#if MAXCLIQUE >= MAXN/2
**** error: MAXCLIQUE too big
#endif

#define MAXINDSET 7     /* max independent set size for indsets() */
#if MAXINDSET >= MAXN/2
**** error: MAXINDSET too big
#endif

static set workset[MAXM];             /* used for scratch work */
static short workshort[MAXN+2];       /* used for scratch work */

/*****************************************************************************
*                                                                            *
*  This file contains a number of procedures which compute vertex-invariants *
*  for stronger partition refinement.   Since entirely different             *
*  vertex-invariants seem to work better for different types of graph, we    *
*  cannot do more than give a small collection of representative examples.   *
*  Any serious computations with difficult graphs may well need to use       *
*  specially-written vertex-invariants.  The use of vertex-invariants        *
*  procedures is supported by nauty from version 1.5 onwards, via the        *
*  options userinvarproc, mininvarlevel, maxinvarlevel and invararg.         *
*  The meaning of these fields in detail are as follows:                     *
*     userinvarproc  is the address of the vertex-invariant procedure.  If   *
*                    no vertex-invariants is required, this field should     *
*                    have the value NILFUNCTION.                             *
*     maxinvarlevel  The absolute value of this is the maximum level in the  *
*                    search tree at which the vertex-invariant will be       *
*                    computed.  The root of the tree is at level 1, so the   *
*                    vertex-invariant will not be invoked at all if          *
*                    maxinvarlevel==0.  Negative values of maxinvarlevel     *
*                    request nauty to not compute the vertex-invariant at    *
*                    a level greater than that of the earliest node (if any) *
*                    on the path to the first leaf of the search tree at     *
*                    which the vertex-invariant refines the partition.       *
*     mininvarlevel  The absolute value of this is the minimum level in the  *
*                    search tree at which the vertex-invariant will be       *
*                    computed.  The root of the tree is at level 1, so there *
*                    is no effective limit if mininvarlevel is -1, 0 or 1.   *
*                    Negative values of mininvarlevel request nauty to not   *
*                    compute the vertex-invariant at a level less than       *
*                    that of the earliest node (if any) on the path to the   *
*                    first leaf of the search tree at which the              *
*                    vertex-invariant refines the partition.                 *
*     invararg       is passed to the vertex-invariant procedure via the     *
*                    argument of the same name.  It can be used by the       *
*                    procedure for any purpose.                              *
*                                                                            *
*  A vertex-invariant must be declared thus:                                 *
* UPROC invarproc(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n) *
*  All of these arguments must be treated as read-only except for invar.     *
*  g        : the graph, exactly as passed to nauty()                        *
*  lab,ptn  : the current partition nest (see nauty.h for the format)        *
*  level    : the level of this node in the search tree.                     *
*  numcells : the number of cells in the partition at this node.             *
*  tvpos    : the index in (lab,ptn) of one cell in the partition.           *
*             If level <= 1, the cell will be the first fragment of the      *
*             first active cell (as provided by the initial call to nauty),  *
*             or the first cell, if there were no active cells.              *
*             If level > 1, the cell will be the singleton cell which was    *
*             created to make this node of the search tree from its parent.  *
*  invararg : a copy of options.invararg                                     *
*  digraph  : a copy of options.digraph                                      *
*  m,n      : size parameters as passed to nauty()                           *
*  invar    : an array to return the answer in.   The procedure must put in  *
*             each invar[i]  (0 <= i < n)  an invariant of the 6-tuple       *
*             (<vertex i>,g,<the partition nest to this level>,level,        *
*               invararg,digraph)                                            *
*             Note that invar[] is declared as a permutation.  Since the     *
*             absolute value of the invariant is irrelevant, only the        *
*             comparative values, any short, int or long value can be        *
*             assigned to the entries of invar[] without fear.               *
*                                                                            *
*  The refinement procedure has already been called before the invariant     *
*  procedure is called.  That means that the partition is equitable if       *
*  digraph==FALSE.                                                           *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*  twopaths() assigns to each vertex v the sum of the weights of each vertex *
*  which can be reached from v along a walk of length two (including itself  *
*  usually).  The weight of each vertex w is defined as the ordinal number   *
*  of the cell containing w, starting at 1 for the first cell.               *
*                                                                            *
*****************************************************************************/

UPROC
twopaths(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,v,w;
        register short wt;
        set *gv,*gw;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = wt;
            if (ptn[i] <= level)
                ++wt;
        }

        for (v = 0, gv = (set*)g; v < n; ++v, gv += M)
        {
            EMPTYSET(workset,m);
            w = -1;
            while ((w = nextelement(gv,M,w)) >= 0)
            {
                gw = GRAPHROW(g,w,m);
                for (i = M; --i >= 0;)
                    UNION(workset[i],gw[i]);
            }
            wt = 0;
            w = -1;
            while ((w = nextelement(workset,M,w)) >= 0)
                ACCUM(wt,workshort[w]);
            invar[v] = wt;
        }
}

/*****************************************************************************
*                                                                            *
*  quadruples() assigns to each vertex v a value depending on the set of     *
*  weights w(v,v1,v2,v3), where w(v,v1,v2,v3) depends on the number of       *
*  vertices adjacent to an odd number of {v,v1,v2,v3}, and to the cells      *
*  that v,v1,v2,v3 belong to.  {v,v1,v2,v3} are permitted to range over all  *
*  distinct 4-tuples which contain at least one member in the cell tvpos.    *
*                                                                            *
*****************************************************************************/

UPROC
quadruples(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,pc;
        register setword sw;
        register set *gw;
        register short wt;
        int v,iv,v1,v2,v3;
        set *gv;
        set ws1[MAXM];
        long wv,wv1,wv2,wv3;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ2(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        iv = tvpos - 1;
        do
        {
             v = lab[++iv];
             gv = GRAPHROW(g,v,m);
             wv = workshort[v];
             for (v1 = 0; v1 < n-2; ++v1)
             {
                wv1 = workshort[v1];
                if (wv1 == wv && v1 <= v)
                    continue;
                wv1 += wv;
                gw = GRAPHROW(g,v1,m);
                for (i = M; --i >= 0;)
                     workset[i] = gv[i] ^ gw[i];
                for (v2 = v1+1; v2 < n-1; ++v2)
                {
                    wv2 = workshort[v2];
                    if (wv2 == wv && v2 <= v)
                        continue;
                    wv2 += wv1;
                    gw = GRAPHROW(g,v2,m);
                    for (i = M; --i >= 0;)
                        ws1[i] = workset[i] ^ gw[i];
                    for (v3 = v2+1; v3 < n; ++v3)
                    {
                        wv3 = workshort[v3];
                        if (wv3 == wv && v3 <= v)
                            continue;
                        wv3 += wv2;
                        gw = GRAPHROW(g,v3,m);
                        pc = 0;
                        for (i = M; --i >= 0;)
                            if (sw = ws1[i] ^ gw[i])          /* not == */
                                pc += POPCOUNT(sw);
                        wt = (FUZZ1(pc) + wv3) & 077777;
                        wt = FUZZ2(wt);
                        ACCUM(invar[v],wt);
                        ACCUM(invar[v1],wt);
                        ACCUM(invar[v2],wt);
                        ACCUM(invar[v3],wt);
                    }
                }
            }
        }
        while (ptn[iv] > level);
}

/*****************************************************************************
*                                                                            *
*  triples() assigns to each vertex v a value depending on the set of        *
*  weights w(v,v1,v2), where w(v,v1,v2) depends on the number of vertices    *
*  adjacent to an odd number of {v,v1,v2}, and to the cells that             *
*  v,v1,v2 belong to.  {v,v1,v2} are permitted to range over all distinct    *
*  triples which contain at least one member in the cell tvpos.              *
*                                                                            *
*****************************************************************************/

UPROC
triples(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,pc;
        register setword sw;
        register set *gw;
        register short wt;
        int v,iv,v1,v2;
        set *gv;
        long wv,wv1,wv2;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ1(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        iv = tvpos - 1;
        do
        {
             v = lab[++iv];
             gv = GRAPHROW(g,v,m);
             wv = workshort[v];
             for (v1 = 0; v1 < n-1; ++v1)
             {
                wv1 = workshort[v1];
                if (wv1 == wv && v1 <= v)
                    continue;
                wv1 += wv;
                gw = GRAPHROW(g,v1,m);
                for (i = M; --i >= 0;)
                     workset[i] = gv[i] ^ gw[i];
                for (v2 = v1+1; v2 < n; ++v2)
                {
                    wv2 = workshort[v2];
                    if (wv2 == wv && v2 <= v)
                        continue;
                    wv2 += wv1;
                    gw = GRAPHROW(g,v2,m);
                    pc = 0;
                    for (i = M; --i >= 0;)
                        if (sw = workset[i] ^ gw[i])            /* not == */
                            pc += POPCOUNT(sw);
                    wt = (FUZZ1(pc) + wv2) & 077777;
                    wt = FUZZ2(wt);
                    ACCUM(invar[v],wt);
                    ACCUM(invar[v1],wt);
                    ACCUM(invar[v2],wt);
                }
            }
        }
        while (ptn[iv] > level);
}

/*****************************************************************************
*                                                                            *
*  adjtriang() assigns to each vertex v a value depending on the numbers     *
*  of common neighbours between each pair {v1,v2} of neighbours of v, and    *
*  which cells v1 and v2 lie in.  The vertices v1 and v2 must be adjacent    *
*  if invararg == 0 and not adjacent if invararg == 1.                       *
*                                                                            *
*****************************************************************************/

UPROC
adjtriang(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int j,pc;
        register setword sw;
        register set *gi;
        register short wt;
        int i,v1,v2;
        boolean v1v2;
        set *gv1,*gv2;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ1(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        for (v1 = 0, gv1 = g; v1 < n; ++v1, gv1 += M)
        {
            for (v2 = (digraph ? 0 : v1+1); v2 < n; ++v2)
            {
                if (v2 == v1)
                    continue;
                v1v2 = (ISELEMENT(gv1,v2) != 0);
                if (invararg == 0 && !v1v2 || invararg == 1 && v1v2)
                    continue;
                wt = workshort[v1];
                ACCUM(wt,workshort[v2]);
                ACCUM(wt,v1v2);

                gv2 = GRAPHROW(g,v2,m);
                for (i = M; --i >= 0;)
                    workset[i] = gv1[i] & gv2[i];
                i = -1;
                while ((i = nextelement(workset,M,i)) >= 0)
                {
                    pc = 0;
                    gi = GRAPHROW(g,i,m);
                    for (j = M; --j >= 0;)
                        if (sw = workset[j] & gi[j])     /* not == */
                            pc += POPCOUNT(sw);
                    pc = (pc + wt) & 077777;
                    ACCUM(invar[i],pc);
                }
            }
        }
}

/*****************************************************************************
*                                                                            *
*  getbigcells(ptn,level,minsize,bigcells,cellstart,cellsize,n) is an        *
*  auxiliary procedure to make a list of all the large cells in the current  *
*  partition.  On entry, ptn, level and n have their usual meanings,         *
*  while minsize is the smallest size of an interesting cell.  On return,    *
*  bigcells is the number of cells of size at least minsize, cellstart[0...] *
*  contains their starting positions in ptn, and cellsize[0...] contain      *
*  their sizes.  These two arrays are in increasing order of cell size,      *
*  then position.                                                            *
*                                                                            *
*****************************************************************************/

void
getbigcells(ptn,level,minsize,bigcells,cellstart,cellsize,n)
nvector *ptn;
int level,minsize,*bigcells,n;
short *cellstart,*cellsize;
{
        register int cell1,cell2,j;
        register short si,st;
        int bc,i,h;

        bc = 0;
        for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
        {
            for (cell2 = cell1; ptn[cell2] > level; ++cell2)
            {}

            if (cell2 >= cell1 + 3)
            {
                cellstart[bc] = cell1;
                cellsize[bc] = cell2 - cell1 + 1;
                ++bc;
            }
        }
        *bigcells = bc;

        j = bc / 3;
        h = 1;
        do
            h = 3 * h + 1;
        while (h < j);

        do                      /* shell sort */
        {
            for (i = h; i < bc; ++i)
            {
                st = cellstart[i];
                si = cellsize[i];
                for (j = i; cellsize[j-h] > si ||
                            cellsize[j-h] == si && cellstart[j-h] > st; )
                {
                    cellsize[j] = cellsize[j-h];
                    cellstart[j] = cellstart[j-h];
                    if ((j -= h) < h)
                        break;
                }
                cellsize[j] = si;
                cellstart[j] = st;
            }
            h /= 3;
        }
        while (h > 0);
}

/*****************************************************************************
*                                                                            *
*  celltrips() assigns to each vertex v a value depending on the set of      *
*  weights w(v,v1,v2), where w(v,v1,v2) depends on the number of vertices    *
*  adjacent to an odd number of {v,v1,v2}.  {v,v1,v2} are  constrained to    *
*  belong to the same cell.  We try the cells in increasing order of size,   *
*  and stop as soon as any cell splits.                                      *
*                                                                            *
*****************************************************************************/

UPROC
celltrips(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,pc;
        register setword sw;
        register set *gw;
        register short wt;
        int v,iv,v1,iv1,v2,iv2;
        int icell,bigcells,cell1,cell2;
        short *cellstart,*cellsize;
        set *gv;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        cellstart = workshort;
        cellsize = workshort + (MAXN/2);
        getbigcells(ptn,level,3,&bigcells,cellstart,cellsize,n);

        for (icell = 0; icell < bigcells; ++icell)
        {
            cell1 = cellstart[icell];
            cell2 = cell1 + cellsize[icell] - 1;
            for (iv = cell1; iv <= cell2 - 2; ++iv)
            {
                v = lab[iv];
                gv = GRAPHROW(g,v,m);
                for (iv1 = iv + 1; iv1 <= cell2 - 1; ++iv1)
                {
                    v1 = lab[iv1];
                    gw = GRAPHROW(g,v1,m);
                    for (i = M; --i >= 0;)
                        workset[i] = gv[i] ^ gw[i];
                    for (iv2 = iv1 + 1; iv2 <= cell2; ++iv2)
                    {
                        v2 = lab[iv2];
                        gw = GRAPHROW(g,v2,m);
                        pc = 0;
                        for (i = M; --i >= 0;)
                            if (sw = workset[i] ^ gw[i])         /* not == */
                                pc += POPCOUNT(sw);
                        wt = FUZZ1(pc);
                        ACCUM(invar[v],wt);
                        ACCUM(invar[v1],wt);
                        ACCUM(invar[v2],wt);
                    }
                }
            }
            wt = invar[lab[cell1]];
            for (i = cell1 + 1; i <= cell2; ++i)
                if (invar[lab[i]] != wt)
                     return;
        }
}

/*****************************************************************************
*                                                                            *
*  cellquads() assigns to each vertex v a value depending on the set of      *
*  weights w(v,v1,v2,v3), where w(v,v1,v2,v3) depends on the number of       *
*  vertices adjacent to an odd number of {v,v1,v2,v3}.  {v,v1,v2,v3} are     *
*  constrained to belong to the same cell.  We try the cells in increasing   *
*  order of size, and stop as soon as any cell splits.                       *
*                                                                            *
*****************************************************************************/

UPROC
cellquads(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,pc;
        register setword sw;
        register set *gw;
        register short wt;
        int v,iv,v1,iv1,v2,iv2,v3,iv3;
        int icell,bigcells,cell1,cell2;
        short *cellstart,*cellsize;
        set *gv;
        set ws1[MAXM];

        for (i = n; --i >= 0;)
            invar[i] = 0;

        cellstart = workshort;
        cellsize = workshort + (MAXN/2);
        getbigcells(ptn,level,4,&bigcells,cellstart,cellsize,n);

        for (icell = 0; icell < bigcells; ++icell)
        {
            cell1 = cellstart[icell];
            cell2 = cell1 + cellsize[icell] - 1;
            for (iv = cell1; iv <= cell2 - 3; ++iv)
            {
                v = lab[iv];
                gv = GRAPHROW(g,v,m);
                for (iv1 = iv + 1; iv1 <= cell2 - 2; ++iv1)
                {
                    v1 = lab[iv1];
                    gw = GRAPHROW(g,v1,m);
                    for (i = M; --i >= 0;)
                         workset[i] = gv[i] ^ gw[i];
                    for (iv2 = iv1 + 1; iv2 <= cell2 - 1; ++iv2)
                    {
                        v2 = lab[iv2];
                        gw = GRAPHROW(g,v2,m);
                        for (i = M; --i >= 0;)
                            ws1[i] = workset[i] ^ gw[i];
                        for (iv3 = iv2 + 1; iv3 <= cell2; ++iv3)
                        {
                            v3 = lab[iv3];
                            gw = GRAPHROW(g,v3,m);
                            pc = 0;
                            for (i = M; --i >= 0;)
                                if (sw = ws1[i] ^ gw[i])          /* not == */
                                    pc += POPCOUNT(sw);
                            wt = FUZZ1(pc);
                            ACCUM(invar[v],wt);
                            ACCUM(invar[v1],wt);
                            ACCUM(invar[v2],wt);
                            ACCUM(invar[v3],wt);
                        }
                    }
                }
            }
            wt = invar[lab[cell1]];
            for (i = cell1 + 1; i <= cell2; ++i)
                if (invar[lab[i]] != wt)
                     return;
        }
}

/*****************************************************************************
*                                                                            *
*  cellquins() assigns to each vertex v a value depending on the set of      *
*  weights w(v,v1,v2,v3,v4), where w(v,v1,v2,v3,v4) depends on the number    *
*  of vertices adjacent to an odd number of {v,v1,v2,v3,v4}.                 *
*  {v,v1,v2,v3,v4} are constrained to belong to the same cell.  We try the   *
*  cells in increasing order of size, and stop as soon as any cell splits.   *
*                                                                            *
*****************************************************************************/

UPROC
cellquins(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,pc;
        register setword sw;
        register set *gw;
        register short wt;
        int v,iv,v1,iv1,v2,iv2,v3,iv3,v4,iv4;
        int icell,bigcells,cell1,cell2;
        short *cellstart,*cellsize;
        set *gv;
        set ws1[MAXM],ws2[MAXM];

        for (i = n; --i >= 0;)
            invar[i] = 0;

        cellstart = workshort;
        cellsize = workshort + (MAXN/2);
        getbigcells(ptn,level,5,&bigcells,cellstart,cellsize,n);

        for (icell = 0; icell < bigcells; ++icell)
        {
            cell1 = cellstart[icell];
            cell2 = cell1 + cellsize[icell] - 1;
            for (iv = cell1; iv <= cell2 - 4; ++iv)
            {
                v = lab[iv];
                gv = GRAPHROW(g,v,m);
                for (iv1 = iv + 1; iv1 <= cell2 - 3; ++iv1)
                {
                    v1 = lab[iv1];
                    gw = GRAPHROW(g,v1,m);
                    for (i = M; --i >= 0;)
                         workset[i] = gv[i] ^ gw[i];
                    for (iv2 = iv1 + 1; iv2 <= cell2 - 2; ++iv2)
                    {
                        v2 = lab[iv2];
                        gw = GRAPHROW(g,v2,m);
                        for (i = M; --i >= 0;)
                            ws1[i] = workset[i] ^ gw[i];
                        for (iv3 = iv2 + 1; iv3 <= cell2 - 1; ++iv3)
                        {
                            v3 = lab[iv3];
                            gw = GRAPHROW(g,v3,m);
                            for (i = M; --i >= 0;)
                                ws2[i] = ws1[i] ^ gw[i];
                            for (iv4 = iv3 + 1; iv4 <= cell2; ++iv4)
                            {
                                v4 = lab[iv4];
                                gw = GRAPHROW(g,v4,m);
                                pc = 0;
                                for (i = M; --i >= 0;)
                                    if (sw = ws2[i] ^ gw[i])      /* not == */
                                        pc += POPCOUNT(sw);
                                wt = FUZZ1(pc);
                                ACCUM(invar[v],wt);
                                ACCUM(invar[v1],wt);
                                ACCUM(invar[v2],wt);
                                ACCUM(invar[v3],wt);
                                ACCUM(invar[v4],wt);
                            }
                        }
                    }
                }
            }
            wt = invar[lab[cell1]];
            for (i = cell1 + 1; i <= cell2; ++i)
                if (invar[lab[i]] != wt)
                     return;
        }
}

/*****************************************************************************
*                                                                            *
*  distances() assigns to each vertex v a value depending on the number of   *
*  vertices at each distance from v, and what cells they lie in.             *
*  If we find any cell which is split in this manner, we don't try any       *
*  further cells.                                                            *
*                                                                            *
*****************************************************************************/

UPROC
distances(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i;
        register set *gw;
        register short wt;
        int d,cell1,cell2,iv,v,w;
        boolean success;
        set sofar[MAXM],frontier[MAXM];

        for (i = n; --i >= 0;)
            invar[i] = 0;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ1(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        success = FALSE;
        for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
        {
            for (cell2 = cell1; ptn[cell2] > level; ++cell2)
            {}
            if (cell2 == cell1)
                continue;

            for (iv = cell1; iv <= cell2; ++iv)
            {
                v = lab[iv];
                EMPTYSET(sofar,m);
                ADDELEMENT(sofar,v);
                EMPTYSET(frontier,m);
                ADDELEMENT(frontier,v);
                for (d = 1; d < n; ++d)
                {
                    EMPTYSET(workset,m);
                    wt = 0;
                    w = -1;
                    while ((w = nextelement(frontier,M,w)) >= 0)
                    {
                        gw = GRAPHROW(g,w,m);
                        ACCUM(wt,workshort[w]);
                        for (i = M; --i >= 0;)
                            workset[i] |= gw[i];
                    }
                    if (wt == 0)
                        break;
                    ACCUM(wt,d);
                    wt = FUZZ2(wt);
                    ACCUM(invar[v],wt);
                    for (i = M; --i >= 0;)
                    {
                        frontier[i] = workset[i] & ~sofar[i];
                        sofar[i] |= frontier[i];
                    }
                }
                if (invar[v] != invar[lab[cell1]])
                    success = TRUE;
            }
            if (success)
                break;
        }
}

/*****************************************************************************
*                                                                            *
*  indsets() assigns to each vertex v a value depending on which cells the   *
*  vertices which join v in an independent set lie in.  The size of the      *
*  independent sets which are used is the largest of invararg and MAXINDSET. *
*                                                                            *
*****************************************************************************/

UPROC
indsets(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i;
        register short wt;
        register set *gv;
        int ss,setsize;
        int v[MAXINDSET];
        long wv[MAXINDSET];
        set s[MAXINDSET-1][MAXM],*s0;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        if (invararg <= 1 || digraph)
            return;

        if (invararg > MAXINDSET)
            setsize = MAXINDSET;
        else
            setsize = invararg;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ2(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        for (v[0] = 0; v[0] < n; ++v[0])
        {
            wv[0] = workshort[v[0]];
            s0 = (set*)s[0];
            EMPTYSET(s0,m);
            for (i = v[0]+1; i < n; ++i)
                ADDELEMENT(s0,i);
            gv = GRAPHROW(g,v[0],m);
            for (i = M; --i >= 0;)
                s0[i] &= ~gv[i];
            ss = 1;
            v[1] = v[0];
            while (ss > 0)
            {
                if (ss == setsize)
                {
                    wt = FUZZ1(wv[ss-1]);
                    for (i = ss; --i >= 0;)
                        ACCUM(invar[v[i]],wt);
                    --ss;
                }
                else if ((v[ss] = nextelement(s[ss-1],M,v[ss])) < 0)
                    --ss;
                else
                {
                    wv[ss] = wv[ss-1] + workshort[v[ss]];
                    ++ss;
                    if (ss < setsize)
                    {
                        gv = GRAPHROW(g,v[ss-1],m);
                        for (i = M; --i >= 0;)
                            s[ss-1][i] = s[ss-2][i] & ~gv[i];
                        v[ss] = v[ss-1];
                    }
                }
            }
        }
}

/*****************************************************************************
*                                                                            *
*  cliques() assigns to each vertex v a value depending on which cells the   *
*  vertices which join v in a clique lie in.  The size of the cliques used   *
*  is the largest of invararg and MAXCLIQUE.                                 *
*                                                                            *
*****************************************************************************/

UPROC
cliques(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i;
        register short wt;
        register set *gv;
        int ss,setsize;
        int v[MAXCLIQUE];
        long wv[MAXCLIQUE];
        set nset[MAXCLIQUE-1][MAXM];

        for (i = n; --i >= 0;)
            invar[i] = 0;

        if (invararg <= 1 || digraph)
            return;

        if (invararg > MAXCLIQUE)
            setsize = MAXCLIQUE;
        else
            setsize = invararg;

        wt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = FUZZ2(wt);
            if (ptn[i] <= level)
                ++wt;
        }

        for (v[0] = 0; v[0] < n; ++v[0])
        {
            wv[0] = workshort[v[0]];
            gv = GRAPHROW(g,v[0],m);
            for (i = M; --i >= 0;)
                nset[0][i] = gv[i];
            ss = 1;
            v[1] = v[0];
            while (ss > 0)
            {
                if (ss == setsize)
                {
                    wt = FUZZ1(wv[ss-1]);
                    for (i = ss; --i >= 0;)
                        ACCUM(invar[v[i]],wt);
                    --ss;
                }
                else if ((v[ss] = nextelement(nset[ss-1],M,v[ss])) < 0)
                    --ss;
                else
                {
                    wv[ss] = wv[ss-1] + workshort[v[ss]];
                    ++ss;
                    if (ss < setsize)
                    {
                        gv = GRAPHROW(g,v[ss-1],m);
                        for (i = M; --i >= 0;)
                            nset[ss-1][i] = nset[ss-2][i] & gv[i];
                        v[ss] = v[ss-1];
                    }
                }
            }
        }
}

/*****************************************************************************
*                                                                            *
*  cellcliq() assigns to each vertex v a value depending on the number of    *
*  cliques which v lies in and which lie in the same cell as v.  The size    *
*  of clique counted is the largest of invararg and MAXCLIQUE.  We try the   *
*  cells in increasing order of size and stop as soon as any cell splits.    *
*                                                                            *
*****************************************************************************/

UPROC
cellcliq(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i;
        register short wt;
        register set *gv;
        int ss,setsize;
        int v[MAXCLIQUE];
        set nset[MAXCLIQUE-1][MAXM];
        short *cellstart,*cellsize;
        int iv,icell,bigcells,cell1,cell2;
        int pc;
        setword sw;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        if (invararg <= 1 || digraph)
            return;

        if (invararg > MAXCLIQUE)
            setsize = MAXCLIQUE;
        else
            setsize = invararg;

        cellstart = workshort;
        cellsize = workshort + (MAXN/2);
        getbigcells(ptn,level,setsize > 6 ? setsize : 6,&bigcells,
                    cellstart,cellsize,n);

        for (icell = 0; icell < bigcells; ++icell)
        {
            cell1 = cellstart[icell];
            cell2 = cell1 + cellsize[icell] - 1;

            EMPTYSET(workset,m);
            for (iv = cell1; iv <= cell2; ++iv)
                ADDELEMENT(workset,lab[iv]);

            for (iv = cell1; iv <= cell2; ++iv)
            {
                v[0] = lab[iv];
                gv = GRAPHROW(g,v[0],m);
                pc = 0;

                for (i = M; --i >= 0;)
                {
                    nset[0][i] = gv[i] & workset[i];
                    if (sw = nset[0][i])                /*  not == */
                        pc += POPCOUNT(sw);
                }
                if (pc <= 1 || pc >= cellsize[icell] - 2)
                    continue;

                ss = 1;
                v[1] = v[0];
                while (ss > 0)
                {
                    if (ss == setsize)
                    {
                        for (i = ss; --i >= 0;)
                            ++invar[v[i]];
                        --ss;
                    }
                    else if ((v[ss] = nextelement(nset[ss-1],M,v[ss])) < 0)
                        --ss;
                    else
                    {
                        ++ss;
                        if (ss < setsize)
                        {
                            gv = GRAPHROW(g,v[ss-1],m);
                            for (i = M; --i >= 0;)
                                nset[ss-1][i] = nset[ss-2][i] & gv[i];
                            v[ss] = v[ss-1];
                        }
                    }
                }
            }
            wt = invar[lab[cell1]];
            for (iv = cell1; iv <= cell2; ++iv)
                if (invar[lab[i]] != wt)
                    return;
        }
}

/*****************************************************************************
*                                                                            *
*  cellind() assigns to each vertex v a value depending on the number of     *
*  independent sets which v lies in and which lie in the same cell as v.     *
*  The size of clique counted is the largest of invararg and MAXCLIQUE.      *
*  We try the cells in increasing order of size and stop as soon as any      *
*  cell splits.                                                              *
*                                                                            *
*****************************************************************************/

UPROC
cellind(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i;
        register short wt;
        register set *gv;
        int ss,setsize;
        int v[MAXCLIQUE];
        set nset[MAXCLIQUE-1][MAXM];
        short *cellstart,*cellsize;
        int iv,icell,bigcells,cell1,cell2;
        int pc;
        setword sw;

        for (i = n; --i >= 0;)
            invar[i] = 0;

        if (invararg <= 1 || digraph)
            return;

        if (invararg > MAXCLIQUE)
            setsize = MAXCLIQUE;
        else
            setsize = invararg;

        cellstart = workshort;
        cellsize = workshort + (MAXN/2);
        getbigcells(ptn,level,setsize > 6 ? setsize : 6,&bigcells,
                    cellstart,cellsize,n);

        for (icell = 0; icell < bigcells; ++icell)
        {
            cell1 = cellstart[icell];
            cell2 = cell1 + cellsize[icell] - 1;

            EMPTYSET(workset,m);
            for (iv = cell1; iv <= cell2; ++iv)
                ADDELEMENT(workset,lab[iv]);

            for (iv = cell1; iv <= cell2; ++iv)
            {
                v[0] = lab[iv];
                gv = GRAPHROW(g,v[0],m);
                pc = 0;

                for (i = M; --i >= 0;)
                {
                    nset[0][i] = ~gv[i] & workset[i];
                    if (sw = nset[0][i])                /*  not == */
                        pc += POPCOUNT(sw);
                }
                if (pc <= 1 || pc >= cellsize[icell] - 2)
                    continue;

                ss = 1;
                v[1] = v[0];
                while (ss > 0)
                {
                    if (ss == setsize)
                    {
                        for (i = ss; --i >= 0;)
                            ++invar[v[i]];
                        --ss;
                    }
                    else if ((v[ss] = nextelement(nset[ss-1],M,v[ss])) < 0)
                        --ss;
                    else
                    {
                        ++ss;
                        if (ss < setsize)
                        {
                            gv = GRAPHROW(g,v[ss-1],m);
                            for (i = M; --i >= 0;)
                                nset[ss-1][i] = nset[ss-2][i] & ~gv[i];
                            v[ss] = v[ss-1];
                        }
                    }
                }
            }
            wt = invar[lab[cell1]];
            for (iv = cell1; iv <= cell2; ++iv)
                if (invar[lab[i]] != wt)
                    return;
        }
}

/*****************************************************************************
*                                                                            *
*  adjacencies() assigns to each vertex v a code depending on which cells    *
*  it is joined to and from, and how many times.  It is intended to provide  *
*  better partitioning that the normal refinement routine for digraphs.      *
*                                                                            *
*****************************************************************************/

UPROC
adjacencies(g,lab,ptn,level,numcells,tvpos,invar,invararg,digraph,m,n)
graph *g;
nvector *lab,*ptn;
int level,numcells,tvpos,invararg,m,n;
permutation *invar;
boolean digraph;
{
        register int i,v,w;
        register short vwt,wwt;
        set *gv,*gw;

        vwt = 1;
        for (i = 0; i < n; ++i)
        {
            workshort[lab[i]] = vwt;
            if (ptn[i] <= level)
                ++vwt;
            invar[i] = 0;
        }

        for (v = 0, gv = (set*)g; v < n; ++v, gv += M)
        {
            vwt = FUZZ1(workshort[v]);
            wwt = 0;
            w = -1;
            while ((w = nextelement(gv,M,w)) >= 0)
            {
                ACCUM(wwt,FUZZ2(workshort[w]));
                ACCUM(invar[w],vwt);
            }
            ACCUM(invar[v],wwt);
        }
}

/*****************************************************************************
*                                                                            *
*  nautinv_null() does nothing.  See dreadnaut.c for its purpose.            *
*                                                                            *
*****************************************************************************/

void
nautinv_null()
{
}
