/* based on geng.c  version 2.7; B D McKay, Jan 2013. */

#define USAGE \
  " [-d#D#] [-Pun] [-lvq] \n\
              [-x#X#] n [mine[:maxe]] [res/mod] [file]"

#define HELPTEXT \
  " Generate all graphs of a specified class.\n\
\n\
      n    : the number of vertices\n\
 mine:maxe : a range for the number of edges\n\
              #:0 means '# or more' except in the case 0:0\n\
   res/mod : only generate subset res out of subsets 0..mod-1\n\
\n\
     -d#   : a lower bound for the minimum degree\n\
     -D#   : a upper bound for the maximum degree\n\
     -v    : display counts by number of edges\n\
     -l    : canonically label output graphs\n\
     -u    : do not output any graphs, just generate and count them\n\
\n\
     -P    : print the adjacency matrix\n\
\n\
     -q    : suppress auxiliary output (except from -v)\n\
\n\
  See program text for much more information.\n"



#ifndef MAXN
#define MAXN 32         /* not more than max(32,WORDSIZE) */
#endif

#if MAXN > 32
 #error "Can't have MAXN greater than 32"
#endif

#define ONE_WORD_SETS
#include "nau0s.h"

#define DYNALLSTAT(type, name, name_sz) \
  static type * name; static size_t name_sz = 0
#define DYNALLOC1(type, name, name_sz, sz, msg) \
  if ((size_t)(sz) > name_sz) \
  { if (name_sz) free(name); name_sz = (sz); \
    if ((name = (type*) calloc(sz, sizeof(type))) == NULL) exit(1); }

#ifndef GTOOLS_H__
#define GTOOLS_H__

#define SIZELEN(n) ((n) <= SMALLN ? 1 : ((n) <= SMALLISHN ? 4 : 8))
/* length of size code in bytes */
#define G6LEN(n) (SIZELEN(n) \
    + ((size_t)(n) / 12) * ((size_t)(n) - 1) \
    + (((size_t)(n) % 12) * ((size_t)(n) - 1) + 11) / 12)

#define BIAS6 63
#define MAXBYTE 126
#define SMALLN 62
#define SMALLISHN 258047
#define C6MASK 63

#define ARG_OK 0
#define ARG_MISSING 1
#define ARG_TOOBIG 2
#define ARG_ILLEGAL 3

#define MAXARG 2000000000L

#define SWBOOLEAN(c, bool) if (sw == c) bool = TRUE;
#define SWINT(c, gotit, val, flag) if (sw == c) \
  { gotit = TRUE; arg_int(&arg, &val, flag); }

#include <time.h>
#define CPUTIME ((double) clock() / CLOCKS_PER_SEC)

#define CATMSG0(fmt) sprintf(msg + strlen(msg), fmt)
#define CATMSG1(fmt, x1) sprintf(msg + strlen(msg), fmt, x1)
#define CATMSG2(fmt, x1, x2) sprintf(msg + strlen(msg), fmt, x1, x2)
#define CATMSG3(fmt, x1, x2, x3) sprintf(msg + strlen(msg), fmt, x1, x2, x3)
#define CATMSG4(fmt, x1, x2, x3, x4) sprintf(msg + strlen(msg), fmt, x1, x2, x3, x4)

#endif /* GTOOLS_H__ */

/************************************************************************/

static void
gt_abort(char *msg)     /* Write message and halt. */
{
  if (msg) fprintf(stderr, "%s", msg);
  exit(1);
}



/**************************************************************************/

static int
longvalue(char **ps, long *l)
{
  boolean neg, pos;
  long sofar, last;
  char *s;

  s = *ps;
  pos = neg = FALSE;
  if (*s == '-') {
    neg = TRUE;
    ++s;
  } else if (*s == '+') {
    pos = TRUE;
    ++s;
  }

  if (*s < '0' || *s > '9') {
    *ps = s;
    return (pos || neg) ? ARG_ILLEGAL : ARG_MISSING;
  }

  sofar = 0;

  for (; *s >= '0' && *s <= '9'; ++s) {
    last = sofar;
    sofar = sofar * 10 + (*s - '0');
    if (sofar < last || sofar > MAXARG) {
      *ps = s;
      return ARG_TOOBIG;
    }
  }
  *ps = s;
  *l = neg ? -sofar : sofar;
  return ARG_OK;
}



/*************************************************************************/

static void
arg_int(char **ps, int *val, char *flag)
{
  int code;
  long longval = 0;

  code = longvalue(ps, &longval);
  *val = longval;
  if (code == ARG_MISSING || code == ARG_ILLEGAL) {
    fprintf(stderr, ">E %s: missing argument value\n", flag);
    gt_abort(NULL);
  } else if (code == ARG_TOOBIG || *val != longval) {
    fprintf(stderr, ">E %s: argument value too large\n", flag);
    gt_abort(NULL);
  }
}



#if MAXN < 32
typedef int xword;   /* Must be as large as MAXN bits, and
                        must be unsigned if equal to MAXN bits */
#else
typedef unsigned int xword;
#endif

static void (*outproc)(FILE*, graph*, int);

