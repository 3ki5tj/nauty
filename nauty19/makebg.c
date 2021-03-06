/* makebg.c : Find all bicoloured graphs with a given number of vertices
             of each colour, a given range of numbers of edges, and a
             given bound on the maximum degree.

             The output is to stdout in either y format or nauty format.
             The output graphs are optionally canonically labelled.

 Usage: makebg [-c -n -u -v -l] [-d<max>] n1 n2 [mine [maxe [mod res]]]

             n1   = the number of vertices in the first class (1..16)
             n2   = the number of vertices in the second class (1..16)
             mine = the minimum number of edges (no bounds if missing)
             maxe = the maximum number of edges (same as mine if missing)
             mod, res = a way to restrict the output to a subset.
                        All the graphs in G(n,mine..maxe) are divided into
                        disjoint classes C(mod,0),C(mod,1),...,C(mod,mod-1),
                        of very approximately equal size.
                        Only the class C(mod,res) is written.
                        The usual relationships between modulo classes are
                        obeyed; for example C(4,3) = C(8,3) union C(8,7).
             -c    : only write connected graphs
             -d<x> : specify an upper bound for the maximum degree.
                     The value of the upper bound must be adjacent to
                     the "d".  Example: -d6
                     You can also give separate maxima for the two parts,
                     for example: -d5:6
             -n    : use nauty format instead of y format for output
             -u    : do not output any graphs, just generate and count them
             -v    : display counts by number of edges to stderr
             -l    : canonically label output graphs
                     (with no attention paid to the colouring)

Output formats.

  If -n is absent, any output graphs are written in y format.

    Each graph occupies one line with a terminating newline.
    Except for the newline, each byte has the format  01xxxxxx, where
    each "x" represents one bit of data.
    First byte:  xxxxxx is the number of vertices n
    Other ceiling(n(n-1)/12) bytes:  These contain the upper triangle of
    the adjacency matrix in column major order.  That is, the entries
    appear in the order (0,1),(0,2),(1,2),(0,3),(1,3),(2,3),(0,4),... .
    The bits are used in left to right order within each byte.
    Any unused bits on the end are set to zero.

  If -n is present, any output graphs are written in nauty format.

    For a graph of n vertices, the output consists of n+1 long ints
    (even if a setword is shorter than a long int). The first contains
    n, and the others contain the adjacency matrix.  Long int i of
    the adjacency matrix (0 <= i <= n-1) is the set of neighbours of
    vertex i, cast from setword to long int.

OUTPROC feature.

   By defining the C preprocessor variable OUTPROC at compile time
   (for Unix the syntax is  -DOUTPROC=procname  on the cc command),
   makebg can be made to call a procedure of your manufacture with each
   output graph instead of writing anything.  Your procedure needs to
   have type void and the argument list  (FILE *f, graph *g, int n).
   f is a stream open for writing (in fact, in the current version it
   is always stdout), g is the graph in nauty format, and n is the number
   of vertices.  Your procedure can be in a separate file so long as it
   is linked with makebg.  The global variables nooutput, nautyformat
   and canonise (all type boolean) can be used to test for the presence
   of the flags -u, -n and -l, respectively.  The size of the first
   colour class can be found in the global int variable class1size;

INSTRUMENT feature.

    If the C preprocessor variable INSTRUMENT is defined at compile time,
    extra code is inserted to collect statistics during execution, and
    more information is written to stderr at termination.

**************************************************************************

    Author:   B. D. McKay, Oct 1994.
              Copyright  B. McKay (1994).  All rights reserved.
              This software is subject to the conditions and waivers
              detailed in the file nauty.h.

    Changes:

**************************************************************************/

#define MAXN 32         /* not more than WORDSIZE */
#define MAXN1 16        /* not more than 16 */
#include "naututil.h"   /* which includes nauty.h and stdio.h */

CPUDEFS

static void (*outproc)();
#ifdef OUTPROC
extern void OUTPROC();
#endif

static FILE *outfile;           /* file for output graphs */
static FILE *msgfile;           /* file for messages */
static boolean connec;          /* presence of -c */
static boolean verbose;         /* presence of -v */
boolean nautyformat;            /* presence of -n */
boolean nooutput;               /* presence of -u */
boolean canonise;               /* presence of -l */
int class1size;                 /* same as n1 */
static int maxdeg1,maxdeg2,n1,maxn2,mine,maxe,nprune,mod,res,curres;
static graph gcan[MAXN];

