/*****************************************************************************
*                                                                            *
* This is the main file for dreadnaut() version 1.9, which is a test-bed     *
*   for nauty() version 1.9.                                                 *
*                                                                            *
*   Copyright (1984-1993) Brendan McKay.  All rights reserved.               *
*   Subject to the waivers and disclaimers in nauty.h.                       *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       10-Nov-87 : final changes for version 1.2                            *
*        5-Dec-87 - replaced all uses of fscanf() by appropriate uses        *
*                   of the new procedures readinteger() and readstring()     *
*                 - changed the '<' command slightly.  If a file of the      *
*                   given name cannot be openned, an attempt is made to      *
*                   open a file with the same name extended by DEFEXT.       *
*                 - improved error processing for 'n' command.               *
*       28-Sep-88 : changes for version 1.4 :                                *
*                 - replaced incorrect %d by %ld in fprintf for ? command    *
*       23-Mar-89 : changes for version 1.5 :                                *
*                 - added optional startup message                           *
*                 - enabled use of refine1 in 'i' command                    *
*                 - implemented $$ command                                   *
*                 - replaced ALLOCS test by DYNALLOC test                    *
*                 - modified @ command and added # command                   *
*                 - declared local procedures static                         *
*       25-Mar-89 - implemented k command                                    *
*       27-Mar-89 - implemented * and I commands                             *
*       29-Mar-89 - implemented K command                                    *
*        2-Apr-89 - added reporting of vertex-invariant statistics           *
*        2-Apr-89 - added ## command                                         *
*        4-Apr-89 - added triples(), quadruples(), adjtriang()               *
*                 - updated error reporting for nauty()                      *
*        5-Apr-89 - removed flushline() from g and e commands                *
*                 - added T command                                          *
*        6-Apr-89 - added cellquads() and distances()                        *
*       26-Apr-89 - modified ? command, added & and && commands              *
*                 - added indsets(), cliques(), cellquins()                  *
*       18-Aug-89 - made g, lab, canong dynamically allocated always         *
*        2-Mar-90 - added celltrips(), cellcliq(),cellind()                  *
*       13-Mar-90 - changed canong and savedg in output to h and h'          *
*       19-Mar-90 - revised help() a little                                  *
*       19-Apr-90 : changes for version 1.6                                  *
*                 - rewrote "*" command to avoid bug in Pyramid C compiler   *
*       20-Apr-90 - rewrote above rewrite to avoid bug in SUN3 gcc           *
*       23-Apr-90 - undid above rewrite and fixed *my* bug <blush> by        *
*                   making NUMINVARS have type int.  Sorry, gcc.             *
*       10-Nov-90 - added calls to null routines (see comment on code)       *
*       27-Aug-92 : renamed to version 1.7, no changes to this file          *
*        5-Jun-93 : renamed to version 1.7+, no changes to this file         *
*       18-Aug-93 : renamed to version 1.8, no changes to this file          *
*       17-Sep-93 : changes for version 1.9 :                                *
*                 - added invariant adjacencies()                            *
*                                                                            *
*****************************************************************************/

#include "naututil.h"    /* which includes nauty.h, which includes stdio.h */

#define PM(x) ((x) ? '+' : '-')
#define SS(n,sing,plur)  (n),((n)==1?(sing):(plur))

#ifdef  CPUDEFS
CPUDEFS                 /* data decls. for CPUTIME */
#endif

#define INFILE fileptr[curfile]
#define OUTFILE outfile

extern long seed;

static graph *g,*canong;
static nvector *lab;

#ifdef  DYNALLOC
static setword *workspace;
static nvector *ptn,*orbits;
static permutation *perm;
#else
static setword workspace[2*MAXM*WORKSIZE];
static nvector ptn[MAXN],orbits[MAXN];
static permutation perm[MAXN];
#endif

static DEFAULTOPTIONS(options);
static statsblk stats;
static set active[MAXM];
static int curfile;
static FILE *fileptr[MAXIFILES];
static FILE *outfile;
static char def_ext[] = DEFEXT;
static boolean firstpath;       /* used in usernode() */

#define U_NODE  1               /* masks for u values */
#define U_AUTOM 2
#define U_LEVEL 4
#define U_TCELL 8
#define U_REF  16

#ifndef  NODEPROC
#define NODEPROC usernode
#else
extern UPROC NODEPROC();
#endif

#ifndef  AUTOMPROC
#define AUTOMPROC userautom
#else
extern UPROC AUTOMPROC();
#endif

#ifndef  LEVELPROC
#define LEVELPROC userlevel
#else
extern UPROC LEVELPROC();
#endif

#ifndef  TCELLPROC
#define TCELLPROC NILFUNCTION
#else
extern UPROC TCELLPROC();
#endif