static FILE *outfile;           /* file for output graphs */
static boolean verbose;         /* presence of -v */
boolean nautyformat;            /* presence of -n */
boolean printadjmat;            /* presence of -P */
boolean nooutput;               /* presence of -u */
boolean canonise;               /* presence of -l */
boolean quiet;                  /* presence of -q */
statsblk nauty_stats;
static int mindeg, maxdeg, maxn, mine, maxe, mod, res;
#define PRUNEMULT 20   /* bigger -> more even split at greater cost */
static int min_splitlevel, odometer, splitlevel, multiplicity;
static graph gcan[MAXN];

#if MAXN <= 16
static xword xbit[] = { 0x0001, 0x0002, 0x0004, 0x0008,
                        0x0010, 0x0020, 0x0040, 0x0080,
                        0x0100, 0x0200, 0x0400, 0x0800,
                        0x1000, 0x2000, 0x4000, 0x8000 };

#define XNEXTBIT(x) \
  ((x) & 0xFF ? 7 - leftbit[(x) & 0xFF] : 15 - leftbit[((x) >> 8) & 0xFF])
#define XPOPCOUNT(x) (bytecount[((x) >> 8) & 0xFF] + bytecount[(x) & 0xFF])
#elif MAXN <= 24
static xword xbit[] = { 0x000001, 0x000002, 0x000004, 0x000008,
                        0x000010, 0x000020, 0x000040, 0x000080,
                        0x000100, 0x000200, 0x000400, 0x000800,
                        0x001000, 0x002000, 0x004000, 0x008000,
                        0x010000, 0x020000, 0x040000, 0x080000,
                        0x100000, 0x200000, 0x400000, 0x800000 };

#define XNEXTBIT(x) \
  ((x) & 0xFF ? 7 - leftbit[(x) & 0xFF] : \
   (x) & 0xFF00 ? 15 - leftbit[((x) >> 8) & 0xFF] : 23 - leftbit[((x) >> 16) & 0xFF])
#define XPOPCOUNT(x) (bytecount[((x) >> 8) & 0xFF] \
                      + bytecount[((x) >> 16) & 0xFF] + bytecount[(x) & 0xFF])
#else
static xword xbit[] = { 0x00000001, 0x00000002, 0x00000004, 0x00000008,
                        0x00000010, 0x00000020, 0x00000040, 0x00000080,
                        0x00000100, 0x00000200, 0x00000400, 0x00000800,
                        0x00001000, 0x00002000, 0x00004000, 0x00008000,
                        0x00010000, 0x00020000, 0x00040000, 0x00080000,
                        0x00100000, 0x00200000, 0x00400000, 0x00800000,
                        0x01000000, 0x02000000, 0x04000000, 0x08000000,
                        0x10000000, 0x20000000, 0x40000000, 0x80000000 };

#define XNEXTBIT(x) \
  ((x) & 0xFF ? 7 - leftbit[(x) & 0xFF] : \
   (x) & 0xFF00 ? 15 - leftbit[((x) >> 8) & 0xFF] : \
   (x) & 0xFF0000 ? 23 - leftbit[((x) >> 16) & 0xFF] : \
   31 - leftbit[((x) >> 24) & 0xFF])
#define XPOPCOUNT(x) (bytecount[((x) >> 8) & 0xFF] \
                    + bytecount[((x) >> 16) & 0xFF] + \
                    + bytecount[((x) >> 24) & 0xFF] + bytecount[(x) & 0xFF])
#endif

typedef struct {
  int ne, dmax;           /* values used for xlb,xub calculation */
  int xlb, xub;           /* saved bounds on extension degree */
  xword lo, hi;           /* work purposes for orbit calculation */
  xword xstart[MAXN + 1]; /* index into xset[] for each cardinality */
  xword *xset;            /* array of all x-sets in card order */
  xword *xcard;           /* cardinalities of all x-sets */
  xword *xinv;            /* map from x-set to index in xset */
  xword *xorb;            /* min orbit representative */
  xword xlim;             /* number of x-sets in xx[] */
} leveldata;


/* The program is so fast that the count of output graphs can quickly
   overflow a 32-bit integer.  Therefore, we use two long values
   for each count, with a ratio of 10^9 between them.  The macro
   ADDBIG adds a small number to one of these big numbers.
   BIGTODOUBLE converts a big number to a double (approximately).
   SUMBIGS adds a second big number into a first big number.
   SUBBIGS subtracts one big number from a second.
   PRINTBIG prints a big number in decimal.
   ZEROBIG sets the value of a big number to 0.
   ISZEROBIG tests if the value is 0.
   SETBIG sets a big number to a value at most 10^9-1.
   ISEQBIG tests if two big numbers are equal.
   ISASBIG tests if a big number is at least as a value at most 10^9-1.
 */

typedef struct {
  long hi, lo;
} bigint;

#define ZEROBIG(big) big.hi = big.lo = 0L
#define ISZEROBIG(big) (big.lo == 0 && big.hi == 0)
#define SETBIG(big, value) { big.hi = 0L; big.lo = (value); }
#define ADDBIG(big, extra) if ((big.lo += (extra)) >= 1000000000L) \
  { ++big.hi; big.lo -= 1000000000L; }
#define PRINTBIG(file, big) if (big.hi == 0) \
    fprintf(file, "%ld", big.lo); else fprintf(file, "%ld%09ld", big.hi, big.lo)
#define BIGTODOUBLE(big) (big.hi * 1000000000.0 + big.lo)
#define SUMBIGS(big1, big2) { if ((big1.lo += big2.lo) >= 1000000000L) \
                              { big1.lo -= 1000000000L; big1.hi += big2.hi + 1L; } \
                              else big1.hi += big2.hi; }