static int xbit[] = {0x0001,0x0002,0x0004,0x0008,
                     0x0010,0x0020,0x0040,0x0080,
                     0x0100,0x0200,0x0400,0x0800,
                     0x1000,0x2000,0x4000,0x8000};

#define XFIRSTBIT(x) \
    ((x)&0xFF ? 7-leftbit[(x)&0xFF] : 15-leftbit[((x)>>8)&0xFF])
#define XPOPCOUNT(x) (bytecount[((x)>>8)&0xFF] + bytecount[(x)&0xFF])

typedef struct
{
    int ne,dmax;         /* values used for xlb,xub calculation */
    int xlb,xub;         /* saved bounds on extension degree */
    int lo,hi;           /* work purposes for orbit calculation */
    int *xorb;           /* min orbit representative */
} leveldata;

static leveldata data[MAXN];      /* data[n] is data for n -> n+1 */
static long count[1+MAXN*(MAXN-1)/2];  /* counts by number of edges */
static int xstart[MAXN+1];  /* index into xset[] for each cardinality */
static int *xset;           /* array of all x-sets in card order */
static int *xcard;          /* cardinalities of all x-sets */
static int *xinv;           /* map from x-set to index in xset */

#ifdef INSTRUMENT
static long nodes[MAXN],rigidnodes[MAXN],fertilenodes[MAXN];
static long a1calls,a1nauty,a1succs;
static long a2calls,a2nauty,a2uniq,a2succs;
#endif

/************************************************************************/

void
writeny(f,g,n)       /* write graph g (n vertices) to file f in y format */
FILE *f;
graph *g;
int n;
{
        static char ybit[] = {32,16,8,4,2,1};
        char s[(MAXN*(MAXN-1)/2 + 5)/6 + 4];
        register int i,j,k;
        register char y,*sp;

        sp = s;
        *(sp++) = 0x40 | n;
        y = 0x40;

        k = -1;
        for (j = 1; j < n; ++j)
        for (i = 0; i < j; ++i)
        {
            if (++k == 6)
            {
                *(sp++) = y;
                y = 0x40;
                k = 0;
            }
            if (g[i] & bit[j]) y |= ybit[k];
        }
        if (n >= 2) *(sp++) = y;
        *(sp++) = '\n';
        *sp = '\0';

        if (fputs(s,f) == EOF || ferror(f))
        {
            fprintf(stderr,">E writeny : error on writing file\n");
            exit(2);
        }
}

/***********************************************************************/

static void
nullwrite(f,g,n)  /* don't write graph g (n vertices) to file f */
FILE *f;
graph *g;
int n;
{
}

/***********************************************************************/

void
writenauty(f,g,n)  /* write graph g (n vertices) to file f in nauty format */
FILE *f;
graph *g;
int n;
{
        long buffer[MAXN+1];
        register int i;

        buffer[0] = n;
        for (i = 0; i < n; ++i)
            buffer[i+1] = g[i];

        if (fwrite((char*)buffer,sizeof(long),n+1,f) != n+1)
        {
            fprintf(stderr,">E writenauty : error on writing file\n");
            exit(2);
        }
}

/*********************************************************************/

static boolean
isconnected(g,n)             /* test if g is connected */
graph *g;
int n;
{
        register setword seen,expanded,toexpand;
        register int i;

        seen = bit[0];
        expanded = 0;

        while (toexpand = (seen & ~expanded))             /* not == */
        {
            i = FIRSTBIT(toexpand);
            expanded |= bit[i];
            seen |= g[i];
        }

        return  POPCOUNT(seen) == n;
}

/**************************************************************************/

static boolean
distinvar(g,invar,n1,n2)   /* make distance invariant */
graph *g;                  /* exit immediately FALSE if n-1 not maximal */
int *invar,n1,n2;          /* else exit TRUE */
{                          /* Note: only invar[n1..n1+n2-1] set */
        register int w,n;
        register setword workset,frontier;
        setword sofar;
        int inv,d,v;

        n = n1 + n2;
        for (v = n-1; v >= n1; --v)
        {
            inv = 0;
            sofar = frontier = bit[v];
            for (d = 1; frontier != 0; ++d)
            {
                workset = 0;
                inv += POPCOUNT(frontier) ^ (0x57 + d);
                while (frontier)
                {
                    w = FIRSTBIT(frontier);
                    frontier &= ~bit[w];
                    workset |= g[w];
                }
                frontier = workset & ~sofar;
                sofar |= frontier;
            }
            invar[v] = inv;
            if (v < n-1 && inv > invar[n-1]) return FALSE;
        }
        return TRUE;
}