#ifndef  REFPROC
#define REFPROC NILFUNCTION
#else
extern UPROC REFPROC();
#endif

#ifndef  INVARPROC
#define INVARPROC NILFUNCTION
#define INVARPROCNAME "none"
#else
extern UPROC INVARPROC();
#define INVARPROCNAME "user-defined"
#endif

static struct invarrec
{
    UPROC (*entrypoint)();
    char *name;
} invarproc[]
    = {INVARPROC, INVARPROCNAME,
       NILFUNCTION, "none",
       twopaths,    "twopaths",
       adjtriang,   "adjtriang",
       triples,     "triples",
       quadruples,  "quadruples",
       celltrips,   "celltrips",
       cellquads,   "cellquads",
       cellquins,   "cellquins",
       distances,   "distances",
       indsets,     "indsets",
       cliques,     "cliques",
       cellcliq,    "cellcliq",
       cellind,     "cellind",
       adjacencies, "adjacencies"};
#define NUMINVARS ((int)(sizeof(invarproc)/sizeof(struct invarrec)))

static void allocg();
static void help();
static UPROC userautom();
static UPROC usernode();
static UPROC userlevel();

extern void nauty_null();
extern void nautil_null();
extern void nautinv_null();

#ifdef  EXTRADECLS
EXTRADECLS
#endif

/*****************************************************************************
*                                                                            *
*  This is a program which illustrates the use of nauty.                     *
*  Commands are read from stdin, and may be separated by white space,        *
*  commas or not separated.  Output is written to stdout.                    *
*  For a short description, see the nauty User's Guide.                      *
*                                                                            *
*****************************************************************************/