#define SUBBIGS(big1, big2) { if ((big1.lo -= big2.lo) < 0L) \
                              { big1.lo += 1000000000L; big1.hi -= big2.hi + 1L; } \
                              else big1.hi -= big2.hi; }
/* Note: SUBBIGS must not allow the value to go negative.
   SUMBIGS and SUBBIGS both permit big1 and big2 to be the same bigint. */
#define ISEQBIG(big1, big2) (big1.lo == big2.lo && big1.hi == big2.hi)
#define ISASBIG(big, value) (big.hi > 0 || big.lo >= (value))

static leveldata data[MAXN];      /* data[n] is data for n -> n+1 */
static bigint ecount[1 + MAXN * (MAXN - 1) / 2];  /* counts by number of edges */
static bigint nodes[MAXN];     /* nodes at each level */

#ifdef PLUGIN
#include PLUGIN
#endif

#ifndef OUTPROC

/***********************************************************************/

static void
writenauty(FILE *f, graph *g, int n)
/* write graph g (n vertices) to file f in nauty format */
{
  if (fwrite(&n, sizeof(int), (size_t)1, f) != 1 ||
      fwrite(g, sizeof(setword), (size_t)n, f) != n) {
    fprintf(stderr, ">E writenauty : error on writing file\n");
    exit(2);
  }
}

/***********************************************************************/