/**************************************************************************/

static void
makeleveldata()      /* make the level data for each level */
{
        register int i,j,h;
        int nn,nxsets,tttn;
        long ncj;
        leveldata *d;
        int xw,cw;
        extern char *malloc();

        nn = maxdeg2 <= n1 ? maxdeg2 : n1;
        ncj = nxsets = 1;
        for (j = 1; j <= nn; ++j)
        {
            ncj = (ncj * (n1 - j + 1)) / j;
            nxsets += ncj;
        }

        tttn = 1 << n1;
        xset = (int*) malloc(nxsets * sizeof(int));
        xcard = (int*) malloc(nxsets * sizeof(int));
        xinv = (int*) malloc(tttn * sizeof(int));
        if (xset==NULL || xcard==NULL || xinv==NULL)
        {
            fprintf(stderr,">E makebg: malloc failed in makeleveldata()\n");
            exit(2);
        }

        j = 0;
        for (i = 0; i < tttn; ++i)
            if ((h = XPOPCOUNT(i)) <= maxdeg2)
            {
                xset[j] = i;
                xcard[j] = h;
                ++j;
            }

        if (j != nxsets)
        {
            fprintf(stderr,">E makebg: j=%d mxsets=%d\n",j,nxsets);
            exit(2);
        }

        h = 1;
        do
            h = 3 * h + 1;
        while (h < nxsets);

        do
        {
            for (i = h; i < nxsets; ++i)
            {
                xw = xset[i];
                cw = xcard[i];
                for (j = i; xcard[j-h] > cw ||
                            xcard[j-h] == cw && xset[j-h] > xw; )
                {
                    xset[j] = xset[j-h];
                    xcard[j] = xcard[j-h];
                    if ((j -= h) < h) break;
                }
                xset[j] = xw;
                xcard[j] = cw;
            }
            h /= 3;
        }
        while (h > 0);

        for (i = 0; i < nxsets; ++i)
            xinv[xset[i]] = i;

        xstart[0] = 0;
        for (i = 1; i < nxsets; ++i)
            if (xcard[i] > xcard[i-1]) xstart[xcard[i]] = i;
        xstart[xcard[nxsets-1]+1] = nxsets;

        for (i = 0; i < maxn2; ++i)
        {

            d = &data[i];

            d->xorb = (int*) malloc(nxsets * sizeof(int));

            if (d->xorb==NULL)
            {
                fprintf(stderr,">E makebg: malloc failed in makeleveldata()\n");
                exit(2);
            }

            d->ne = d->dmax = d->xlb = d->xub = -1;
        }
}

/**************************************************************************/

static UPROC
userautomproc(count,p,orbits,numorbits,stabvertex,n)
int count,numorbits,stabvertex,n;   /* form orbits on powerset of VG */
permutation *p;                     /* called by nauty */
nvector *orbits;                    /* operates on data[n-n1] */
{
        register int i,j1,j2;
        register int moved,pxi,pi;
        int w,lo,hi;
        register int *xorb;

        xorb = data[n-n1].xorb;
        lo = data[n-n1].lo;
        hi = data[n-n1].hi;

        if (count == 1)                         /* first automorphism */
            for (i = lo; i < hi; ++i)
                xorb[i] = i;

        moved = 0;
        for (i = 0; i < n; ++i)
            if (p[i] != i) moved |= xbit[i];

        for (i = lo; i < hi; ++i)
        {
            if ((w = xset[i] & moved) == 0) continue;
            pxi = xset[i] & ~moved;
            while (w)
            {
                j1 = XFIRSTBIT(w);
                w &= ~xbit[j1];
                pxi |= xbit[p[j1]];
            }
            pi = xinv[pxi];

            j1 = xorb[i];
            while (xorb[j1] != j1)
                j1 = xorb[j1];
            j2 = xorb[pi];
            while (xorb[j2] != j2)
                j2 = xorb[j2];

            if      (j1 < j2) xorb[j2] = xorb[i] = xorb[pi] = j1;
            else if (j1 > j2) xorb[j1] = xorb[i] = xorb[pi] = j2;
        }
}