main()
{
        int m,n,newm,newn;
        boolean gvalid,ovalid,cvalid,pvalid,minus,prompt,doquot;
        int i,worksize,numcells,refcode,umask,qinvar;
        int oldorg;
        char *s1,*s2,*invarprocname;
        int c,d;
        register long li;
        set *gp;
        double timebefore,timeafter;
        char filename[100];
        graph *savedg;
        nvector *savedlab;
        int sgn,sgactn,sgorg;
        int cgactn,gactn;

        curfile = 0;
        fileptr[curfile] = stdin;
        prompt = DOPROMPT(INFILE);
        outfile = stdout;
        n = m = 1;

#ifdef  INITSEED
        INITSEED;
#endif

        umask = 0;
        pvalid = FALSE;
        gvalid = FALSE;
        ovalid = FALSE;
        cvalid = FALSE;
        minus = FALSE;
        worksize = 2*MAXM*WORKSIZE;
        labelorg = oldorg = 0;
        cgactn = sgactn = gactn = 0;

#ifdef  DYNALLOC
        workspace = (setword*) ALLOCS(WORKSIZE,2*MAXM*sizeof(setword));
        ptn = (nvector*) ALLOCS(MAXN,sizeof(nvector));
        orbits = (nvector*) ALLOCS(MAXN,sizeof(nvector));
        perm = (permutation*) ALLOCS(MAXN,sizeof(permutation));

        if (workspace == NILSET || ptn == (nvector*)NULL ||
                orbits == (nvector*)NULL || perm == (permutation*)NULL)
        {
            fprintf(ERRFILE,"ALLOCS failed; reduce MAXN.\n\n");
            EXIT;
        }
#endif

#ifdef  INITIALIZE
        INITIALIZE;
#endif

        allocg(&g,&lab,&gactn,n);
        if (gactn == 0)
        {
            fprintf(ERRFILE,"ALLOCS failed for g: this shouldn't happen.\n\n");
            EXIT;
        }

        invarprocname = "none";
        if (prompt)
        {
            fprintf(PROMPTFILE,"Dreadnaut version %s.\n",DREADVERSION);
            fprintf(PROMPTFILE,"> ");
        }

     /* Calling dummy routines in nautinv.c, nauty.c and nautil.c causes
        those segments to get loaded in various Macintosh variants.  This
        causes an apparent, but illusory, improvement in the time required
        for the first call to nauty().   */

        nautinv_null();
        nautil_null();
        nauty_null();

        while (curfile >= 0)
            if ((c = getc(INFILE)) == EOF || c == '\004')
            {
                fclose(INFILE);
                --curfile;
                if (curfile >= 0)
                    prompt = DOPROMPT(INFILE);
            }
            else switch (c)
            {
            case '\n':  /* possibly issue prompt */
                if (prompt)
                    fprintf(PROMPTFILE,"> ");
                minus = FALSE;
                break;

            case ' ':   /* do nothing */
            case '\t':
#ifndef  NLMAP
            case '\r':
#endif
            case '\f':
                break;

            case '-':   /* remember this for next time */
                minus = TRUE;
                break;

            case '+':   /* forget - */
            case ',':
            case ';':
                minus = FALSE;
                break;

            case '<':   /* new input file */
                minus = FALSE;
                if (curfile == MAXIFILES - 1)
                    fprintf(ERRFILE,"exceeded maximum input nesting of %d\n\n",
                            MAXIFILES);
                if (!readstring(INFILE,filename))
                {
                    fprintf(ERRFILE,
                            "missing file name on '>' command : ignored\n\n");
                    break;
                }
                if ((fileptr[curfile+1] = fopen(filename,"r")) == NULL)
                {
                    for (s1 = filename; *s1 != '\0'; ++s1) {}
                    for (s2 = def_ext; (*s1 = *s2) != '\0'; ++s1, ++s2) {}
                    fileptr[curfile+1] = fopen(filename,"r");
                }
                if (fileptr[curfile+1] != NULL)
                {
                    ++curfile;
                    prompt = DOPROMPT(INFILE);
                    if (prompt)
                        fprintf(PROMPTFILE,"> ");
                }
                else
                    fprintf(ERRFILE,"can't open input file\n\n");
                break;

            case '>':   /* new output file */
                if ((d = getc(INFILE)) != '>')
                    ungetc((char)d,INFILE);
                if (minus)
                {
                    minus = FALSE;
                    if (outfile != stdout)
                    {
                        fclose(outfile);
                        outfile = stdout;
                    }
                }
                else
                {
                    if (!readstring(INFILE,filename))
                    {
                        fprintf(ERRFILE,
                            "improper file name, reverting to stdout\n\n");
                        outfile = stdout;
                        break;
                    }
                    OPENOUT(outfile,filename,d=='>');
                    if (outfile == NULL)
                    {
                        fprintf(ERRFILE,
                            "can't open output file, reverting to stdout\n\n");
                        outfile = stdout;
                    }
                }
                break;

            case '!':   /* ignore rest of line */
                do
                    c = getc(INFILE);
                while (c != '\n' && c != EOF);
                if (c == '\n')
                    ungetc('\n',INFILE);
                break;

            case 'n':   /* read n value */
                minus = FALSE;
                i = getint(INFILE);
                if (i <= 0 || i > MAXN)
                    fprintf(ERRFILE,
                         " n can't be less than 1 or more than %d\n\n",MAXN);
                else
                {
                    gvalid = FALSE;
                    ovalid = FALSE;
                    cvalid = FALSE;
                    pvalid = FALSE;
                    n = i;
                    m = (n + WORDSIZE - 1) / WORDSIZE;
                    allocg(&g,&lab,&gactn,n);
                    if (gactn == 0)
                    {
                        fprintf(ERRFILE,"can't allocate space for graph\n");
                        n = m = 1;
                        break;
                    }
                }
                break;

            case 'g':   /* read graph */
                minus = FALSE;
                readgraph(INFILE,g,options.digraph,prompt,FALSE,
                          options.linelength,m,n);
                gvalid = TRUE;
                cvalid = FALSE;
                ovalid = FALSE;
                break;

            case 'e':   /* edit graph */
                minus = FALSE;
                readgraph(INFILE,g,options.digraph,prompt,gvalid,
                          options.linelength,m,n);
                gvalid = TRUE;
                cvalid = FALSE;
                ovalid = FALSE;
                break;

            case 'r':   /* relabel graph and current partition */
                minus = FALSE;
                if (gvalid)
                {
                    allocg(&canong,(nvector**)NULL,&cgactn,n);
                    if (cgactn == 0)
                    {
                        fprintf(ERRFILE,
                                "can't allocate work space for 'r'\n\n");
                        break;
                    }
                    readperm(INFILE,perm,prompt,n);
                    relabel(g,(pvalid ? lab : (nvector*)NULL),perm,canong,m,n);
                    cvalid = FALSE;
                    ovalid = FALSE;
                }
                else
                    fprintf(ERRFILE,"g is not defined\n\n");
                break;

            case '_':   /* complement graph */
                minus = FALSE;
                if (gvalid)
                {
                    complement(g,m,n);
                    cvalid = FALSE;
                    ovalid = FALSE;
                }
                else
                    fprintf(ERRFILE,"g is not defined\n\n");
                break;

            case '@':   /* copy canong into savedg */
                minus = FALSE;
                if (cvalid)
                {
                    allocg(&savedg,&savedlab,&sgactn,n);
                    if (sgactn == 0)
                    {
                        fprintf(ERRFILE,"can`t allocate space for h'\n\n");
                        break;
                    }
                    sgn = n;
                    for (li = (long)n * (long)m; --li >= 0;)
                        savedg[li] = canong[li];
                    for (i = n; --i >= 0;)
                        savedlab[i] = lab[i];
                    sgorg = labelorg;
                }
                else
                    fprintf(ERRFILE,"h is not defined\n\n");
                break;

            case '#':   /* compare canong to savedg */
                if ((d = getc(INFILE)) != '#')
                    ungetc((char)d,INFILE);

                if (cvalid)
                {
                    if (sgactn > 0)
                    {
                        if (sgn != n)
                            fprintf(OUTFILE,
                                  "h and h' have different sizes.\n");
                        else
                        {
                            for (li = (long)n * (long)m; --li >= 0;)
                                if (savedg[li] != canong[li])
                                    break;
                            if (li >= 0)
                                fprintf(OUTFILE,
                                   "h and h' are different.\n");
                            else
                            {
                                fprintf(OUTFILE,
                                   "h and h' are identical.\n");
                                if (d == '#')
                                    putmapping(OUTFILE,savedlab,sgorg,
                                           lab,labelorg,options.linelength,n);
                            }
                        }
                    }
                    else
                        fprintf(ERRFILE,"h' is not defined\n\n");
                }
                else
                    fprintf(ERRFILE,"h is not defined\n\n");
                break;

            case 'j':   /* relabel graph randomly */
                minus = FALSE;
                if (gvalid)
                {
                    allocg(&canong,(nvector**)NULL,&cgactn,n);
                    if (cgactn == 0)
                    {
                        fprintf(ERRFILE,
                                "can't allocate work space for 'j'\n\n");
                        break;
                    }
                    ranperm(perm,n);
                    relabel(g,(pvalid ? lab : (nvector*)NULL),perm,canong,m,n);
                    cvalid = FALSE;
                    ovalid = FALSE;
                }
                else
                    fprintf(ERRFILE,"g is not defined\n\n");
                break;

            case 'v':   /* write vertex degrees */
                minus = FALSE;
                if (gvalid)
                    putdegs(OUTFILE,g,options.linelength,m,n);
                else
                    fprintf(ERRFILE,"g is not defined\n\n");
                break;

            case '%':   /* do Mathon doubling operation */
                minus = FALSE;
                if (gvalid)
                {
                    if (2L * ((long)n + 1L) > MAXN)
                    {
                        fprintf(ERRFILE,"n can't be more than %d\n\n",MAXN);
                        break;
                    }
                    newn = 2 * (n + 1);
                    newm = (newn + WORDSIZE - 1) / WORDSIZE;
                    allocg(&canong,(nvector**)NULL,&cgactn,n);
                    if (cgactn == 0)
                    {
                        fprintf(ERRFILE,
                                "can't allocate work space for '%'\n\n");
                        break;
                    }

                    for (li = (long)n * (long)m; --li >= 0;)
                        canong[li] = g[li];

                    allocg(&g,&lab,&gactn,newn);
                    if (gactn == 0)
                    {
                        fprintf(ERRFILE,"can't allocate space for graph \n\n");
                        break;
                    }
                    mathon(canong,m,n,g,newm,newn);
                    m = newm;
                    n = newn;
                    cvalid = FALSE;
                    ovalid = FALSE;
                    pvalid = FALSE;
                }
                else
                    fprintf(ERRFILE,"g is not defined\n\n");
                break;

            case 's':   /* generate random graph */
                minus = FALSE;
                i = getint(INFILE);
                if (i <= 0)
                    i = 2;
                rangraph(g,options.digraph,i,m,n);
                gvalid = TRUE;
                cvalid = FALSE;
                ovalid = FALSE;
                break;

            case 'q':   /* quit */
                EXIT;
                break;

            case '"':   /* copy comment to output */
                minus = FALSE;
                copycomment(INFILE,OUTFILE,'"');
                break;

            case 'I':   /* do refinement and invariants procedure */
                if (!pvalid)
                    unitptn(lab,ptn,&numcells,n);
                cellstarts(ptn,0,active,m,n);
#ifdef  CPUTIME
                timebefore = CPUTIME;
#endif
                doref(g,lab,ptn,0,&numcells,&qinvar,perm,active,&refcode,
                      refine,options.invarproc,
                      0,0,options.invararg,options.digraph,m,n);
#ifdef  CPUTIME
                timeafter = CPUTIME;
#endif
                fprintf(OUTFILE," %d cell%s; code = %x",
                        SS(numcells,"","s"),refcode);
                if (options.invarproc != NILFUNCTION)
                    fprintf(OUTFILE," (%s %s)",invarprocname,
                        (qinvar == 2 ? "worked" : "failed"));
#ifdef  CPUTIME
                fprintf(OUTFILE,"; cpu time = %.2f seconds\n",
                        timeafter-timebefore);
#else
                fprintf(OUTFILE,"\n");
#endif
                if (numcells > 1)
                    pvalid = TRUE;
                break;

            case 'i':   /* do refinement */
                if (!pvalid)
                    unitptn(lab,ptn,&numcells,n);
                cellstarts(ptn,0,active,m,n);
                if (m == 1)
                    refine1(g,lab,ptn,0,&numcells,perm,active,&refcode,m,n);
                else
                    refine(g,lab,ptn,0,&numcells,perm,active,&refcode,m,n);
                fprintf(OUTFILE," %d cell%s; code = %x\n",
                        SS(numcells,"","s"),refcode);
                if (numcells > 1)
                    pvalid = TRUE;
                break;

            case 'x':   /* execute nauty */
                minus = FALSE;
                ovalid = FALSE;
                cvalid = FALSE;
                if (!gvalid)
                {
                    fprintf(ERRFILE,"g is not defined\n\n");
                    break;
                }
                if (pvalid)
                {
                    fprintf(OUTFILE,"[fixing partition]\n");
                    options.defaultptn = FALSE;
                }
                else
                    options.defaultptn = TRUE;
                options.outfile = outfile;

                if (options.getcanon)
                {
                    allocg(&canong,(nvector**)NULL,&cgactn,n);
                    if (cgactn == 0)
                    {
                        fprintf(ERRFILE,"can't allocate space for h\n\n");
                        break;
                    }
                }

                firstpath = TRUE;
#ifdef  CPUTIME
                timebefore = CPUTIME;
#endif
                nauty(g,lab,ptn,NILSET,orbits,&options,&stats,workspace,
                      worksize,m,n,canong);
#ifdef  CPUTIME
                timeafter = CPUTIME;
#endif
                if (stats.errstatus != 0)
                    fprintf(ERRFILE,
                      "nauty returned error status %d [this can't happen]\n\n",
                       stats.errstatus);
                else
                {
                    if (options.getcanon)
                        cvalid = TRUE;
                    ovalid = TRUE;
                    fprintf(OUTFILE,"%d orbit%s",SS(stats.numorbits,"","s"));
                    if (stats.grpsize2 == 0)
                        fprintf(OUTFILE,"; grpsize=%.0f",stats.grpsize1+0.1);
                    else
                    {
                        while (stats.grpsize1 >= 10.0)
                        {
                            stats.grpsize1 /= 10.0;
                            ++stats.grpsize2;
                        }
                        fprintf(OUTFILE,"; grpsize=%12.10fe%d",
                                   stats.grpsize1,stats.grpsize2);
                    }
                    fprintf(OUTFILE,"; %d gen%s",
                            SS(stats.numgenerators,"","s"));
                    fprintf(OUTFILE,"; %ld node%s",SS(stats.numnodes,"","s"));
                    if (stats.numbadleaves)
                        fprintf(OUTFILE," (%ld bad lea%s)",
                                SS(stats.numbadleaves,"f","ves"));
                    fprintf(OUTFILE,"; maxlev=%d\n", stats.maxlevel);
                    fprintf(OUTFILE,"tctotal=%ld",stats.tctotal);
                    if (options.getcanon)
                        fprintf(OUTFILE,"; canupdates=%ld",stats.canupdates);
#ifdef  CPUTIME
                    fprintf(OUTFILE,"; cpu time = %.2f seconds\n",
                            timeafter-timebefore);
#else
                    fprintf(OUTFILE,"\n");
#endif
                    if (options.invarproc != NILFUNCTION &&
                                           options.maxinvarlevel != 0)
                    {
                        fprintf(OUTFILE,"invarproc \"%s\" succeeded %ld/%ld",
                            invarprocname,stats.invsuccesses,stats.invapplics);
                        if (stats.invarsuclevel > 0)
                            fprintf(OUTFILE," beginning at level %d.\n",
                                    stats.invarsuclevel);
                        else
                            fprintf(OUTFILE,".\n");
                    }
                }
                break;

            case 'f':   /* read initial partition */
                if (minus)
                {
                    pvalid = FALSE;
                    minus = FALSE;
                }
                else
                {
                    readptn(INFILE,lab,ptn,&numcells,prompt,n);
                    pvalid = TRUE;
                }
                break;

            case 't':   /* type graph */
                minus = FALSE;
                if (!gvalid)
                    fprintf(ERRFILE,"g is not defined\n\n");
                else
                    putgraph(OUTFILE,g,options.linelength,m,n);
                break;

            case 'T':   /* type graph preceded by n, $ and g commands */
                minus = FALSE;
                if (!gvalid)
                    fprintf(ERRFILE,"g is not defined\n\n");
                else
                {
                    fprintf(OUTFILE,"n=%d $=%d g\n",n,labelorg);
                    putgraph(OUTFILE,g,options.linelength,m,n);
                    fprintf(OUTFILE,"$$\n");
                }
                break;

            case 'u':   /* call user procs */
                if (minus)
                {
                    umask = 0;
                    minus = FALSE;
                }
                else
                {
                    umask = getint(INFILE);
                    if (umask < 0)
                        umask = ~0;
                }
                if (umask & U_NODE)
                    options.usernodeproc = NODEPROC;
                else
                    options.usernodeproc = NILFUNCTION;
                if (umask & U_AUTOM)
                    options.userautomproc = AUTOMPROC;
                else
                    options.userautomproc = NILFUNCTION;
                if (umask & U_LEVEL)
                    options.userlevelproc = LEVELPROC;
                else
                    options.userlevelproc = NILFUNCTION;
                if (umask & U_TCELL)
                    options.usertcellproc = TCELLPROC;
                else
                    options.usertcellproc = NILFUNCTION;
                if (umask & U_REF)
                    options.userrefproc = REFPROC;
                else
                    options.userrefproc = NILFUNCTION;
                break;

            case 'o':   /* type orbits */
                minus = FALSE;
                if (ovalid)
                    putorbits(OUTFILE,orbits,options.linelength,n);
                else
                    fprintf(ERRFILE,"orbits are not defined\n\n");
                break;

            case 'b':   /* type canonlab and canong */
                minus = FALSE;
                if (cvalid)
                    putcanon(OUTFILE,lab,canong,options.linelength,m,n);
                else
                    fprintf(ERRFILE,"h is not defined\n\n");
                break;

            case 'z':   /* type hashcode for canong */
                minus = FALSE;
                if (cvalid)
                    fprintf(OUTFILE,"[%8lx %8lx]\n",
                                    hash(canong,(long)m * (long)n,13),
                                    hash(canong,(long)m * (long)n,7));
                else
                    fprintf(ERRFILE,"h is not defined\n\n");
                break;

            case 'c':   /* set getcanon option */
                options.getcanon = !minus;
                minus = FALSE;
                break;

            case 'w':   /* read size of workspace */
                minus = FALSE;
                worksize = getint(INFILE);
                if (worksize > 2*MAXM*WORKSIZE)
                {
                    fprintf(ERRFILE,
                       "too big - setting worksize = %d\n\n", 2*MAXM*WORKSIZE);
                    worksize = 2*MAXM*WORKSIZE;
                }
                break;

            case 'l':   /* read linelength for output */
                options.linelength = getint(INFILE);
                minus = FALSE;
                break;

            case 'y':   /* set tc_level field of options */
                options.tc_level = getint(INFILE);
                minus = FALSE;
                break;

            case 'k':   /* set invarlev fields of options */
                options.mininvarlevel = getint(INFILE);
                options.maxinvarlevel = getint(INFILE);
                minus = FALSE;
                break;

            case 'K':   /* set invararg field of options */
                options.invararg = getint(INFILE);
                minus = FALSE;
                break;

            case '*':   /* set invarproc field of options */
                minus = FALSE;
                d = getint(INFILE);
                if (d >= -1 && d <= NUMINVARS-2)
                {
                    options.invarproc = invarproc[d+1].entrypoint;
                    invarprocname = invarproc[d+1].name;
                }
                else
                    fprintf(ERRFILE,"no such vertex-invariant\n\n");
                break;

            case 'a':   /* set writeautoms option */
                options.writeautoms = !minus;
                minus = FALSE;
                break;

            case 'm':   /* set writemarkers option */
                options.writemarkers = !minus;
                minus = FALSE;
                break;

            case 'p':   /* set cartesian option */
                options.cartesian = !minus;
                minus = FALSE;
                break;

            case 'd':   /* set digraph option */
                if (options.digraph && minus)
                    gvalid = FALSE;
                options.digraph = !minus;
                minus = FALSE;
                break;

            case '$':   /* set label origin */
                if ((d = getc(INFILE)) == '$')
                    labelorg = oldorg;
                else
                {
                    ungetc((char)d,INFILE);
                    oldorg = labelorg;
                    i = getint(INFILE);
                    if (i < 0)
                        fprintf(ERRFILE,"labelorg must be >= 0\n\n");
                    else
                        labelorg = i;
                }
                break;

            case '?':   /* type options, etc. */
                minus = FALSE;
                fprintf(OUTFILE,"m=%d n=%d labelorg=%d",m,n,labelorg);
                if (!gvalid)
                    fprintf(OUTFILE," g=undef");
                else
                {
                    li = 0;
                    for (i = 0, gp = g; i < n; ++i, gp += m)
                        li += setsize(gp,m);
                    if (options.digraph)
                        fprintf(OUTFILE," arcs=%ld",li);
                    else
                        fprintf(OUTFILE," edges=%ld",li/2);
                }
                fprintf(OUTFILE," options=(%cc%ca%cm%cp%cd",
                            PM(options.getcanon),PM(options.writeautoms),
                            PM(options.writemarkers),PM(options.cartesian),
                            PM(options.digraph));
                if (umask & 31)
                    fprintf(OUTFILE," u=%d",umask&31);
                if (options.tc_level > 0)
                    fprintf(OUTFILE," y=%d",options.tc_level);
                if (options.mininvarlevel != 0 || options.maxinvarlevel != 0)
                    fprintf(OUTFILE," k=(%d,%d)",
                                  options.mininvarlevel,options.maxinvarlevel);
                if (options.invararg > 0)
                    fprintf(OUTFILE," K=%d",options.invararg);
                fprintf(OUTFILE,")\n");
                fprintf(OUTFILE,"linelen=%d worksize=%d input_depth=%d",
                                options.linelength,worksize,curfile);
                if (options.invarproc != NILFUNCTION)
                    fprintf(OUTFILE," invarproc=%s",invarprocname);
                if (pvalid)
                    fprintf(OUTFILE,"; %d cell%s",SS(numcells,"","s"));
                else
                    fprintf(OUTFILE,"; 1 cell");
                fprintf(OUTFILE,"\n");
                if (OUTFILE != PROMPTFILE)
                    fprintf(PROMPTFILE,"m=%d n=%d depth=%d labelorg=%d\n",
                            m,n,curfile,labelorg);
                break;

            case '&':   /* list the partition and possibly the quotient */
                if ((d = getc(INFILE)) == '&')
                    doquot = TRUE;
                else
                {
                    ungetc((char)d,INFILE);
                    doquot = FALSE;
                }
                minus = FALSE;
                if (pvalid)
                    putptn(OUTFILE,lab,ptn,0,options.linelength,n);
                else
                    fprintf(OUTFILE,"unit partition\n");
                if (doquot)
                {
                    if (!pvalid)
                        unitptn(lab,ptn,&numcells,n);
                    putquotient(OUTFILE,g,lab,ptn,0,options.linelength,m,n);
                }
                break;

            case 'h':   /* type help information */
                minus = FALSE;
                help(PROMPTFILE);
                break;

            default:    /* illegal command */
                fprintf(ERRFILE,"'%c' is illegal - type 'h' for help\n\n",c);
                flushline(INFILE);
                if (prompt)
                    fprintf(PROMPTFILE,"> ");
                break;

            }  /* end of switch */
}