static void writeadjmat(FILE *f, graph *g, int n)
{
  int i, j;

  for ( j = 0; j < n; j++ ) {
    graph *gj = GRAPHROW(g, j, m);
    for ( i = 0; i < n; i++ )
      fprintf(f, "%c ", (ISELEMENT(gj, i) ? '*' : ' '));
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

#endif

/**********************************************************************/

static boolean
isbiconnected(graph *g, int n)
/* test if g is biconnected */
{
  int sp, v, w;
  setword sw;
  setword visited;
  int numvis, num[MAXN], lp[MAXN], stack[MAXN];

  if (n <= 2) return FALSE;

  visited = bit[0];
  stack[0] = 0;
  num[0] = 0;
  lp[0] = 0;
  numvis = 1;
  sp = 0;
  v = 0;

  for ( ; ; ) {
    if ( (sw = g[v] & ~visited) != 0 ) {             /* not "==" */
      w = v;
      v = FIRSTBITNZ(sw);             /* visit next child */
      stack[++sp] = v;
      visited |= bit[v];
      lp[v] = num[v] = numvis++;
      /* update the low-point of `v' lp[v] for possible backedges
       * a backedge is an edge from `v' to a previously vertex `w'
       * encountered in the search tree that is adjacent to `w'
       * thus the set of `w' is given by visited & g[v]
       * with the exclusion of the parent `w' */
      sw = g[v] & visited & ~bit[w];
      while (sw) {
        w = FIRSTBITNZ(sw);
        sw &= ~bit[w];
        if (num[w] < lp[v]) lp[v] = num[w];
      }
    } else {
      w = v;                        /* back up to parent */
      if (sp <= 1) return numvis == n;
      v = stack[--sp];
      if (lp[w] >= num[v]) return FALSE;
      /* update the low-point value for the parent `v' of `w' */
      if (lp[w] < lp[v]) lp[v] = lp[w];
    }
  }
}



/**********************************************************************/

static boolean
distinvar(graph *g, int *invar, int n)
/* make distance invariant
   return FALSE if n-1 not maximal else return TRUE */
{
  int w;
  setword workset, frontier;
  setword sofar;
  int inv, d, v;

  for (v = n - 1; v >= 0; --v) {
    inv = 0;
    sofar = frontier = bit[v];
    for (d = 1; frontier != 0; ++d) {
      workset = 0;
      inv += POPCOUNT(frontier) ^ (0x57 + d);
      while (frontier) {
        w = FIRSTBITNZ(frontier);
        frontier ^= bit[w];
        workset |= g[w];
      }
      frontier = workset & ~sofar;
      sofar |= frontier;
    }
    invar[v] = inv;
    if (v < n - 1 && inv > invar[n - 1]) return FALSE;
  }
  return TRUE;
}



/**************************************************************************/

static void
makeleveldata(void)
/* make the level data for each level */
{
  long h;
  int n, nn;
  long ncj;
  leveldata *d;
  xword *xcard, *xinv;
  xword *xset, xw, tttn, nxsets;
  xword cw;
  xword i, j;

  for (n = 1; n < maxn; ++n) {
    nn = maxdeg <= n ? maxdeg : n;
    ncj = nxsets = 1;
    for (j = 1; j <= (xword) nn; ++j) {
      ncj = (ncj * (n - j + 1)) / j;
      nxsets += ncj;
    }
    tttn = 1L << n;

    d = &data[n];

    d->ne = d->dmax = d->xlb = d->xub = -1;

    d->xset = xset = (xword*) calloc(nxsets, sizeof(xword));
    d->xcard = xcard = (xword*) calloc(nxsets, sizeof(xword));
    d->xinv = xinv = (xword*) calloc(tttn, sizeof(xword));
    d->xorb = (xword*) calloc(nxsets, sizeof(xword));

    if (xset == NULL || xcard == NULL || xinv == NULL || d->xorb == NULL) {
      fprintf(stderr, ">E bcgen: calloc failed in makeleveldata()\n");
      exit(2);
    }

    j = 0;

    for (i = 0;; ++i) {
      if ((h = XPOPCOUNT(i)) <= maxdeg) {
        xset[j] = i;
        xcard[j] = h;
        ++j;
      }
      if (i == (xword)((1L << n) - 1)) break;
    }

    if (j != nxsets) {
      fprintf(stderr, ">E bcgen: j=%u mxsets=%u\n",
              j, (unsigned)nxsets);
      exit(2);
    }

    for ( h = 1; h < (long) nxsets; )
      h = h * 3 + 1;

    do {
      for (i = h; i < nxsets; ++i) {
        xw = xset[i];
        cw = xcard[i];
        for (j = i; xcard[j - h] > cw ||
             (xcard[j - h] == cw && xset[j - h] > xw); ) {
          xset[j] = xset[j - h];
          xcard[j] = xcard[j - h];
          if ((j -= h) < (xword) h) break;
        }
        xset[j] = xw;
        xcard[j] = cw;
      }
      h /= 3;
    } while (h > 0);

    for (i = 0; i < nxsets; ++i) xinv[xset[i]] = i;

    d->xstart[0] = 0;
    for (i = 1; i < nxsets; ++i)
      if (xcard[i] > xcard[i - 1]) d->xstart[xcard[i]] = i;
    d->xstart[xcard[nxsets - 1] + 1] = nxsets;
  }
}



/**************************************************************************/

static void
geng_userautomproc(int count, int *p, int *orbits,
                   int numorbits, int stabvertex, int n)
/* form orbits on powerset of VG
   called by nauty;  operates on data[n] */
{
  xword i, j1, j2, moved, pi, pxi;
  xword lo, hi;
  xword *xorb, *xinv, *xset, w;

  (void) orbits; (void) numorbits; (void) stabvertex;
  xorb = data[n].xorb;
  xset = data[n].xset;
  xinv = data[n].xinv;
  lo = data[n].lo;
  hi = data[n].hi;

  if (count == 1)                           /* first automorphism */
    for (i = lo; i < hi; ++i) xorb[i] = i;

  moved = 0;
  for (i = 0; i < (xword) n; ++i)
    if ((xword) p[i] != i) moved |= xbit[i];

  for (i = lo; i < hi; ++i) {
    if ((w = xset[i] & moved) == 0) continue;
    pxi = xset[i] & ~moved;
    while (w) {
      j1 = XNEXTBIT(w);
      w ^= xbit[j1];
      pxi |= xbit[p[j1]];
    }
    pi = xinv[pxi];

    j1 = xorb[i];
    while (xorb[j1] != j1) j1 = xorb[j1];
    j2 = xorb[pi];
    while (xorb[j2] != j2) j2 = xorb[j2];

    if (j1 < j2) xorb[j2] = xorb[i] = xorb[pi] = j1;
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
refinex(graph *g, int *lab, int *ptn, int level, int *numcells,
        int *count, set *active, boolean goodret, int *code, int m, int n)
{
  int i, c1, c2, labc1;
  setword x, lact;
  int split1, split2, cell1, cell2;
  int cnt, bmin, bmax;
  set *gptr;
  setword workset;
  int workperm[MAXN];
  int bucket[MAXN + 2];

  (void) level; (void) m;
  if (n == 1) {
    *code = 1;
    return;
  }

  *code = 0;
  lact = *active;

  while (*numcells < n && lact) {
    TAKEBIT(split1, lact);

    for (split2 = split1; ptn[split2] > 0; ++split2) ;
    if (split1 == split2) {         /* trivial splitting cell */
      gptr = GRAPHROW(g, lab[split1], 1);
      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > 0; ++cell2) {
        }
        if (cell1 == cell2) continue;

        c1 = cell1;
        c2 = cell2;
        while (c1 <= c2) {
          labc1 = lab[c1];
          if (ISELEMENT1(gptr, labc1))
            ++c1;
          else {
            lab[c1] = lab[c2];
            lab[c2] = labc1;
            --c2;
          }
        }
        if (c2 >= cell1 && c1 <= cell2) {
          ptn[c2] = 0;
          ++*numcells;
          lact |= bit[c1];
        }
      }
    } else {        /* nontrivial splitting cell */
      workset = 0;
      for (i = split1; i <= split2; ++i) workset |= bit[lab[i]];

      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > 0; ++cell2) {
        }
        if (cell1 == cell2) continue;
        i = cell1;
        if ((x = workset & g[lab[i]]) != 0) cnt = POPCOUNT(x);
        else cnt = 0;
        count[i] = bmin = bmax = cnt;
        bucket[cnt] = 1;
        while (++i <= cell2) {
          if ((x = workset & g[lab[i]]) != 0)
            cnt = POPCOUNT(x);
          else
            cnt = 0;

          while (bmin > cnt) bucket[--bmin] = 0;
          while (bmax < cnt) bucket[++bmax] = 0;
          ++bucket[cnt];
          count[i] = cnt;
        }
        if (bmin == bmax) continue;
        c1 = cell1;
        for (i = bmin; i <= bmax; ++i)
          if (bucket[i]) {
            c2 = c1 + bucket[i];
            bucket[i] = c1;
            if (c1 != cell1) {
              lact |= bit[c1];
              ++*numcells;
            }
            if (c2 <= cell2) ptn[c2 - 1] = 0;
            c1 = c2;
          }
        for (i = cell1; i <= cell2; ++i)
          workperm[bucket[count[i]]++] = lab[i];
        for (i = cell1; i <= cell2; ++i) lab[i] = workperm[i];
      }
    }

    if (ptn[n - 2] == 0) {
      if (lab[n - 1] == n - 1) {
        *code = 1;
        if (goodret) return;
      } else {
        *code = -1;
        return;
      }
    } else {
      i = n - 1;
      while (TRUE) {
        if (lab[i] == n - 1) break;
        --i;
        if (ptn[i] == 0) {
          *code = -1;
          return;
        }
      }
    }
  }
}