/*****************************************************************************
*                                                                            *
*  refinex(g,lab,ptn,level,numcells,count,active,goodret,code,m,n) is a      *
*  custom version of refine() which can exit quickly if required.            *
*                                                                            *
*  Only use at level==0.                                                     *
*  goodret : whether to do an early return for code 1                        *
*  code := -1 for n-1 not max, 0 for maybe, 1 for definite                   *
*                                                                            *
*****************************************************************************/

static void
refinex(g,lab,ptn,level,numcells,count,active,goodret,code,m,n)
graph *g;
register nvector *lab,*ptn;
permutation *count;
int *numcells,level,m,n,*code;
set *active;
boolean goodret;
{
        register int i,c1,c2,labc1;
        register setword x;
        int split1,split2,cell1,cell2;
        int cnt,bmin,bmax;
        set *gptr;
        setword workset;
        int workperm[MAXN];
        int bucket[MAXN+2];

        if (n == 1)
        {
            *code = 1;
            return;
        }

        *code = 0;
        split1 = -1;
        while (*numcells < n && ((split1 = nextelement(active,1,split1)) >= 0
                             || (split1 = nextelement(active,1,-1)) >= 0))
        {
            DELELEM1(active,split1);
            for (split2 = split1; ptn[split2] > 0; ++split2)
            {}
            if (split1 == split2)       /* trivial splitting cell */
            {
                gptr = GRAPHROW(g,lab[split1],1);
                for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
                {
                    for (cell2 = cell1; ptn[cell2] > 0; ++cell2)
                    {}
                    if (cell1 == cell2)
                        continue;
                    c1 = cell1;
                    c2 = cell2;
                    while (c1 <= c2)
                    {
                        labc1 = lab[c1];
                        if (ISELEM1(gptr,labc1))
                            ++c1;
                        else
                        {
                            lab[c1] = lab[c2];
                            lab[c2] = labc1;
                            --c2;
                        }
                    }
                    if (c2 >= cell1 && c1 <= cell2)
                    {
                        ptn[c2] = 0;
                        ++*numcells;
                        ADDELEM1(active,c1);
                    }
                }
            }

            else        /* nontrivial splitting cell */
            {
                workset = 0;
                for (i = split1; i <= split2; ++i)
                    workset |= bit[lab[i]];

                for (cell1 = 0; cell1 < n; cell1 = cell2 + 1)
                {
                    for (cell2 = cell1; ptn[cell2] > 0; ++cell2)
                    {}
                    if (cell1 == cell2)
                        continue;
                    i = cell1;
                    if (x = workset & g[lab[i]])     /* not == */
                        cnt = POPCOUNT(x);
                    else
                        cnt = 0;
                    count[i] = bmin = bmax = cnt;
                    bucket[cnt] = 1;
                    while (++i <= cell2)
                    {
                        if (x = workset & g[lab[i]]) /* not == */
                            cnt = POPCOUNT(x);
                        else
                            cnt = 0;
                        while (bmin > cnt)
                            bucket[--bmin] = 0;
                        while (bmax < cnt)
                            bucket[++bmax] = 0;
                        ++bucket[cnt];
                        count[i] = cnt;
                    }
                    if (bmin == bmax)
                    {
                        continue;
                    }
                    c1 = cell1;
                    for (i = bmin; i <= bmax; ++i)
                        if (bucket[i])
                        {
                            c2 = c1 + bucket[i];
                            bucket[i] = c1;
                            if (c1 != cell1)
                            {
                                ADDELEM1(active,c1);
                                ++*numcells;
                            }
                            if (c2 <= cell2)
                                ptn[c2-1] = 0;
                            c1 = c2;
                        }
                    for (i = cell1; i <= cell2; ++i)
                        workperm[bucket[count[i]]++] = lab[i];
                    for (i = cell1; i <= cell2; ++i)
                        lab[i] = workperm[i];
                }
            }

            if (ptn[n-2] == 0)
            {
                if (lab[n-1] == n-1)
                {
                    *code = 1;
                    if (goodret) return;
                }
                else
                {
                    *code = -1;
                    return;
                }
            }
            else
            {
                i = n - 1;
                while (1)
                {
                    if (lab[i] == n-1) break;
                    --i;
                    if (ptn[i] == 0)
                    {
                        *code = -1;
                        return;
                    }
                }
            }
        }
}

/**************************************************************************/