/*****************************************************************************
*                                                                            *
*  allocg(&gptr,&lptr,&actn,n) tries to find storage a graph of order n and  *
*  possibly an nvector of length n.                                          *
*  If actn > 0, then gptr is assumed to point at an area of actn*m setwords, *
*  where m = ceil(actn/WORDSIZE) and lptr is assumed to point to an area of  *
*  actn ints. That area is reused if it is large enough, or freed if it is   *
*  not large enough.  On return, gptr and lptr point at the required areas,  *
*  and actn is set as above.   Failure is indicated by actn == 0.            *
*  If the lptr argument is NULL, only the larger area is allocated.          *
*                                                                            *
*****************************************************************************/

static void
allocg(gptr,lptr,actn,n)
graph **gptr;
nvector **lptr;
int *actn;
int n;
{
        int m;
        boolean lnull;

        if (*actn >= n)
            return;
        else
        {
            lnull = lptr == (nvector**)NULL;
            if (*actn > 0)
            {
                FREES((char*)*gptr);
                if (!lnull)
                    FREES((char*)*lptr);
            }
            m = (n + WORDSIZE - 1) / WORDSIZE;
            *gptr = (graph*) ALLOCS(n,m*sizeof(setword));
            if (*gptr == (graph*)NULL)
            {
                *actn = 0;
                return;
            }
            if (!lnull)
            {
                *lptr = (nvector*) ALLOCS(n,sizeof(nvector));
                if (*lptr == (nvector*)NULL)
                {
                    *actn = 0;
                    FREES((char*)*gptr);
                    return;
                }
            }
            *actn = n;
        }
}