/**************************************************************************/

static void
makecanon(graph *g, graph *gcan, int n)
/* gcan := canonise(g) */
{
  int lab[MAXN], ptn[MAXN], orbits[MAXN];
  static DEFAULTOPTIONS_GRAPH(options);

  options.getcanon = TRUE;

  nauty(g, lab, ptn, NULL, orbits, &options, &nauty_stats,
        dnwork, 2 * 60, 1, n, gcan);
}



/**************************************************************************/

static boolean
accept1(graph *g, int n, xword x, graph *gx, int *deg, boolean *rigid)
/* decide if n in theta(g+x) -  version for n+1 < maxn */
{
  int i;
  int lab[MAXN], ptn[MAXN], orbits[MAXN];
  int count[MAXN];
  graph h[MAXN];
  xword xw;
  int nx, numcells, code;
  int i0, i1, degn;
  set active[MAXM];
  statsblk stats;
  static DEFAULTOPTIONS_GRAPH(options);

  nx = n + 1;
  for (i = 0; i < n; ++i) gx[i] = g[i];
  gx[n] = 0;
  deg[n] = degn = XPOPCOUNT(x);

  /* augment the graph with a new vertex `n'
   * which is adjacent to vertices in `x' (as bits) */
  xw = x;
  while (xw) {
    i = XNEXTBIT(xw);
    xw ^= xbit[i];
    gx[i] |= bit[n];
    gx[n] |= bit[i];
    ++deg[i];
  }

  i0 = 0;
  i1 = n;
  for (i = 0; i < nx; ++i) {
    if (deg[i] == degn) lab[i1--] = i;
    else lab[i0++] = i;
    ptn[i] = 1;
  }
  ptn[n] = 0;
  if (i0 == 0) {
    numcells = 1;
    active[0] = bit[0];
  } else {
    numcells = 2;
    active[0] = bit[0] | bit[i1 + 1];
    ptn[i1] = 0;
  }
  refinex(gx, lab, ptn, 0, &numcells, count, active, FALSE, &code, 1, nx);

  if (code < 0) return FALSE;

  if (numcells == nx) {
    *rigid = TRUE;
    return TRUE;
  }

  options.getcanon = TRUE;
  options.defaultptn = FALSE;
  options.userautomproc = &geng_userautomproc;

  active[0] = 0;
  nauty(gx, lab, ptn, active, orbits, &options, &stats,
      dnwork, 2 * 60, 1, nx, h);

  if (orbits[lab[n]] == orbits[n]) {
    *rigid = stats.numorbits == nx;
    return TRUE;
  } else
    return FALSE;
}



/**************************************************************************/