static void
makecanon(g,gcan,n)
graph *g,*gcan;                 /* gcan := canonise(g) */
int n;
{
        nvector lab[MAXN],ptn[MAXN],orbits[MAXN];
        statsblk stats;
        static DEFAULTOPTIONS(options);
        setword workspace[50];

        options.writemarkers = FALSE;
        options.writeautoms = FALSE;
        options.getcanon = TRUE;

        nauty(g,lab,ptn,NILSET,orbits,&options,&stats,workspace,50,1,n,gcan);
}

/**************************************************************************/

static boolean
accept1(g,n2,x,gx,deg,rigid)     /* decide if n2 in theta(g+x) */
graph *g,*gx;                   /* version for n2+1 < maxn2 */
int n2,*deg;
int x;
boolean *rigid;
{
        register int i,n;
        nvector lab[MAXN],ptn[MAXN],orbits[MAXN];
        permutation count[MAXN];
        graph h[MAXN];
        int xw;
        int nx,numcells,code;
        int i0,i1,degn;
        set active[MAXM];
        statsblk stats;
        static DEFAULTOPTIONS(options);
        setword workspace[50];

#ifdef INSTRUMENT
        ++a1calls;
#endif

        n = n1 + n2;
        nx = n + 1;
        for (i = 0; i < n; ++i)
            gx[i] = g[i];
        gx[n] = 0;
        deg[n] = degn = XPOPCOUNT(x);

        xw = x;
        while (xw)
        {
            i = XFIRSTBIT(xw);
            xw &= ~xbit[i];
            gx[i] |= bit[n];
            gx[n] |= bit[i];
            ++deg[i];
        }

        for (i = 0; i < n1; ++i)
        {
            lab[i] = i;
            ptn[i] = 1;
        }
        ptn[n1-1] = 0;

        i0 = n1;
        i1 = n;
        for (i = n1; i < nx; ++i)
        {
            if (deg[i] == degn) lab[i1--] = i;
            else                lab[i0++] = i;
            ptn[i] = 1;
        }

        ptn[n] = 0;

        if (i0 == n1)
        {
            numcells = 2;
            active[0] = bit[0] | bit[n1];
        }
        else
        {
            numcells = 3;
            active[0] = bit[0] | bit[n1] | bit[i1+1];
            ptn[i1] = 0;
        }
        refinex(gx,lab,ptn,0,&numcells,count,active,FALSE,&code,1,nx);

        if (code < 0) return FALSE;

        if (numcells == nx)
        {
            *rigid = TRUE;
#ifdef INSTRUMENT
            ++a1succs;
#endif
            return TRUE;
        }

        options.writemarkers = FALSE;
        options.writeautoms = FALSE;
        options.getcanon = TRUE;
        options.defaultptn = FALSE;
        options.userautomproc = userautomproc;

        active[0] = 0;
#ifdef INSTRUMENT
        ++a1nauty;
#endif
        nauty(gx,lab,ptn,active,orbits,&options,&stats,workspace,50,1,nx,h);

        if (orbits[lab[n]] == orbits[n])
        {
            *rigid = stats.numorbits == nx;
#ifdef INSTRUMENT
            ++a1succs;
#endif
            return TRUE;
        }
        else
            return FALSE;
}

/**************************************************************************/