/*****************************************************************************
*                                                                            *
*  help(f) writes help information to file f.                                *
*                                                                            *
*****************************************************************************/

static void
help(f)
FILE *f;
{
        fprintf(f,
"dreadnaut commands: (# is an integer; '=' is optional)\n");
        fprintf(f,
" n=#,l=#,w=# :  set n,linelen,worksize\n");
        fprintf(f,
" y=#,k=# #,K=#,$=#: set tc_lev,min/maxinvarlev,invararg,labelorg\n");
        fprintf(f,
" f=[...], -f : fix partition, turn off fixing (use | to separate cells)\n");
        fprintf(f,
" g : enter graph; e : edit graph; t,T : type graph; o :type orbits\n");
        fprintf(f,
" b : type canonlab and h; ? : type options, n, #edges, ...\n");
        fprintf(f,
" z : type hashcodes for h;  \"...\" : copy comment;   q : quit\n");
        fprintf(f,
" s=# : make random graph prob=1/#;   _ : complement;  v : type degrees\n");
        fprintf(f,
" j : relabel randomly;   r...; : relabel g according to given permutation\n");
        fprintf(f,
" x : execute nauty;  i : refine only;  %% : do Mathon doubling;  h : help\n");
        fprintf(f,
" >file, >>file, -> : output to file, append to file, revert to stdout\n");
        fprintf(f,
" <file : read from file; & : type partition, && : partition and quotient\n");
        fprintf(f,
" $$ : restore labelorg; ! : ignore rest of line\n");
        fprintf(f,
" @ : h' := h;  # : h=h' test; ## : write h->h' mapping\n");
        fprintf(f,
" *=# : set invarproc (-1=user,0=none,1=twopaths,2=adjtriang,3=triples\n");
        fprintf(f,
"  4=quadruples,5=celltrips,6=cellquads,7=cellquins,8=distances,9=indsets\n");
        fprintf(f,
"  10=cliques,11=cellcliq,12=cellind,13=adjacencies) - also set k and/or K\n");
        fprintf(f,
" u=# : call userprocs (1=node,2=autom,4=level,8=tcell,16=ref; add them)\n");
        fprintf(f,
"Options (precede by +/-):  c : make h;  a : write automs\n");
        fprintf(f,
"   m : write markers;  p : cartesian rep;  d : digraph (or loops present)\n");
        fprintf(f,
"g/e commands:  '#' : add edge;  '-#' : delete edge;  ';' : next vertex\n");
        fprintf(f,
"     '#:' : start vertex #;  '?' : examine neighbours;  '.' : finish.\n");
}