static boolean
accept2(graph *g, int n, xword x, graph *gx, int *deg, boolean nuniq)
/* decide if n in theta(g+x)  --  version for n+1 == maxn */
{
  int i;
  int lab[MAXN], ptn[MAXN], orbits[MAXN];
  int degx[MAXN], invar[MAXN];
  setword vmax, gv;
  int qn, qv;
  int count[MAXN];
  xword xw;
  int nx, numcells, code;
  int degn, i0, i1, j, j0, j1;
  set active[MAXM];
  static DEFAULTOPTIONS_GRAPH(options);
  boolean cheapacc;

  nx = n + 1;
  for (i = 0; i < n; ++i) {
    gx[i] = g[i];
    degx[i] = deg[i];
  }
  gx[n] = 0;
  degx[n] = degn = XPOPCOUNT(x);

  xw = x;
  while (xw) {
    i = XNEXTBIT(xw);
    xw ^= xbit[i];
    gx[i] |= bit[n];
    gx[n] |= bit[i];
    ++degx[i];
  }

  options.defaultptn = TRUE;
  if (nuniq) goto FIND_CANON;

  i0 = 0;
  i1 = n;
  for (i = 0; i < nx; ++i) {
    if (degx[i] == degn) lab[i1--] = i;
    else lab[i0++] = i;
    ptn[i] = 1;
  }
  ptn[n] = 0;
  options.defaultptn = FALSE;

  if (i0 == 0) {
    numcells = 1;
    active[0] = bit[0];

    if (!distinvar(gx, invar, nx)) return FALSE;
    qn = invar[n];
    j0 = 0;
    j1 = n;
    while (j0 <= j1) {
      j = lab[j0];
      qv = invar[j];
      if (qv < qn)
        ++j0;
      else {
        lab[j0] = lab[j1];
        lab[j1] = j;
        --j1;
      }
    }
    if (j0 > 0) {
      if (j0 == n) goto FIND_CANON;
      ptn[j1] = 0;
      ++numcells;
      active[0] |= bit[j0];
    }
  } else {
    numcells = 2;
    ptn[i1] = 0;
    active[0] = bit[0] | bit[i1 + 1];

    vmax = 0;
    for (i = i1 + 1; i < nx; ++i) vmax |= bit[lab[i]];

    gv = gx[n] & vmax;
    qn = POPCOUNT(gv);

    j0 = i1 + 1;
    j1 = n;
    while (j0 <= j1) {
      j = lab[j0];
      gv = gx[j] & vmax;
      qv = POPCOUNT(gv);
      if (qv > qn)
        return FALSE;
      else if (qv < qn)
        ++j0;
      else {
        lab[j0] = lab[j1];
        lab[j1] = j;
        --j1;
      }
    }
    if (j0 > i1 + 1) {
      if (j0 == n) goto FIND_CANON;
      ptn[j1] = 0;
      ++numcells;
      active[0] |= bit[j0];
    }
  }

  refinex(gx, lab, ptn, 0, &numcells, count, active, TRUE, &code, 1, nx);

  if (code < 0) return FALSE;

  cheapacc = FALSE;
  if (code > 0 || numcells >= nx - 4)
    cheapacc = TRUE;
  else if (numcells == nx - 5) {
    for (j1 = nx - 2; j1 >= 0 && ptn[j1] > 0; --j1) {
    }
    if (nx - j1 != 5) cheapacc = TRUE;
  } else {
    j1 = nx;
    j0 = 0;
    for (i1 = 0; i1 < nx; ++i1) {
      --j1;
      if (ptn[i1] > 0) {
        ++j0;
        while (ptn[++i1] > 0) {
        }
      }
    }
    if (j1 <= j0 + 1) cheapacc = TRUE;
  }

  if (cheapacc) goto FIND_CANON;

  options.getcanon = TRUE;
  //options.defaultptn = FALSE;

  active[0] = 0;
  nauty(gx, lab, ptn, active, orbits, &options, &nauty_stats,
      dnwork, 2 * 60, 1, nx, gcan);

  if (orbits[lab[n]] == orbits[n]) {
    goto FIND_CANON;
  } else
    return FALSE;

FIND_CANON:
  if (canonise) {
    makecanon(gx, gcan, nx);
  } else { /* only compute the size of the automorphism group */
    nauty(gx, lab, ptn, NULL, orbits, &options, &nauty_stats,
        dnwork, 2 * 60, 1, nx, gcan);
  }
  return TRUE;
}



/**************************************************************************/

static void
xbnds(int n, int ne, int dmax)
/* find bounds on extension degree;  store answer in data[*].*  */
{
  int xlb, xub, d, nn, m, xc;

  xlb = n == 1 ? 0 : (dmax > (2 * ne + n - 2) / (n - 1) ?
                      dmax : (2 * ne + n - 2) / (n - 1));
  xub = n < maxdeg ? n : maxdeg;

  for (xc = xub; xc >= xlb; --xc) {
    d = xc;
    m = ne + d;
    for (nn = n + 1; nn < maxn; ++nn) {
      if (d < (2 * m + nn - 2) / (nn - 1)) d = (2 * m + nn - 2) / (nn - 1);
      m += d;
    }
    if (d > maxdeg || m > maxe) xub = xc - 1;
    else break;
  }

  if (ne + xlb < mine)
    for (xc = xlb; xc <= xub; ++xc) {
      m = ne + xc;
      for (nn = n + 1; nn < maxn; ++nn)
        m += maxdeg < nn ? maxdeg : nn;
      if (m < mine) xlb = xc + 1;
      else break;
    }

  data[n].ne = ne;
  data[n].dmax = dmax;
  data[n].xlb = xlb;
  data[n].xub = xub;
}



/**************************************************************************/

static void
genextend(graph *g, int n, int *deg, int ne, boolean rigid, int xlb, int xub)
/* extend from n to n+1 -- version for general graphs */
{
  xword x, d, dlow;
  xword *xset, *xcard, *xorb;
  xword i, imin, imax;
  int nx, xc, j, dmax, dcrit;
  int xlbx, xubx;
  graph gx[MAXN];
  int degx[MAXN];
  boolean rigidx;

  ADDBIG(nodes[n], 1);

  nx = n + 1;
  dmax = deg[n - 1];
  dcrit = mindeg - maxn + n;
  d = dlow = 0;
  for (i = 0; i < (xword) n; ++i) {
    if (deg[i] == dmax) d |= xbit[i];
    if (deg[i] == dcrit) dlow |= xbit[i];
  }

  if (xlb == dmax && XPOPCOUNT(d) + dmax > n) ++xlb;
  if (nx == maxn && xlb < mindeg) xlb = mindeg;
  if (xlb > xub) return;

  imin = data[n].xstart[xlb];
  imax = data[n].xstart[xub + 1];
  xset = data[n].xset;
  xcard = data[n].xcard;
  xorb = data[n].xorb;

  if (nx == maxn) {
    for (i = imin; i < imax; ++i) {
      if (!rigid && xorb[i] != i) continue;
      x = xset[i];
      xc = xcard[i];
      if (xc == dmax && (x & d) != 0) continue;
      if ((dlow & ~x) != 0) continue;

      if ( accept2(g, n, x, gx, deg,
                  xc > dmax + 1 || (xc == dmax + 1 && (x & d) == 0))
          && isbiconnected(gx, nx) )
      {
        ADDBIG(ecount[ne + xc], 1);
        OPTCALL(outproc)(outfile, canonise ? gcan : gx, nx);
      }
    }
  } else {
    for (i = imin; i < imax; ++i) {
      if (!rigid && xorb[i] != i) continue;
      x = xset[i];
      xc = xcard[i];
      if (xc == dmax && (x & d) != 0) continue;
      if ((dlow & ~x) != 0) continue;
      if (nx == splitlevel) {
        if (odometer-- != 0) continue;
        odometer = mod - 1;
      }

      for (j = 0; j < n; ++j) degx[j] = deg[j];
      if (data[nx].ne != ne + xc || data[nx].dmax != xc)
        xbnds(nx, ne + xc, xc);
      xlbx = data[nx].xlb;
      xubx = data[nx].xub;
      if (xlbx > xubx) continue;

      data[nx].lo = data[nx].xstart[xlbx];
      data[nx].hi = data[nx].xstart[xubx + 1];
      if (accept1(g, n, x, gx, degx, &rigidx)) {
        genextend(gx, nx, degx, ne + xc, rigidx, xlbx, xubx);
      }
    }
  }

  if (n == splitlevel - 1 && n >= min_splitlevel
      && ISASBIG(nodes[n], multiplicity))
    --splitlevel;
}