static boolean
accept2(g,n2,x,gx,deg,nuniq)   /* decide if n in theta(g+x) */
graph *g,*gx;                  /* version for n+1 == maxn */
int n2,x,deg[];
boolean nuniq;
{
        register int i,n;
        nvector lab[MAXN],ptn[MAXN],orbits[MAXN];
        int degx[MAXN],invar[MAXN];
        setword vmax,gv;
        int qn,qv;
        permutation count[MAXN];
        int xw;
        int nx,numcells,code;
        int degn,i0,i1,j,j0,j1;
        set active[MAXM];
        statsblk stats;
        static DEFAULTOPTIONS(options);
        setword workspace[50];

#ifdef INSTRUMENT
        ++a2calls;
        if (nuniq) ++a2uniq;
#endif
        n = n1 + n2;
        nx = n + 1;
        for (i = 0; i < n; ++i)
        {
            gx[i] = g[i];
            degx[i] = deg[i];
        }
        gx[n] = 0;
        degx[n] = degn = XPOPCOUNT(x);

        xw = x;
        while (xw)
        {
            i = XFIRSTBIT(xw);
            xw &= ~xbit[i];
            gx[i] |= bit[n];
            gx[n] |= bit[i];
            ++degx[i];
        }

        if (nuniq)
        {
#ifdef INSTRUMENT
            ++a2succs;
#endif
            if (canonise) makecanon(gx,gcan,nx);
            return TRUE;
        }

        for (i = 0; i < n1; ++i)
        {
            lab[i] = i;
            ptn[i] = 1;
        }
        ptn[n1-1] = 0;

        i0 = n1;
        i1 = n;
        for (i = n1; i < nx; ++i)
        {
            if (degx[i] == degn) lab[i1--] = i;
            else                 lab[i0++] = i;
            ptn[i] = 1;
        }

        ptn[n] = 0;

        if (i0 == n1)
        {
            numcells = 2;
            active[0] = bit[0] | bit[n1];

            if (!distinvar(gx,invar,n1,n2+1)) return FALSE;
            qn = invar[n];
            j0 = n1;
            j1 = n;
            while (j0 <= j1)
            {
                j = lab[j0];
                qv = invar[j];
                if (qv < qn)
                    ++j0;
                else
                {
                    lab[j0] = lab[j1];
                    lab[j1] = j;
                    --j1;
                }
            }
            if (j0 > n1)
            {
                if (j0 == n)
                {
#ifdef INSTRUMENT
                    ++a2succs;
#endif
                    if (canonise) makecanon(gx,gcan,nx);
                    return TRUE;
                }
                ptn[j1] = 0;
                ++numcells;
                active[0] |= bit[j0];
            }
        }
        else
        {
            numcells = 3;
            ptn[i1] = 0;
            active[0] = bit[0] | bit[n1] | bit[i1+1];

            vmax = 0;
            j = MAXN;
            for (i = 0; i < n1; ++i)
                if (degx[i] < j && degx[i] > 0)
                {
                    j = degx[i];
                    vmax = bit[i];
                }
                else if (degx[i] == j)
                    vmax |= bit[i];

            gv = gx[n] & vmax;
            qn = POPCOUNT(gv);

            j0 = i1+1;
            j1 = n;
            while (j0 <= j1)
            {
                j = lab[j0];
                gv = gx[j] & vmax;
                qv = POPCOUNT(gv);
                if (qv > qn)
                    return FALSE;
                else if (qv < qn)
                    ++j0;
                else
                {
                    lab[j0] = lab[j1];
                    lab[j1] = j;
                    --j1;
                }
            }
            if (j0 > i1+1)
            {
                if (j0 == n)
                {
#ifdef INSTRUMENT
                    ++a2succs;
#endif
                    if (canonise) makecanon(gx,gcan,nx);
                    return TRUE;
                }
                ptn[j1] = 0;
                ++numcells;
                active[0] |= bit[j0];
            }
        }

        refinex(gx,lab,ptn,0,&numcells,count,active,TRUE,&code,1,nx);

        if (code < 0) return FALSE;
        else if (code > 0 || numcells >= nx-4)
        {
#ifdef INSTRUMENT
            ++a2succs;
#endif
            if (canonise) makecanon(gx,gcan,nx);
            return TRUE;
        }

        options.writemarkers = FALSE;
        options.writeautoms = FALSE;
        options.getcanon = TRUE;
        options.defaultptn = FALSE;

        active[0] = 0;
#ifdef INSTRUMENT
        ++a2nauty;
#endif
        nauty(gx,lab,ptn,active,orbits,&options,&stats,workspace,50,1,nx,gcan);

        if (orbits[lab[n]] == orbits[n])
        {
#ifdef INSTRUMENT
            ++a2succs;
#endif
            if (canonise) makecanon(gx,gcan,nx);
            return TRUE;
        }
        else
            return FALSE;
}

/**************************************************************************/

static void
xbnds(n2,ne,dmax)    /* find bounds on degree for vertex n2 */
int n2,ne,dmax;      /* store answer in data[*].*  */
{
        register int xlb,xub,m;

        xlb = n2 == 0 ? (connec ? 1 : 0) : dmax;
        m = mine - ne - (maxn2 - n2 -1)*maxdeg2;
        if (m > xlb) xlb = m;

        xub = maxdeg2;
        m = (maxe - ne) / (maxn2 - n2);
        if (m < xub) xub = m;

        data[n2].ne = ne;
        data[n2].dmax = dmax;
        data[n2].xlb = xlb;
        data[n2].xub = xub;
}

/**************************************************************************/