/*****************************************************************************
*                                                                            *
*  usernode(g,lab,ptn,level,numcells,tc,code,m,n) is a simple version of the *
*  procedure named by options.usernodeproc.                                  *
*                                                                            *
*****************************************************************************/

static UPROC
usernode(g,lab,ptn,level,numcells,tc,code,m,n)
graph *g;
nvector *lab,*ptn;
int numcells,m,n,level,tc,code;
{
        register int i;

        for (i = 0; i < level; ++i)
            PUTC('.',OUTFILE);
        if (numcells == n)
            fprintf(OUTFILE,"(n/%d)\n",code);
        else if (tc < 0)
            fprintf(OUTFILE,"(%d/%d)\n",numcells,code);
        else
            fprintf(OUTFILE,"(%d/%d/%d)\n",numcells,code,tc);
        if (firstpath)
            putptn(OUTFILE,lab,ptn,level,options.linelength,n);
        if (numcells == n)
            firstpath = FALSE;
}

/*****************************************************************************
*                                                                            *
*  userautom(count,perm,orbits,numorbits,stabvertex,n) is a simple           *
*  version of the procedure named by options.userautomproc.                  *
*                                                                            *
*****************************************************************************/

static UPROC
userautom(count,perm,orbits,numorbits,stabvertex,n)
permutation *perm;
nvector *orbits;
int count,numorbits,stabvertex,n;
{
        fprintf(OUTFILE,
             "**userautomproc:  count=%d stabvertex=%d numorbits=%d\n",
             count,stabvertex + labelorg,numorbits);
        putorbits(OUTFILE,orbits,options.linelength,n);
}

/*****************************************************************************
*                                                                            *
*  userlevel(lab,ptn,level,orbits,stats,tv,index,tcellsize,numcells,cc,n)    *
*  is a simple version of the procedure named by options.userlevelproc.      *
*                                                                            *
*****************************************************************************/

static UPROC
userlevel(lab,ptn,level,orbits,stats,tv,index,tcellsize,numcells,cc,n)
nvector *lab,*ptn,*orbits;
statsblk *stats;
int level,tv,index,tcellsize,numcells,cc,n;
{
      fprintf(OUTFILE,
            "**userlevelproc:  level=%d tv=%d index=%d tcellsize=%d cc=%d\n",
            level,tv + labelorg,index,tcellsize,cc);
      fprintf(OUTFILE,"    nodes=%ld cells=%d orbits=%d generators=%d\n",
            stats->numnodes,numcells,stats->numorbits,stats->numgenerators);
}