/**************************************************************************/
/**************************************************************************/

int
main(int argc, char *argv[])
{
  char *arg;
  boolean badargs, gote, gotmr, gotf, gotd, gotD, gotx, gotX;
  boolean secret, safe;
  char *outfilename, sw;
  int i, j, argnum;
  graph g[1];
  int tmaxe, deg[1];
  bigint nout;
  int splitlevinc;
  double t1, t2;
  char msg[201];

  if (argc > 1 && (strcmp(argv[1], "-help") == 0
               ||  strcmp(argv[1], "/?") == 0
               ||  strcmp(argv[1], "--help") == 0)) {
    printf("\nUsage: %s" USAGE "\n\n" HELPTEXT, argv[0]);
    return 0;
  }
  nauty_check(WORDSIZE, 1, MAXN, NAUTYVERSIONID);

  if (MAXN > 32 || MAXN > WORDSIZE || MAXN > 8 * sizeof(xword)) {
    fprintf(stderr, "bcgen: incompatible MAXN %d, WORDSIZE %d, or xword %d\n",
        MAXN, WORDSIZE, (int) sizeof(xword));
    fprintf(stderr, "--See notes in program source\n");
    exit(1);
  }

  badargs = FALSE;
  verbose = FALSE;
  nautyformat = FALSE;
  printadjmat = FALSE;
  nooutput = FALSE;
  canonise = FALSE;
  outfilename = NULL;
  secret = FALSE;
  safe = FALSE;

  maxdeg = MAXN;
  mindeg = 0;

  gotX = gotx = gotd = gotD = gote = gotmr = gotf = FALSE;

  argnum = 0;
  for (j = 1; !badargs && j < argc; ++j) {
    arg = argv[j];
    if (arg[0] == '-' && arg[1] != '\0') {
      ++arg;
      while (*arg != '\0') {
        sw = *arg++;
        SWBOOLEAN('n', nautyformat)
        else SWBOOLEAN('P', printadjmat)
        else SWBOOLEAN('v', verbose)
        else SWBOOLEAN('u', nooutput)
        else SWBOOLEAN('l', canonise)
        else SWBOOLEAN('q', quiet)
        else SWBOOLEAN('$', secret)
        else SWBOOLEAN('S', safe)
        else SWINT('d', gotd, mindeg, "-d")
        else SWINT('D', gotD, maxdeg, "-D")
        else SWINT('x', gotx, multiplicity, "-x")
        else SWINT('X', gotX, splitlevinc, "-X")
#ifdef PLUGIN_SWITCHES
        PLUGIN_SWITCHES
#endif
        else badargs = TRUE;
      }
    } else if (arg[0] == '-' && arg[1] == '\0')
      gotf = TRUE;
    else {
      if (argnum == 0) {
        if (sscanf(arg, "%d", &maxn) != 1) badargs = TRUE;
        ++argnum;
      } else if (gotf)
        badargs = TRUE;
      else {
        if (!gotmr) {
          if (sscanf(arg, "%d/%d", &res, &mod) == 2) {
            gotmr = TRUE;
            continue;
          }
        }
        if (!gote) {
          if (sscanf(arg, "%d:%d", &mine, &maxe) == 2
              || sscanf(arg, "%d-%d", &mine, &maxe) == 2) {
            gote = TRUE;
            if (maxe == 0 && mine > 0) maxe = MAXN * (MAXN - 1) / 2;
            continue;
          } else if (sscanf(arg, "%d", &mine) == 1) {
            gote = TRUE;
            maxe = mine;
            continue;
          }
        }
        if (!gotf) {
          outfilename = arg;
          gotf = TRUE;
          continue;
        }
      }
    }
  }

  if (argnum == 0)
    badargs = TRUE;
  else if (maxn < 1 || maxn > MAXN) {
    fprintf(stderr, ">E bcgen: n must be in the range 1..%d\n", MAXN);
    badargs = TRUE;
  }

  if (!gotmr) {
    mod = 1;
    res = 0;
  }

  if (!gote) {
    mine = 0;
    maxe = (maxn * maxn - maxn) / 2;
  }

  /* the degree of a vertex of a biconnected graph is at least 2 */
  if (mindeg < 2 && maxn > 2) mindeg = 2;
  if (maxdeg >= maxn) maxdeg = maxn - 1;
  if (maxe > maxn * maxdeg / 2) maxe = maxn * maxdeg / 2;
  if (maxdeg > maxe) maxdeg = maxe;
  if (mindeg < 0) mindeg = 0;
  if (mine < (maxn * mindeg + 1) / 2) mine = (maxn * mindeg + 1) / 2;

  if (!badargs && (mine > maxe || maxe < 0 || maxdeg < 0)) {
    fprintf(stderr,
            ">E bcgen: impossible mine,maxe,mindeg,maxdeg values\n");
    badargs = TRUE;
  }

  if (!badargs && (res < 0 || res >= mod)) {
    fprintf(stderr, ">E bcgen: must have 0 <= res < mod\n");
    badargs = TRUE;
  }

  if (mine < maxn) mine = maxn;

  if (badargs) {
    fprintf(stderr, ">E Usage: %s\n", USAGE);
    fprintf(stderr, "   Use %s -help to see more detailed instructions.\n", argv[0]);
    return -1;
  }

  if ((printadjmat != 0) + (nooutput != 0) + (nautyformat != 0) > 1)
    gt_abort(">E bcgen: -Pun are incompatible\n");

#ifdef OUTPROC
  outproc = OUTPROC;
#else
  if (nautyformat) outproc = writenauty;
  else if (nooutput) outproc = NULL;
  else if (printadjmat) outproc = writeadjmat;
#endif

#ifdef PLUGIN_INIT
  PLUGIN_INIT
#endif

  for (i = 0; i <= maxe; ++i) ZEROBIG(ecount[i]);
  for (i = 0; i < maxn; ++i) ZEROBIG(nodes[i]);

  if (nooutput)
    outfile = stdout;
  else if (!gotf || outfilename == NULL) {
    outfilename = "stdout";
    outfile = stdout;
  } else if ((outfile = fopen(outfilename,
                              nautyformat ? "wb" : "w")) == NULL) {
    fprintf(stderr,
            ">E bcgen: can't open %s for writing\n", outfilename);
    gt_abort(NULL);
  }

  tmaxe = (maxn * maxn - maxn) / 2;

  if (safe) ++tmaxe;

  if (maxe > tmaxe) maxe = tmaxe;

  if (gotx) {
    if (multiplicity < 3 * mod || multiplicity > 999999999)
      gt_abort(">E bcgen: -x value must be in [3*mod,10^9-1]\n");
  } else
    multiplicity = PRUNEMULT * mod;

  if (!gotX) splitlevinc = 0;

  if (!quiet) {
    msg[0] = '\0';
    if (strlen(argv[0]) > 75)
      fprintf(stderr, ">A %s", argv[0]);
    else
      CATMSG1(">A %s", argv[0]);

    CATMSG1(" -%s", canonise     ? "l" : "");
    if (mod > 1)
      CATMSG2("X%dx%d", splitlevinc, multiplicity);
    CATMSG4("d%dD%d n=%d e=%d", mindeg, maxdeg, maxn, mine);
    if (maxe > mine) CATMSG1("-%d", maxe);
    if (mod > 1) CATMSG2(" class=%d/%d", res, mod);
    CATMSG0("\n");
    fputs(msg, stderr);
    fflush(stderr);
  }

  g[0] = 0;
  deg[0] = 0;

  t1 = CPUTIME;

  if (maxn == 1) {
    if (res == 0) {
      ADDBIG(ecount[0], 1);
      OPTCALL(outproc)(outfile, g, 1);
    }
  } else {
    makeleveldata();

    if (maxn >= 14 && mod > 1) splitlevel = maxn - 4;
    else if (maxn >= 6 && mod > 1) splitlevel = maxn - 3;
    else splitlevel = -1;

    if (splitlevel > 0) splitlevel += splitlevinc;
    if (splitlevel > maxn - 1) splitlevel = maxn - 1;
    if (splitlevel < 3) splitlevel = -1;

    min_splitlevel = 6;
    odometer = secret ? -1 : res;

    if (maxe >= mine &&
        (mod <= 1 || (mod > 1 && (splitlevel > 2 || res == 0)))) {
      xbnds(1, 0, 0);
      genextend(g, 1, deg, 0, TRUE, data[1].xlb, data[1].xub);
    }
  }
  t2 = CPUTIME;

  ZEROBIG(nout);
  for (i = 0; i <= maxe; ++i) SUMBIGS(nout, ecount[i]);

  if (verbose) {
    for (i = 0; i <= maxe; ++i)
      if (!ISZEROBIG(ecount[i])) {
        fprintf(stderr, ">C ");
        PRINTBIG(stderr, ecount[i]);
        fprintf(stderr, " graphs with %d edges\n", i);
      }
  }

  if (!quiet) {
    fprintf(stderr, ">Z ");
    PRINTBIG(stderr, nout);
    fprintf(stderr, " graphs generated in %3.2f sec\n", t2 - t1);
  }

  for (i = 1; i < maxn; ++i) {
    free(data[i].xorb);
    free(data[i].xset);
    free(data[i].xinv);
    free(data[i].xcard);
  }

#ifdef PLUGIN_DONE
  PLUGIN_DONE
#endif
  return 0;
}