static void
genextend(g,n2,deg,ne,rigid,xlb,xub)        /* extend from n2 to n2+1 */
graph *g;
int n2,*deg,ne,xlb,xub;
boolean rigid;
{
        register int x,d;
        int *xorb,xc;
        int nx,i,j,imin,imax,dmax;
        int xlbx,xubx,n;
        graph gx[MAXN];
        int degx[MAXN];
        boolean rigidx;

#ifdef INSTRUMENT
        boolean haschild;

        haschild = FALSE;
        ++nodes[n2];
        if (rigid) ++rigidnodes[n2];
#endif

        n = n1 + n2;
        nx = n2 + 1;
        dmax = deg[n-1];

        d = 0;
        for (i = 0; i < n1; ++i)
            if (deg[i] == maxdeg1) d |= xbit[i];

        if (xlb > xub) return;

        imin = xstart[xlb];
        imax = xstart[xub+1];
        xorb = data[n2].xorb;

        if (nx == maxn2)
            for (i = imin; i < imax; ++i)
            {
                if (!rigid && xorb[i] != i) continue;
                x = xset[i];
                xc = xcard[i];
                if ((x & d) != 0) continue;

                if (nx == nprune)
                {
                    if (curres == 0) curres = mod;
                    if (--curres != 0) continue;
                }
                if (accept2(g,n2,x,gx,deg,xc > dmax))
                    if (!connec || isconnected(gx,n+1))
                    {
                        ++count[ne+xc];
#ifdef INSTRUMENT
                        haschild = TRUE;
#endif
                        (*outproc)(outfile,canonise ? gcan : gx,n+1);
                    }
            }
        else
            for (i = imin; i < imax; ++i)
            {
                if (!rigid && xorb[i] != i) continue;
                x = xset[i];
                xc = xcard[i];
                if ((x & d) != 0) continue;
                if (nx == nprune)
                {
                    if (curres == 0) curres = mod;
                    if (--curres != 0) continue;
                }
                for (j = 0; j < n; ++j)
                    degx[j] = deg[j];
                if (data[nx].ne != ne+xc || data[nx].dmax != xc)
                    xbnds(nx,ne+xc,xc);
                xlbx = data[nx].xlb;
                xubx = data[nx].xub;
                if (xlbx > xubx) continue;

                data[nx].lo = xstart[xlbx];
                data[nx].hi = xstart[xubx+1];
                if (accept1(g,n2,x,gx,degx,&rigidx))
                {
#ifdef INSTRUMENT
                    haschild = TRUE;
#endif
                    genextend(gx,nx,degx,ne+xc,rigidx,xlbx,xubx);
                }
            }
#ifdef INSTRUMENT
        if (haschild) ++fertilenodes[n2];
#endif
}

/**************************************************************************/
/**************************************************************************/

main(argc,argv)
int argc;
char *argv[];
{
        char *arg;
        long ltemp;
        boolean badargs;
        int i,j,imin,imax,argsgot;
        graph g[MAXN1];
        int deg[MAXN1];
        long nout;
        double t1,t2;

        badargs = FALSE;
        connec = FALSE;
        verbose = FALSE;
        nautyformat = FALSE;
        nooutput = FALSE;
        canonise = FALSE;

        maxdeg1 = maxdeg2 = MAXN;

        argsgot = 0;
        for (i = 1; !badargs && i < argc; ++i)
        {
            arg = argv[i];
            if (arg[0] == '-' && arg[1] != '\0')
            {
                if (arg[1] == 'c' || arg[1] == 'C') connec = TRUE;
                else if (arg[1] == 'd' || arg[1] == 'D')
                {
                    j = sscanf(arg+2,"%d:%d",&maxdeg1,&maxdeg2);
                    if      (j == 1) maxdeg2 = maxdeg1;
                    else if (j == 0) badargs = TRUE;
                }
                else if (arg[1] == 'n' || arg[1] == 'N') nautyformat = TRUE;
                else if (arg[1] == 'u' || arg[1] == 'U') nooutput = TRUE;
                else if (arg[1] == 'v' || arg[1] == 'V') verbose = TRUE;
                else if (arg[1] == 'l' || arg[1] == 'L') canonise = TRUE;
                else badargs = TRUE;
            }
            else
            {
                if (argsgot > 5)
                    badargs = TRUE;
                else
                {
                    if (sscanf(arg,"%ld",&ltemp) != 1) badargs = TRUE;
                    else if (argsgot == 0) n1 = ltemp;
                    else if (argsgot == 1) maxn2 = ltemp;
                    else if (argsgot == 2) mine = ltemp;
                    else if (argsgot == 3) maxe = ltemp;
                    else if (argsgot == 4) mod = ltemp;
                    else if (argsgot == 5) res = ltemp;
                }
                ++argsgot;
            }
        }

        if (argsgot < 2)
            badargs = TRUE;
        else if (n1 < 1 || maxn2 < 0 || n1 > MAXN1 || n1+maxn2 > MAXN)
        {
            fprintf(stderr,
               ">E makebg: must have n1=1..%d,n1+n2=n1..%d\n",MAXN1,MAXN);
            badargs = TRUE;
        }

        if (argsgot == 2)
        {
            mine = 0;
            maxe = n1 * maxn2;
        }
        else if (argsgot == 3)
            maxe = mine;

        if (argsgot <= 4)
        {
            mod = 1;
            res = 0;
        }
        else if (argsgot == 5 || argsgot > 6)
            badargs = TRUE;

        if (maxdeg1 > maxn2) maxdeg1 = maxn2;
        if (maxdeg2 > n1) maxdeg2 = n1;
        if (maxe > n1*maxdeg1) maxe =  n1*maxdeg1;
        if (maxe > maxn2*maxdeg2) maxe =  maxn2*maxdeg2;

        if (!badargs && (mine > maxe || maxe < 0 || maxdeg1 < 0 || maxdeg2 < 0))
        {
            fprintf(stderr,">E makebg: impossible mine,maxe,maxdeg values\n");
            badargs = TRUE;
        }
        if (connec && mine < n1+maxn2-1) mine = n1 + maxn2 - 1;

        if (badargs)
        {
            fprintf(stderr,
">E Usage: makebg [-c -n -u -v -l] [-d<max>] n1 n2 [mine [maxe [mod res]]]\n");
            exit(2);
        }

#ifdef OUTPROC
        outproc = OUTPROC;
#else
        if (nautyformat)   outproc = writenauty;
        else if (nooutput) outproc = nullwrite;
        else               outproc = writeny;
#endif

        for (i = 0; i <= maxe; ++i)
            count[i] = 0;

        msgfile = stderr;
        outfile = stdout;
        fprintf(msgfile,">A makebg n=%d:%d e=%d:%d d=%d:%d class=%d/%d\n",
                        n1,maxn2,mine,maxe,maxdeg1,maxdeg2,mod,res);

        class1size = n1;

        for (i = 0; i < n1; ++i)
        {
            g[i] = 0;
            deg[i] = 0;
        }

        t1 = CPUTIME;

        if (maxn2 == 0)
        {
            if (res == 0)
            {
                ++count[0];
                (*outproc)(outfile,g,n1);
            }
        }
        else
        {
            makeleveldata();
            curres = res;
            if (mod <= 1)        nprune = 0;
            else if (maxn2 >= 6) nprune = maxn2 - 2;
            else if (maxn2 >= 3) nprune = maxn2 - 1;
            else                 nprune = maxn2;

            xbnds(0,0,0);
            imin = xstart[data[0].xlb];
            imax = xstart[data[0].xub+1];

            for (i = imin; i < imax; ++i)
                data[0].xorb[i] = -1;

            for (i = data[0].xlb; i <= data[0].xub; ++i)
                data[0].xorb[xstart[i]] = xstart[i];

            genextend(g,0,deg,0,FALSE,data[0].xlb,data[0].xub);
        }
        t2 = CPUTIME;

        nout = 0;
        for (i = 0; i <= maxe; ++i)
            nout += count[i];

        if (verbose)
            for (i = 0; i <= maxe; ++i)
                if (count[i] > 0)
                    fprintf(msgfile,
                      ">C %7ld graphs with %d edges\n",count[i],i);

#ifdef INSTRUMENT
        fprintf(msgfile,"\n>N node counts\n");
        for (i = 0; i < maxn2; ++i)
            fprintf(msgfile," level %2d: %7ld (%ld rigid, %ld fertile)\n",
                            i,nodes[i],rigidnodes[i],fertilenodes[i]);
        fprintf(msgfile,">A1 %ld calls to accept1, %ld nauty, %ld succeeded\n",
                        a1calls,a1nauty,a1succs);
        fprintf(msgfile,
             ">A2 %ld calls to accept2, %ld nuniq, %ld nauty, %ld succeeded\n",
                        a2calls,a2uniq,a2nauty,a2succs);
        fprintf(msgfile,"\n");
#endif

        fprintf(msgfile,">Z %ld graphs generated in %3.2f sec\n",nout,t2-t1);

        exit(0);
}
