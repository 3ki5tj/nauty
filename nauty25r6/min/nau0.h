/* combine all essential nauty functions and definitions
 *  nauty.h nautil.c sorttemplates.c nauty.c naugraph.c
 * define NAU_SCHREIER to enable shreier code, disabled by default
 * source code for schreier is not included here
 * This file was edited manually */
#ifndef NAU0_H__
#define NAU0_H__


#ifdef __INTEL_COMPILER
  #pragma warning push
/* 1418: external function
 * 981: evaluated unspecified order
 * 161:  unrecognized pragma
 * 869: unused variables */
  #pragma warning disable 1418 981 161 869
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
  #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
    /* diagnostic push and pop are added in GCC 4.6 */
    #pragma GCC diagnostic push
  #endif
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#ifndef INLINE
#define INLINE __inline
#endif

#ifndef RESTRICT
#if defined(_MSC_VER) && (_MSC_VER < 1400)
#define RESTRICT
#else
#define RESTRICT __restrict
#endif
#endif

#ifndef LONGLONG
#if defined(_MSC_VER)
#define LONGLONG __int64
#else
#define LONGLONG long long
#endif
#endif



#ifndef NAUTY_H__
#define NAUTY_H__

#define ONE_WORD_SETS

/* The parts between the ==== lines are modified by configure when
   creating nauty.h out of nauty-h.in.  If configure is not being used,
   it is necessary to check they are correct.
   ====================================================================*/

/* Check whether various headers or options are available */
#define HAVE_SYSTYPES_H  1    /* <sys/types.h> */
#ifndef SIZEOF_INT
#define SIZEOF_INT 4
#endif
#ifndef SIZEOF_LONG
#define SIZEOF_LONG 8
#endif
#ifndef SIZEOF_LONG_LONG
#define SIZEOF_LONG_LONG 8   /* 0 if nonexistent */
#endif


/* Support of gcc extensions __builtin_clz, __builtin_clzl, __builtin_clzll */
#define HAVE_CLZ 0
#define HAVE_CLZL 0
#define HAVE_CLZLL 0

/*==================================================================*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined(__cray) || defined(__cray__) || defined(cray)
#define SYS_CRAY        /* Cray UNIX, portable or standard C */
#endif

#if defined(__unix) || defined(__unix__) || defined(unix)
#define SYS_UNIX
#endif

/*****************************************************************************
*                                                                            *
*    AUTHOR: Brendan D. McKay                                                *
*            Research School of Computer Science                             *
*            Australian National University                                  *
*            Canberra, ACT 0200, Australia                                   *
*            phone:  +61 2 6125 3845    fax:  +61 2 6125 0010                *
*            email:  bdm@cs.anu.edu.au                                       *
*                                                                            *
*   Nauty is copyright (1984-2013) Brendan McKay.  All rights reserved.      *
*   Permission
*   is hereby given for use and/or distribution with the exception of        *
*   sale for profit or application with nontrivial military significance.    *
*   You must not remove this copyright notice, and you must document any     *
*   changes that you make to this program.                                   *
*   This software is subject to this copyright only, irrespective of         *
*   any copyright attached to any package of which this is a part.           *
*                                                                            *
*   This program is only provided "as is".  No responsibility will be taken  *
*   by the author, his employer or his pet rabbit* for any misfortune which  *
*   befalls you because of its use.  I don't think it will delete all your   *
*   files, burn down your computer room or turn your children against you,   *
*   but if it does: stiff cheddar.  On the other hand, I very much welcome   *
*   bug reports, or at least I would if there were any bugs.                 *
*                                                       * RIP, 1989          *
*   Traces is copyright Adolfo Piperno (2011-).                              *
*                                                                            *
*   Reference manual:                                                        *
*     B. D. McKay and A. Piperno, nauty User's Guide (Version 2.5),          *
*         http://pallini.di.uniroma1.it                                      *
*         http://cs.anu.edu.au/~bdm/nauty/                                   *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       10-Nov-87 : final changes for version 1.2                            *
*        5-Dec-87 : renamed to version 1.3 (no changes to this file)         *
*       28-Sep-88 : added PC Turbo C support, making version 1.4             *
*       23-Mar-89 : changes for version 1.5 :                                *
*                   - reworked M==1 code                                     *
*                   - defined NAUTYVERSION string                            *
*                   - made NAUTYH_READ to allow this file to be read twice   *
*                   - added optional ANSI function prototypes                *
*                   - added validity check for WORDSIZE                      *
*                   - added new fields to optionblk structure                *
*                   - updated DEFAULTOPTIONS to add invariants fields        *
*                   - added (set*) cast to definition of GRAPHROW            *
*                   - added definition of ALLOCS and FREES                   *
*       25-Mar-89 : - added declaration of new function doref()              *
*                   - added UNION macro                                      *
*       29-Mar-89 : - reduced the default MAXN for small machines            *
*                   - removed OUTOFSPACE (no longer used)                    *
*                   - added SETDIFF and XOR macros                           *
*        2-Apr-89 : - extended statsblk structure                            *
*        4-Apr-89 : - added IS_* macros                                      *
*                   - added ERRFILE definition                               *
*                   - replaced statsblk.outofspace by statsblk.errstatus     *
*        5-Apr-89 : - deleted definition of np2vector (no longer used)       *
*                   - introduced EMPTYSET macro                              *
*       12-Apr-89 : - eliminated MARK, UNMARK and ISMARKED (no longer used)  *
*       18-Apr-89 : - added MTOOBIG and CANONGNIL                            *
*       12-May-89 : - made ISELEM1 and ISELEMENT return 0 or 1               *
*        2-Mar-90 : - added EXTPROC macro and used it                        *
*       12-Mar-90 : - added SYS_CRAY, with help from N. Sloane and A. Grosky *
*                   - added dummy groupopts field to optionblk               *
*                   - select some ANSI things if __STDC__ exists             *
*       20-Mar-90 : - changed default MAXN for Macintosh versions            *
*                   - created SYS_MACTHINK for Macintosh THINK compiler      *
*       27-Mar-90 : - split SYS_MSDOS into SYS_PCMS4 and SYS_PCMS5           *
*       13-Oct-90 : changes for version 1.6:                                 *
*                   - fix definition of setword for WORDSIZE==64             *
*       14-Oct-90 : - added SYS_APOLLO version to avoid compiler bug         *
*       15-Oct-90 : - improve detection of ANSI conformance                  *
*       17-Oct-90 : - changed temp name in EMPTYSET to avoid A/UX bug        *
*       16-Apr-91 : changes for version 1.7:                                 *
*                   - made version SYS_PCTURBO use free(), not cfree()       *
*        2-Sep-91 : - noted that SYS_PCMS5 also works for Quick C            *
*                   - moved MULTIPLY to here from nauty.c                    *
*       12-Jun-92 : - changed the top part of this comment                   *
*       27-Aug-92 : - added version SYS_IBMC, thanks to Ivo Duentsch         *
*        5-Jun-93 : - renamed to version 1.7+, only change in naututil.h     *
*       29-Jul-93 : changes for version 1.8:                                 *
*                   - fixed error in default 64-bit version of FIRSTBIT      *
*                     (not used in any version before ALPHA)                 *
*                   - installed ALPHA version (thanks to Gordon Royle)       *
*                   - defined ALLOCS,FREES for SYS_IBMC                      *
*        3-Sep-93 : - make calloc void* in ALPHA version                     *
*       17-Sep-93 : - renamed to version 1.9,                                *
*                        changed only dreadnaut.c and nautinv.c              *
*       24-Feb-94 : changes for version 1.10:                                *
*                   - added version SYS_AMIGAAZT, thanks to Carsten Saager   *
*                     (making 1.9+)                                          *
*       19-Apr-95 : - added prototype wrapper for C++,                       *
*                     thanks to Daniel Huson                                 *
*        5-Mar-96 : - added SYS_ALPHA32 version (32-bit setwords on Alpha)   *
*       13-Jul-96 : changes for version 2.0:                                 *
*                   - added dynamic allocation                               *
*                   - ERRFILE must be defined                                *
*                   - added FLIPELEM1 and FLIPELEMENT macros                 *
*       13-Aug-96 : - added SWCHUNK? macros                                  *
*                   - added TAKEBIT macro                                    *
*       28-Nov-96 : - include sys/types.h if not ANSI (tentative!)           *
*       24-Jan-97 : - and stdlib.h if ANSI                                   *
*                   - removed use of cfree() from UNIX variants              *
*       25-Jan-97 : - changed options.getcanon from boolean to int           *
*                     Backwards compatibility is ok, as boolean and int      *
*                     are the same.  Now getcanon=2 means to get the label   *
*                     and not care about the group.  Sometimes faster.       *
*        6-Feb-97 : - Put in #undef for FALSE and TRUE to cope with          *
*                     compilers that illegally predefine them.               *
*                   - declared nauty_null and nautil_null                    *
*        2-Jul-98 : - declared ALLBITS                                       *
*       21-Oct-98 : - allow WORDSIZE==64 using unsigned long long            *
*                   - added BIGNAUTY option for really big graphs            *
*       11-Dec-99 : - made bit, leftbit and bytecount static in each file    *
*        9-Jan-00 : - declared nauty_check() and nautil_check()              *
*       12-Feb-00 : - Used #error for compile-time checks                    *
*                   - Added DYNREALLOC                                       *
*        4-Mar-00 : - declared ALLMASK(n)                                    *
*       27-May-00 : - declared CONDYNFREE                                    *
*       28-May-00 : - declared nautil_freedyn()                              *
*       16-Aug-00 : - added OLDNAUTY and changed canonical labelling         *
*       16-Nov-00 : - function prototypes are now default and unavoidable    *
*                   - removed UPROC, now assume all compilers know void      *
*                   - removed nvector, now just int (as it always was)       *
*                   - added extra parameter to targetcell()                  *
*                   - removed old versions which were only to skip around    *
*                     bugs that should have been long fixed:                 *
*                     SYS_APOLLO and SYS_VAXBSD.                             *
*                   - DEFAULTOPIONS now specifies no output                  *
*                   - Removed obsolete SYS_MACLSC version                    *
*       21-Apr-01 : - Added code to satisfy compilation into Magma.  This    *
*                       is activated by defining NAUTY_IN_MAGMA above.       *
*                   - The *_null routines no longer exist                    *
*                   - Default maxinvarlevel is now 1.  (This has no effect   *
*                        unless an invariant is specified.)                  *
*                   - Now labelorg has a concrete declaration in nautil.c    *
*                        and EXTDEFS is not needed                           *
*        5-May-01 : - NILFUNCTION, NILSET, NILGRAPH now obsolete.  Use NULL. *
*       11-Sep-01 : - setword is unsigned int in the event that UINT_MAX     *
*                     is defined and indicates it is big enough              *
*       17-Oct-01 : - major rewrite for 2.1.  SYS_* variables gone!          *
*                     Some modernity assumed, eg size_t                      *
*        8-Aug-02 : - removed MAKEEMPTY  (use EMPTYSET instead)              *
*                   - deleted OLDNAUTY everywhere                            *
*       27-Aug-02 : - converted to use autoconf.  Now the original of this   *
*                     file is nauty-h.in. Run configure to make nauty.h.     *
*       20-Dec-02 : - increased INFINITY                                     *
*                     some reorganization to please Magma                    *
*                   - declared nauty_freedyn()                               *
*       17-Nov-03 : - renamed INFINITY to NAUTY_INFINITY                     *
*       29-May-04 : - added definition of SETWORD_FORMAT                     *
*       14-Sep-04 : - extended prototypes even to recursive functions        *
*       16-Oct-04 : - added DEFAULTOPTIONS_GRAPH                             *
*       24-Oct-04 : Starting 2.3                                             *
*                   - remove register declarations as modern compilers       *
*                     tend to find them a nuisance                           *
*                   - Don't define the obsolete symbol INFINITY if it is     *
*                     defined already                                        *
*       17-Nov-04 : - make 6 counters in statsblk unsigned long              *
*       17-Jan-04 : - add init() and cleanup() to dispatchvec                *
*       12-Nov-05 : - Changed NAUTY_INFINITY to 2^30+2 in BIGNAUTY case      *
*       22-Nov-06 : Starting 2.4                                             *
*                   - removed usertcellproc from options                     *
*                     changed bestcell to targetcell in dispatch vector      *
*                     declare targetcell and maketargetcell                  *
*       29-Nov-06 : - add extraoptions to optionblk                          *
*                   - add declarations of extra_autom and extra_level        *
*       10-Dec-06 : - BIGNAUTY is gone!  Now permutation=shortish=int.       *
*                     NAUTY_INFINITY only depends on whether sizeof(int)=2.  *
*       27-Jun-08 : - define nauty_counter and LONG_LONG_COUNTERS            *
*       30-Jun-08 : - declare version 2.4                                    *
*        8-Nov-09 : - final release of version 2.4;                          *
*       10-Nov-10 : Starting 2.5                                             *
*                   - declare shortish and permutation obsolete, now int     *
*       14-Nov-10 : - SETWORDSNEEDED(n)                                      *
*       23-May-10 : - declare densenauty()                                   *
*       29-Jun-10 : - add PRINT_COUNTER(f,x)                                 *
*                   - add DEFAULTOPTIONS_DIGRAPH()                           *
*       27-Mar-11 : - declare writegroupsize()                               *
*       21-Feb-12 : - add ENABLE_ANSI                                        *
*       18-Mar-12 : - add COUNTER_FMT                                        *
*       18-Aug-12 : - add ADDONEARC, ADDONEEDGE, EMPTYGRAPH                  *
*       29-Aug-12 : - add CLZ macros and FIRSTBITNZ                          *
*       19-Oct-12 : - add DEFAULT_WORDSIZE                                   *
*        3-Jan-12 : Released 2.5rc1                                          *
*                                                                            *
*       CZ notes: removed NAUTY_IN_MAGMA for simplicity
*                 remove DEFAULT_WORDSIZE
* ++++++ This file is automatically generated, don't edit it by hand! ++++++
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*   16-bit, 32-bit and 64-bit versions can be selected by defining WORDSIZE. *
*   The largest graph that can be handled has MAXN vertices.                 *
*   Both WORDSIZE and MAXN can be defined on the command line.               *
*   WORDSIZE must be 16, 32 or 64; MAXN must be <= NAUTY_INFINITY-2;         *
*                                                                            *
*   With a very slight loss of efficiency (depending on platform), nauty     *
*   can be compiled to dynamically allocate arrays.  Predefine MAXN=0 to     *
*   achieve this effect, which is default behaviour from version 2.0.        *
*   In that case, graphs of size up to NAUTY_INFINITY-2 can be handled       *
*   if the the memory is available.                                          *
*                                                                            *
*   If only very small graphs need to be processed, use MAXN<=WORDSIZE       *
*   since this causes substantial code optimizations.                        *
*                                                                            *
*   Conventions and Assumptions:                                             *
*                                                                            *
*    A 'setword' is the chunk of memory that is occupied by one part of      *
*    a set.  This is assumed to be >= WORDSIZE bits in size.                 *
*                                                                            *
*    The rightmost (loworder) WORDSIZE bits of setwords are numbered         *
*    0..WORDSIZE-1, left to right.  It is necessary that the 2^WORDSIZE      *
*    setwords with the other bits zero are totally ordered under <,=,>.      *
*    This needs care on a 1's-complement machine.                            *
*                                                                            *
*    The int variables m and n have consistent meanings throughout.          *
*    Graphs have n vertices always, and sets have m setwords always.         *
*                                                                            *
*    A 'set' consists of m contiguous setwords, whose bits are numbered      *
*    0,1,2,... from left (high-order) to right (low-order), using only       *
*    the rightmost WORDSIZE bits of each setword.  It is used to             *
*    represent a subset of {0,1,...,n-1} in the usual way - bit number x     *
*    is 1 iff x is in the subset.  Bits numbered n or greater, and           *
*    unnumbered bits, are assumed permanently zero.                          *
*                                                                            *
*    A 'graph' consists of n contiguous sets.  The i-th set represents       *
*    the vertices adjacent to vertex i, for i = 0,1,...,n-1.                 *
*                                                                            *
*    A 'permutation' is an array of n ints repesenting a permutation of      *
*    the set {0,1,...,n-1}.  The value of the i-th entry is the number to    *
*    which i is mapped.                                                      *
*                                                                            *
*    If g is a graph and p is a permutation, then g^p is the graph in        *
*    which vertex i is adjacent to vertex j iff vertex p[i] is adjacent      *
*    to vertex p[j] in g.                                                    *
*                                                                            *
*    A partition nest is represented by a pair (lab,ptn), where lab and ptn  *
*    are int arrays.  The "partition at level x" is the partition whose      *
*    cells are {lab[i],lab[i+1],...,lab[j]}, where [i,j] is a maximal        *
*    subinterval of [0,n-1] such that ptn[k] > x for i <= k < j and          *
*    ptn[j] <= x.  The partition at level 0 is given to nauty by the user.   *
*    This is  refined for the root of the tree, which has level 1.           *
*                                                                            *
*****************************************************************************/

/* WORDSIZE is the number of set elements per setword (16, 32 or 64).
   WORDSIZE and setword are defined as follows:

   If WORDSIZE is so far undefined, use 32 unless longs have more
      than 32 bits, in which case use 64.
   Define setword thus:
      WORDSIZE==16 : unsigned short
      WORDSIZE==32 : unsigned int unless it is too small,
                        in which case unsigned long
      WORDSIZE==64 : the first of unsigned int, unsigned long,
                      unsigned long long, which is large enough.
 */

#ifdef WORDSIZE

#if  (WORDSIZE != 16) && (WORDSIZE != 32) && (WORDSIZE != 64)
 #error "WORDSIZE must be 16, 32 or 64"
#endif

#else  /* WORDSIZE undefined */

#if SIZEOF_LONG > 4
#define WORDSIZE 64
#else
#define WORDSIZE 32
#endif

#endif  /* WORDSIZE */

#if WORDSIZE == 16
typedef unsigned short setword;
#define SETWORD_SHORT
#endif

#if WORDSIZE == 32
#if SIZEOF_INT >= 4
typedef unsigned int setword;
#define SETWORD_INT
#else
typedef unsigned long setword;
#define SETWORD_LONG
#endif
#endif

#if WORDSIZE == 64
#if SIZEOF_INT >= 8
typedef unsigned int setword;
#define SETWORD_INT
#else
#if SIZEOF_LONG >= 8
typedef unsigned long setword;
#define SETWORD_LONG
#else
typedef unsigned LONGLONG setword;
#define SETWORD_LONGLONG
#endif
#endif
#endif


#if SIZEOF_LONG_LONG >= 8 && SIZEOF_LONG == 4
typedef unsigned LONGLONG nauty_counter;
#define LONG_LONG_COUNTERS 1
#define COUNTER_FMT "%llu"
#else
typedef unsigned long nauty_counter;
#define LONG_LONG_COUNTERS 0
#define COUNTER_FMT "%lu"
#endif
#define PRINT_COUNTER(f, x) fprintf(f, COUNTER_FMT, x)

#define NAUTYVERSIONID (25480)  /* 10000*version */
#define NAUTYREQUIRED NAUTYVERSIONID  /* Minimum compatible version */

#if WORDSIZE == 16
#define NAUTYVERSION "2.5 (16 bits)"
#endif
#if WORDSIZE == 32
#define NAUTYVERSION "2.5 (32 bits)"
#endif
#if WORDSIZE == 64
#define NAUTYVERSION "2.5 (64 bits)"
#endif

#ifndef  MAXN  /* maximum allowed n value; use 0 for dynamic sizing. */
#define MAXN 0
#define MAXM 0
#else
#define MAXM ((MAXN + WORDSIZE - 1) / WORDSIZE)  /* max setwords in a set */
#endif  /* MAXN */

/* Starting at version 2.2, set operations work for all set sizes unless
   ONE_WORD_SETS is defined.  In the latter case, if MAXM=1, set ops
   work only for single-setword sets.  In any case, macro versions
   ending with 1 work for single-setword sets and versions ending with
   0 work for all set sizes.
 */

#if  WORDSIZE == 16
#define SETWD(pos) ((pos) >> 4)  /* number of setword containing bit pos */
#define SETBT(pos) ((pos) & 0xF) /* position within setword of bit pos */
#define TIMESWORDSIZE(w) ((w) << 4)
#define SETWORDSNEEDED(n) ((((n) - 1) >> 4) + 1)  /* setwords needed for n bits */
#endif

#if  WORDSIZE == 32
#define SETWD(pos) ((pos) >> 5)
#define SETBT(pos) ((pos) & 0x1F)
#define TIMESWORDSIZE(w) ((w) << 5)
#define SETWORDSNEEDED(n) ((((n) - 1) >> 5) + 1)
#endif

#if  WORDSIZE == 64
#define SETWD(pos) ((pos) >> 6)
#define SETBT(pos) ((pos) & 0x3F)
#define TIMESWORDSIZE(w) ((w) << 6)    /* w*WORDSIZE */
#define SETWORDSNEEDED(n) ((((n) - 1) >> 6) + 1)
#endif

#define BITT bit

#define ADDELEMENT1(setadd, pos)  (*(setadd) |= BITT[pos])
#define DELELEMENT1(setadd, pos)  (*(setadd) &= ~BITT[pos])
#define FLIPELEMENT1(setadd, pos) (*(setadd) ^= BITT[pos])
#define ISELEMENT1(setadd, pos)   ((*(setadd) & BITT[pos]) != 0)
#define EMPTYSET1(setadd, m)   *(setadd) = 0;
#define GRAPHROW1(g, v, m) ((set*)(g) + (v))
#define ADDONEARC1(g, v, w, m) (g)[v] |= BITT[w]
#define ADDONEEDGE1(g, v, w, m) { ADDONEARC1(g, v, w, m); ADDONEARC1(g, w, v, m); }
#define EMPTYGRAPH1(g, m, n) EMPTYSET0(g, n)  /* really EMPTYSET0 */

#define ADDELEMENT0(setadd, pos)  ((setadd)[SETWD(pos)] |= BITT[SETBT(pos)])
#define DELELEMENT0(setadd, pos)  ((setadd)[SETWD(pos)] &= ~BITT[SETBT(pos)])
#define FLIPELEMENT0(setadd, pos) ((setadd)[SETWD(pos)] ^= BITT[SETBT(pos)])
#define ISELEMENT0(setadd, pos) (((setadd)[SETWD(pos)] & BITT[SETBT(pos)]) != 0)
#define EMPTYSET0(setadd, m) \
  { setword *es; \
    for (es = (setword*)(setadd) + (m); --es >= (setword*)(setadd); ) *es = 0; }
#define GRAPHROW0(g, v, m) ((set*)(g) + (m) * (size_t)(v))
#define ADDONEARC0(g, v, w, m) ADDELEMENT0(GRAPHROW0(g, v, m), w)
#define ADDONEEDGE0(g, v, w, m) { ADDONEARC0(g, v, w, m); ADDONEARC0(g, w, v, m); }
#define EMPTYGRAPH0(g, m, n) EMPTYSET0(g, (m) * (size_t)(n))

#if  (MAXM == 1) && defined(ONE_WORD_SETS)
#define ADDELEMENT ADDELEMENT1
#define DELELEMENT DELELEMENT1
#define FLIPELEMENT FLIPELEMENT1
#define ISELEMENT ISELEMENT1
#define EMPTYSET EMPTYSET1
#define GRAPHROW GRAPHROW1
#define ADDONEARC ADDONEARC1
#define ADDONEEDGE ADDONEEDGE1
#define EMPTYGRAPH EMPTYGRAPH1
#else
#define ADDELEMENT ADDELEMENT0
#define DELELEMENT DELELEMENT0
#define FLIPELEMENT FLIPELEMENT0
#define ISELEMENT ISELEMENT0
#define EMPTYSET EMPTYSET0
#define GRAPHROW GRAPHROW0
#define ADDONEARC ADDONEARC0
#define ADDONEEDGE ADDONEEDGE0
#define EMPTYGRAPH EMPTYGRAPH0
#endif


#define NOTSUBSET(word1, word2) ((word1) & ~(word2))  /* test if the 1-bits
                                                         in setword word1 do not form a subset of those in word2  */
#define INTERSECT(word1, word2) ((word1) &= (word2))  /* AND word2 into word1 */
#define UNION(word1, word2)     ((word1) |= (word2))  /* OR word2 into word1 */
#define SETDIFF(word1, word2)   ((word1) &= ~(word2)) /* - word2 into word1 */
#define XOR(word1, word2)       ((word1) ^= (word2))  /* XOR word2 into word1 */
#define ZAPBIT(word, x) ((word) &= ~BITT[x])  /* delete bit x in setword */
#define TAKEBIT(iw, w) { (iw) = FIRSTBITNZ(w); (w) ^= BITT[iw]; }

#ifdef SETWORD_LONGLONG
#define MSK3232 0xFFFFFFFF00000000ULL
#define MSK1648 0xFFFF000000000000ULL
#define MSK0856 0xFF00000000000000ULL
#define MSK1632 0x0000FFFF00000000ULL
#define MSK0840     0xFF0000000000ULL
#define MSK1616         0xFFFF0000ULL
#define MSK0824         0xFF000000ULL
#define MSK0808             0xFF00ULL
#define MSK63C  0x7FFFFFFFFFFFFFFFULL
#define MSK31C          0x7FFFFFFFULL
#define MSK15C              0x7FFFULL
#define MSK64   0xFFFFFFFFFFFFFFFFULL
#define MSK32           0xFFFFFFFFULL
#define MSK16               0xFFFFULL
#define MSK8                  0xFFULL
#endif

#ifdef SETWORD_LONG
#define MSK3232 0xFFFFFFFF00000000UL
#define MSK1648 0xFFFF000000000000UL
#define MSK0856 0xFF00000000000000UL
#define MSK1632 0x0000FFFF00000000UL
#define MSK0840     0xFF0000000000UL
#define MSK1616         0xFFFF0000UL
#define MSK0824         0xFF000000UL
#define MSK0808             0xFF00UL
#define MSK63C  0x7FFFFFFFFFFFFFFFUL
#define MSK31C          0x7FFFFFFFUL
#define MSK15C              0x7FFFUL
#define MSK64   0xFFFFFFFFFFFFFFFFUL
#define MSK32           0xFFFFFFFFUL
#define MSK16               0xFFFFUL
#define MSK8                  0xFFUL
#endif

#if defined(SETWORD_INT) || defined(SETWORD_SHORT)
#define MSK3232 0xFFFFFFFF00000000U
#define MSK1648 0xFFFF000000000000U
#define MSK0856 0xFF00000000000000U
#define MSK1632 0x0000FFFF00000000U
#define MSK0840     0xFF0000000000U
#define MSK1616         0xFFFF0000U
#define MSK0824         0xFF000000U
#define MSK0808             0xFF00U
#define MSK63C  0x7FFFFFFFFFFFFFFFU
#define MSK31C          0x7FFFFFFFU
#define MSK15C              0x7FFFU
#define MSK64   0xFFFFFFFFFFFFFFFFU
#define MSK32           0xFFFFFFFFU
#define MSK16               0xFFFFU
#define MSK8                  0xFFU
#endif

#if defined(SETWORD_LONGLONG)
#if WORDSIZE == 16
#define SETWORD_FORMAT "%04llx"
#endif
#if WORDSIZE == 32
#define SETWORD_FORMAT "%08llx"
#endif
#if WORDSIZE == 64
#define SETWORD_FORMAT "%16llx"
#endif
#endif

#if defined(SETWORD_LONG)
#if WORDSIZE == 16
#define SETWORD_FORMAT "%04lx"
#endif
#if WORDSIZE == 32
#define SETWORD_FORMAT "%08lx"
#endif
#if WORDSIZE == 64
#define SETWORD_FORMAT "%16lx"
#endif
#endif

#if defined(SETWORD_INT)
#if WORDSIZE == 16
#define SETWORD_FORMAT "%04x"
#endif
#if WORDSIZE == 32
#define SETWORD_FORMAT "%08x"
#endif
#if WORDSIZE == 64
#define SETWORD_FORMAT "%16x"
#endif
#endif

#if defined(SETWORD_SHORT)
#if WORDSIZE == 16
#define SETWORD_FORMAT "%04hx"
#endif
#if WORDSIZE == 32
#define SETWORD_FORMAT "%08hx"
#endif
#if WORDSIZE == 64
#define SETWORD_FORMAT "%16hx"
#endif
#endif

/* POPCOUNT(x) = number of 1-bits in a setword x
   FIRSTBIT(x) = number of first 1-bit in non-zero setword (0..WORDSIZE-1)
                   or WORDSIZE if x == 0
   FIRSTBITNZ(x) = as FIRSTBIT(x) but assumes x is not zero
   BITMASK(x)  = setword whose rightmost WORDSIZE-x-1 (numbered) bits
                 are 1 and the rest 0 (0 <= x < WORDSIZE)
                 (I.e., bits 0..x are unselected and the rest selected.)
   ALLBITS     = all (numbered) bits in a setword  */

#if  WORDSIZE == 64
#define POPCOUNT(x) (bytecount[(x) >> 56 & 0xFF] + bytecount[(x) >> 48 & 0xFF] \
                     + bytecount[(x) >> 40 & 0xFF] + bytecount[(x) >> 32 & 0xFF] \
                     + bytecount[(x) >> 24 & 0xFF] + bytecount[(x) >> 16 & 0xFF] \
                     + bytecount[(x) >> 8 & 0xFF] + bytecount[(x) & 0xFF])
#define FIRSTBIT(x) ((x) & MSK3232 ? \
                     (x) & MSK1648 ? \
                     (x) & MSK0856 ? \
                     0 + leftbit[((x) >> 56) & MSK8] : \
                     8 + leftbit[(x) >> 48] \
                     : (x) & MSK0840 ? \
                     16 + leftbit[(x) >> 40] : \
                     24 + leftbit[(x) >> 32] \
                     : (x) & MSK1616 ? \
                     (x) & MSK0824 ? \
                     32 + leftbit[(x) >> 24] : \
                     40 + leftbit[(x) >> 16] \
                     : (x) & MSK0808 ? \
                     48 + leftbit[(x) >> 8] : \
                     56 + leftbit[x])
#define BITMASK(x)  (MSK63C >> (x))
#define ALLBITS  MSK64
#define SWCHUNK0(w) ((long)((w) >> 48) & 0xFFFFL)
#define SWCHUNK1(w) ((long)((w) >> 32) & 0xFFFFL)
#define SWCHUNK2(w) ((long)((w) >> 16) & 0xFFFFL)
#define SWCHUNK3(w) ((long)(w) & 0xFFFFL)
#endif

#if  WORDSIZE == 32
#define POPCOUNT(x) (bytecount[(x) >> 24 & 0xFF] + bytecount[(x) >> 16 & 0xFF] \
                     + bytecount[(x) >> 8 & 0xFF] + bytecount[(x) & 0xFF])
#define FIRSTBIT(x) ((x) & MSK1616 ? ((x) & MSK0824 ? \
                                      leftbit[((x) >> 24) & MSK8] : 8 + leftbit[(x) >> 16]) \
                     : ((x) & MSK0808 ? 16 + leftbit[(x) >> 8] : 24 + leftbit[x]))
#define BITMASK(x)  (MSK31C >> (x))
#define ALLBITS  MSK32
#define SWCHUNK0(w) ((long)((w) >> 16) & 0xFFFFL)
#define SWCHUNK1(w) ((long)(w) & 0xFFFFL)
#endif

#if  WORDSIZE == 16
#define POPCOUNT(x) (bytecount[(x) >> 8 & 0xFF] + bytecount[(x) & 0xFF])
#define FIRSTBIT(x) ((x) & MSK0808 ? leftbit[((x) >> 8) & MSK8] : 8 + leftbit[x])
#define BITMASK(x)  (MSK15C >> (x))
#define ALLBITS  MSK16
#define SWCHUNK0(w) ((long)(w) & 0xFFFFL)
#endif

#if defined(SETWORD_LONGLONG) && HAVE_CLZLL
#undef FIRSTBIT
#undef FIRSTBITNZ
#define FIRSTBITNZ(x) __builtin_clzll(x)
#define FIRSTBIT(x) ((x) ? FIRSTBITNZ(x) : WORDSIZE)
#endif
#if defined(SETWORD_LONG) && HAVE_CLZL
#undef FIRSTBIT
#undef FIRSTBITNZ
#define FIRSTBITNZ(x) __builtin_clzl(x)
#define FIRSTBIT(x) ((x) ? FIRSTBITNZ(x) : WORDSIZE)
#endif
#if defined(SETWORD_INT) && HAVE_CLZ
#undef FIRSTBIT
#undef FIRSTBITNZ
#define FIRSTBITNZ(x) __builtin_clz(x)
#define FIRSTBIT(x) ((x) ? FIRSTBITNZ(x) : WORDSIZE)
#endif

#ifndef FIRSTBITNZ
#define FIRSTBITNZ FIRSTBIT
#endif

#ifdef  SYS_CRAY
#undef POPCOUNT
#undef FIRSTBIT
#undef BITMASK
#define POPCOUNT(x) _popcnt(x)
#define FIRSTBIT(x) _leadz(x)
#define BITMASK(x)  _mask(65 + (x))
#endif

#define ALLMASK(n) ((n) ? ~BITMASK((n) - 1) : (setword)0)  /* First n bits */

/* various constants: */
#undef FALSE
#undef TRUE
#define FALSE    0
#define TRUE     1

#if SIZEOF_INT >= 4
#define NAUTY_INFINITY 0x40000002
#else
#define NAUTY_INFINITY 0x7FFF
#endif

/* The following four types are obsolete, use int in new code. */
typedef int shortish;
typedef shortish permutation;
typedef int nvector, np2vector;

#if MAXN > NAUTY_INFINITY - 2
 #error MAXN must be at most NAUTY_INFINITY-2
#endif

/* typedefs for sets, graphs, permutations, etc.: */

typedef int boolean;    /* boolean MUST be the same as int */

#define UPROC void      /* obsolete */

typedef setword set, graph;

typedef struct {
  double grpsize1;          /* size of group is */
  int grpsize2;             /*    grpsize1 * 10^grpsize2 */
#define groupsize1 grpsize1     /* for backwards compatibility */
#define groupsize2 grpsize2
  int numorbits;            /* number of orbits in group */
  int numgenerators;        /* number of generators found */
  int errstatus;            /* if non-zero : an error code */
#define outofspace errstatus;   /* for backwards compatibility */
  unsigned long numnodes;        /* total number of nodes */
  unsigned long numbadleaves;    /* number of leaves of no use */
  int maxlevel;                  /* maximum depth of search */
  unsigned long tctotal;         /* total size of all target cells */
  unsigned long canupdates;      /* number of updates of best label */
  unsigned long invapplics;      /* number of applications of invarproc */
  unsigned long invsuccesses;    /* number of successful uses of invarproc() */
  int invarsuclevel;        /* least level where invarproc worked */
} statsblk;

/* codes for errstatus field (see nauty.c for more accurate descriptions): */
#define NTOOBIG      1      /* n > MAXN or n > WORDSIZE*m */
#define MTOOBIG      2      /* m > MAXM */
#define CANONGNIL    3      /* canong = NULL, but getcanon = TRUE */

/* manipulation of real approximation to group size */
#define MULTIPLY(s1, s2, i) if ((s1 *= i) >= 1e10) { s1 /= 1e10; s2 += 10; }

struct optionstruct;  /* incomplete definition */

typedef struct {
  boolean (*isautom)          /* test for automorphism */
    (graph*, int*, boolean, int, int);
  int (*testcanlab)           /* test for better labelling */
  (graph *, graph *, int*, int*, int, int);
  void (*updatecan)           /* update canonical object */
  (graph *, graph *, int*, int, int, int);
  void (*refine)              /* refine partition */
  (graph *, int*, int*, int, int*, int*, set *, int*, int, int);
  void (*refine1)             /* refine partition, MAXM==1 */
  (graph *, int*, int*, int, int*, int*, set *, int*, int, int);
  boolean (*cheapautom)       /* test for easy automorphism */
    (int*, int, boolean, int);
  int (*targetcell)           /* decide which cell to split */
  (graph *, int*, int*, int, int, int, int);
  void (*freedyn)(void);      /* free dynamic memory */
  void (*check)               /* check compilation parameters */
  (int, int, int, int);
  void (*init)(graph*, graph**, graph*, graph**, int*, int*, set*,
               struct optionstruct*, int*, int, int);
  void (*cleanup)(graph*, graph**, graph*, graph**, int*, int*,
                  struct optionstruct*, statsblk*, int, int);
} dispatchvec;

typedef struct optionstruct {
  int getcanon;               /* make canong and canonlab? */
#define LABELONLY 2   /* new value UNIMPLEMENTED */
  boolean digraph;            /* multiple edges or loops? */
  boolean writeautoms;        /* write automorphisms? */
  boolean writemarkers;       /* write stats on pts fixed, etc.? */
  boolean defaultptn;         /* set lab,ptn,active for single cell? */
  boolean cartesian;          /* use cartesian rep for writing automs? */
  int linelength;             /* max chars/line (excl. '\n') for output */
  FILE *outfile;              /* file for output, if any */
  void (*userrefproc)         /* replacement for usual refine procedure */
  (graph *, int*, int*, int, int*, int*, set *, int*, int, int);
  void (*userautomproc)       /* procedure called for each automorphism */
  (int, int*, int*, int, int, int);
  void (*userlevelproc)       /* procedure called for each level */
  (int*, int*, int, int*, statsblk *, int, int, int, int, int, int);
  void (*usernodeproc)        /* procedure called for each node */
  (graph *, int*, int*, int, int, int, int, int, int);
  void (*invarproc)           /* procedure to compute vertex-invariant */
  (graph *, int*, int*, int, int, int, int*, int, boolean, int, int);
  int tc_level;               /* max level for smart target cell choosing */
  int mininvarlevel;          /* min level for invariant computation */
  int maxinvarlevel;          /* max level for invariant computation */
  int invararg;               /* value passed to (*invarproc)() */
  dispatchvec *dispatch;      /* vector of object-specific routines */
  boolean schreier;           /* use random schreier method */
  void *extra_options;        /* arbitrary extra options */
} optionblk;

#ifndef CONSOLWIDTH
#define CONSOLWIDTH 78
#endif



#define DEFAULTOPTIONS_GRAPH(options) optionblk options = \
{ 0, FALSE, FALSE, FALSE, TRUE, FALSE, CONSOLWIDTH, \
  NULL, NULL, NULL, NULL, NULL, NULL, 100, 0, 1, 0, &dispatch_graph, FALSE, NULL }



#define DEFAULTOPTIONS_DIGRAPH(options) optionblk options = \
{ 0, TRUE, FALSE, FALSE, TRUE, FALSE, CONSOLWIDTH, \
  NULL, NULL, NULL, NULL, NULL, adjacencies, 100, 0, 999, 0, &dispatch_graph, FALSE, NULL }



#ifndef DEFAULTOPTIONS
#define DEFAULTOPTIONS DEFAULTOPTIONS_GRAPH
#endif

#define PUTC(c, f) putc(c, f)

/* ALLOCS(x,y) should return a pointer (any pointer type) to x*y units of new
   storage, not necessarily initialised.  A "unit" of storage is defined by
   the sizeof operator.   x and y are integer values of type int or larger,
   but x*y may well be too large for an int.  The macro should cast to the
   correct type for the call.  On failure, ALLOCS(x,y) should return a NULL
   pointer.  FREES(p) should free storage previously allocated by ALLOCS,
   where p is the value that ALLOCS returned. */

#define ALLOCS(x, y) malloc((size_t)(x) * (size_t)(y))
#define REALLOCS(p, x) realloc(p, (size_t)(x))
#define FREES(p) free(p)

/* The following macros are used by nauty if MAXN=0.  They dynamically
   allocate arrays of size dependent on m or n.  For each array there
   should be two static variables:
     type *name;
     size_t name_sz;
   "name" will hold a pointer to an allocated array.  "name_sz" will hold
   the size of the allocated array in units of sizeof(type).  DYNALLSTAT
   declares both variables and initialises name_sz=0.  DYNALLOC1 and
   DYNALLOC2 test if there is enough space allocated, and if not free
   the existing space and allocate a bigger space.  The allocated space
   is not initialised.

   In the case of DYNALLOC1, the space is allocated using
       ALLOCS(sz,sizeof(type)).
   In the case of DYNALLOC2, the space is allocated using
       ALLOCS(sz1,sz2*sizeof(type)).

   DYNREALLOC is like DYNALLOC1 except that the old contents are copied
   into the new space.  realloc() is assumed.  This is not currently
   used by nauty or dreadnaut.

   DYNFREE frees any allocated array and sets name_sz back to 0.
   CONDYNFREE does the same, but only if name_sz exceeds some limit.
 */

#define DYNALLSTAT(type, name, name_sz) \
  static type * name; static size_t name_sz = 0
#define DYNALLOC1(type, name, name_sz, sz, msg) \
  if ((size_t)(sz) > name_sz) \
  { if (name_sz) FREES(name); name_sz = (sz); \
    if ((name = (type*)ALLOCS(sz, sizeof(type))) == NULL) { alloc_error(msg); } }
#define DYNALLOC2(type, name, name_sz, sz1, sz2, msg) \
  if ((size_t)(sz1) * (size_t)(sz2) > name_sz) \
  { if (name_sz) FREES(name); name_sz = (size_t)(sz1) * (size_t)(sz2); \
    if ((name = (type*)ALLOCS((sz1), (sz2) * sizeof(type))) == NULL) \
    { alloc_error(msg); } }
#define DYNREALLOC(type, name, name_sz, sz, msg) \
  { if ((size_t)(sz) > name_sz) \
    { if ((name = (type*)REALLOCS(name, (sz) * sizeof(type))) == NULL) \
      { alloc_error(msg); } else name_sz = (sz); } }
#define DYNFREE(name, name_sz) if (name_sz) { FREES(name); name_sz = 0; }
#define CONDYNFREE(name, name_sz, minsz) \
  if (name_sz > (size_t)(minsz)) { FREES(name); name_sz = 0; }

/* File to write error messages to (used as first argument to fprintf()). */
#define ERRFILE stderr

/* Don't use OLDEXTDEFS, it is only still here for Magma. */
#ifdef OLDEXTDEFS
#define EXTDEF_CLASS
#ifdef EXTDEFS
#define EXTDEF_TYPE 1
#else
#define EXTDEF_TYPE 2
#endif
#else
#define EXTDEF_CLASS static
#define EXTDEF_TYPE 2
#endif

extern int labelorg;   /* Declared in nautil.c */

/* array giving setwords with single 1-bit */
#if  WORDSIZE == 64
#ifdef SETWORD_LONGLONG
EXTDEF_CLASS const
setword bit[] = { 01000000000000000000000LL, 0400000000000000000000LL,
                  0200000000000000000000LL, 0100000000000000000000LL,
                  040000000000000000000LL, 020000000000000000000LL,
                  010000000000000000000LL, 04000000000000000000LL,
                  02000000000000000000LL, 01000000000000000000LL,
                  0400000000000000000LL, 0200000000000000000LL,
                  0100000000000000000LL, 040000000000000000LL,
                  020000000000000000LL, 010000000000000000LL,
                  04000000000000000LL, 02000000000000000LL,
                  01000000000000000LL, 0400000000000000LL, 0200000000000000LL,
                  0100000000000000LL, 040000000000000LL, 020000000000000LL,
                  010000000000000LL, 04000000000000LL, 02000000000000LL,
                  01000000000000LL, 0400000000000LL, 0200000000000LL,
                  0100000000000LL, 040000000000LL, 020000000000LL, 010000000000LL,
                  04000000000LL, 02000000000LL, 01000000000LL, 0400000000LL,
                  0200000000LL, 0100000000LL, 040000000LL, 020000000LL,
                  010000000LL, 04000000LL, 02000000LL, 01000000LL, 0400000LL,
                  0200000LL, 0100000LL, 040000LL, 020000LL, 010000LL, 04000LL,
                  02000LL, 01000LL, 0400LL, 0200LL, 0100LL, 040LL, 020LL, 010LL,
                  04LL, 02LL, 01LL };
#else
EXTDEF_CLASS const
setword bit[] = { 01000000000000000000000, 0400000000000000000000,
                  0200000000000000000000, 0100000000000000000000,
                  040000000000000000000, 020000000000000000000,
                  010000000000000000000, 04000000000000000000,
                  02000000000000000000, 01000000000000000000,
                  0400000000000000000, 0200000000000000000,
                  0100000000000000000, 040000000000000000, 020000000000000000,
                  010000000000000000, 04000000000000000, 02000000000000000,
                  01000000000000000, 0400000000000000, 0200000000000000,
                  0100000000000000, 040000000000000, 020000000000000,
                  010000000000000, 04000000000000, 02000000000000,
                  01000000000000, 0400000000000, 0200000000000, 0100000000000,
                  040000000000, 020000000000, 010000000000, 04000000000,
                  02000000000, 01000000000, 0400000000, 0200000000, 0100000000,
                  040000000, 020000000, 010000000, 04000000, 02000000, 01000000,
                  0400000, 0200000, 0100000, 040000, 020000, 010000, 04000,
                  02000, 01000, 0400, 0200, 0100, 040, 020, 010, 04, 02, 01 };
#endif
#endif

#if  WORDSIZE == 32
EXTDEF_CLASS const
setword bit[] = { 020000000000, 010000000000, 04000000000, 02000000000,
                  01000000000, 0400000000, 0200000000, 0100000000, 040000000,
                  020000000, 010000000, 04000000, 02000000, 01000000, 0400000,
                  0200000, 0100000, 040000, 020000, 010000, 04000, 02000, 01000,
                  0400, 0200, 0100, 040, 020, 010, 04, 02, 01 };
#endif

#if WORDSIZE == 16
EXTDEF_CLASS const
setword bit[] = { 0100000, 040000, 020000, 010000, 04000, 02000, 01000, 0400, 0200,
                  0100, 040, 020, 010, 04, 02, 01 };
#endif

/*  array giving number of 1-bits in bytes valued 0..255: */
EXTDEF_CLASS const
int bytecount[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8 };

/* array giving position (1..7) of high-order 1-bit in byte: */
EXTDEF_CLASS const
int leftbit[] = { 8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
                  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


/* The following is for C++ programs that read nauty.h.  Compile nauty
   itself using C, not C++.  */


extern void alloc_error(char *s);
extern void breakout(int*, int*, int, int, int, set*, int);
extern boolean cheapautom(int*, int, boolean, int);
extern void doref(graph *, int*, int*, int, int*, int*, int*, set *, int*,
                  void (*)(graph*, int*, int*, int, int*, int*, set*, int*, int, int),
                  void (*)(graph*, int*, int*, int, int, int, int*, int, boolean, int, int),
                  int, int, int, boolean, int, int);
extern void extra_autom(int*, int);
extern void extra_level(int, int*, int*, int, int, int, int, int, int);
extern boolean isautom(graph *, int*, boolean, int, int);
extern dispatchvec dispatch_graph;
extern int itos(int, char*);
extern void fmperm(int*, set*, set*, int, int);
extern void fmptn(int*, int*, int, set*, set*, int, int);
extern void longprune(set*, set*, set*, set*, int);
extern int nextelement(set*, int, int);
extern int orbjoin(int*, int*, int);
extern void permset(set*, set*, int, int*);
extern void putstring(FILE*, char*);
extern void refine(graph*, int*, int*, int, int*, int*, set*, int*, int, int);
extern void refine1(graph*, int*, int*, int, int*, int*, set*, int*, int, int);
extern void shortprune(set*, set*, int);
extern int testcanlab(graph*, graph*, int*, int*, int, int);
extern void updatecan(graph*, graph*, int*, int, int, int);
extern void writeperm(FILE *, int*, boolean, int, int);
extern void nauty_freedyn(void);
extern void nauty_check(int, int, int, int);
extern void nautil_freedyn(void);
extern void naugraph_freedyn(void);

#endif /* NAUTY_H___ */




#ifndef NAUTIL_H__
#define NAUTIL_H__


/*****************************************************************************
*                                                                            *
*  Auxiliary source file for version 2.5 of nauty.                           *
*                                                                            *
*   Copyright (1984-2013) Brendan McKay.  All rights reserved.               *
*   Subject to waivers and disclaimers in nauty.h.                           *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       10-Nov-87 : final changes for version 1.2                            *
*        5-Dec-87 : renamed to version 1.3 (no changes to this file)         *
*       28-Sep-88 : renamed to version 1.4 (no changes to this file)         *
*       23-Mar-89 : changes for version 1.5 :                                *
*                   - added procedure refine1()                              *
*                   - changed type of ptn from int* to nvector* in fmptn()   *
*                   - declared level in breakout()                           *
*                   - changed char[] to char* in a few places                *
*                   - minor rearrangement in bestcell()                      *
*       31-Mar-89 : - added procedure doref()                                *
*        5-Apr-89 : - changed MAKEEMPTY uses to EMPTYSET                     *
*       12-Apr-89 : - changed writeperm() and fmperm() to not use MARKing    *
*        5-May-89 : - redefined MASH to gain about 8% efficiency             *
*       18-Oct-90 : changes for version 1.6 :                                *
*                   - improved line breaking in writeperm()                  *
*       10-Nov-90 : - added dummy routine nautil_null()                      *
*       27-Aug-92 : changes for version 1.7 :                                *
*                   - made linelength <= 0 mean no line breaks               *
*        5-Jun-93 : renamed to version 1.7+ (no changes to this file)        *
*       18-Aug-93 : renamed to version 1.8 (no changes to this file)         *
*       17-Sep-93 : renamed to version 1.9 (no changes to this file)         *
*       29-Jun-95 : changes for version 1.10 :                               *
*                   - replaced loop in nextelement() to save reference past  *
*                     end of array (thanks to Kevin Maylsiak)                *
*       11-Jul-96 : changes for version 2.0 :                                *
*                   - added alloc_error()                                    *
*                   - added dynamic allocation                               *
*       21-Oct-98 : use 077777 in place of INFINITY for CLEANUP()            *
*        9-Jan-00 : added nautil_check()                                     *
*       12-Feb-00 : did a little formating of the code                       *
*       28-May-00 : added nautil_freedyn()                                   *
*       16-Aug-00 : added OLDNAUTY behaviour                                 *
*       16-Nov-00 : moved graph-specific things to naugraph.c                *
*                   use function prototypes, remove UPROC, nvector           *
*       22-Apr-01 : added code for compilation into Magma                    *
*                   removed nautil_null()                                    *
*                   removed EXTDEFS and included labelorg                    *
*       21-Nov-01 : use NAUTYREQUIRED in nautil_check()                      *
*       26-Jun-02 : revised permset() to avoid fetch past the end of         *
*                     the array (thanks to Jan Kieffer)                      *
*       17-Nov-03 : changed INFINITY to NAUTY_INFINITY                       *
*       14-Sep-04 : extended prototypes to recursive functions               *
*       23-Nov-06 : replace targetcell() by maketargetcell()                 *
*       10-Dec-06 : remove BIGNAUTY                                          *
*       10-Dec-10 : remove shortish and permutation types                    *
*       11-May-10 : use sorttemplates.c                                      *
*       27-Mar-11 : add writegroupsize()                                     *
*       16-Sep-12 : small change to objoin(), more efficient for sparse case *
*       22-Sep-12 : change documentation of orbjoin()                        *
*                                                                            *
*****************************************************************************/

/* macros for hash-codes: */
/* Don't use NAUTY_INFINITY here as that would make the canonical
 * labelling depend on whether BIGNAUTY is in operation */
#define MASH(l, i) ((((l) ^ 065435) + (i)) & 077777)
/* : expression whose long value depends only on long l and int/long i.
     Anything goes, preferably non-commutative. */

#define CLEANUP(l) ((int)((l) % 077777))
/* : expression whose value depends on long l and is less than 077777
     when converted to int then short.  Anything goes. */

#if  MAXM == 1
#define M 1
#else
#define M m
#endif

#if !MAXN
DYNALLSTAT(int, workperm0, workperm0_sz);
#else
static int workperm0[MAXN];
#endif

int labelorg = 0;   /* no TLS_ATTR on purpose */

/* aproto: header new_nauty_protos.h */

/*****************************************************************************
*                                                                            *
*  nextelement(set1,m,pos) = the position of the first element in set set1   *
*  which occupies a position greater than pos.  If no such element exists,   *
*  the value is -1.  pos can have any value less than n, including negative  *
*  values.                                                                   *
*                                                                            *
*  GLOBALS ACCESSED: none                                                    *
*                                                                            *
*****************************************************************************/

int nextelement(set *set1, int m, int pos)
{
  setword setwd;

#if  MAXM == 1
  if (pos < 0) setwd = set1[0];
  else setwd = set1[0] & BITMASK(pos);

  if (setwd == 0) return -1;
  else return FIRSTBITNZ(setwd);
#else
  int w;

  if (pos < 0) {
    w = 0;
    setwd = set1[0];
  } else {
    w = SETWD(pos);
    setwd = set1[w] & BITMASK(SETBT(pos));
  }

  for (;; ) {
    if (setwd != 0) return TIMESWORDSIZE(w) + FIRSTBITNZ(setwd);
    if (++w == m) return -1;
    setwd = set1[w];
  }
#endif
}



/*****************************************************************************
*                                                                            *
*  permset(set1,set2,m,perm)  defines set2 to be the set                     *
*  {perm[i] | i in set1}.                                                    *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>,leftbit<r>                                       *
*                                                                            *
*****************************************************************************/

void permset(set *set1, set *set2, int m, int *perm)
{
  setword setw;
  int pos, b;

#if  MAXM == 1
  EMPTYSET(set2, m);
  setw = set1[0];
  while (setw != 0) {
    TAKEBIT(b, setw);
    pos = perm[b];
    ADDELEMENT(set2, pos);
  }
#else
  int w;

  EMPTYSET(set2, m);
  for (w = 0; w < m; ++w) {
    setw = set1[w];
    while (setw != 0) {
      TAKEBIT(b, setw);
      pos = perm[TIMESWORDSIZE(w) + b];
      ADDELEMENT(set2, pos);
    }
  }
#endif
}



/*****************************************************************************
*                                                                            *
*  putstring(f,s) writes the nul-terminated string s to file f.              *
*                                                                            *
*****************************************************************************/

void
putstring(FILE *f, char *s)
{
  while (*s != '\0') {
    PUTC(*s, f);
    ++s;
  }
}



/*****************************************************************************
*                                                                            *
*  itos(i,s) converts the int i to a nul-terminated decimal character        *
*  string s.  The value returned is the number of characters excluding       *
*  the nul.                                                                  *
*                                                                            *
*  GLOBALS ACCESSED: NONE                                                    *
*                                                                            *
*****************************************************************************/

int
itos(int i, char *s)
{
  int digit, j, k;
  char c;
  int ans;

  if (i < 0) {
    k = 0;
    i = -i;
    j = 1;
    s[0] = '-';
  } else {
    k = -1;
    j = 0;
  }

  do {
    digit = i % 10;
    i = i / 10;
    s[++k] = (char) (digit + '0');
  } while (i);

  s[k + 1] = '\0';
  ans = k + 1;

  for (; j < k; ++j, --k) {
    c = s[j];
    s[j] = s[k];
    s[k] = c;
  }

  return ans;
}



/*****************************************************************************
*                                                                            *
*  orbits represents a partition of {0,1,...,n-1}, by orbits[i] = the        *
*  smallest element in the same cell as i.  map[] is any array with values   *
*  in {0,1,...,n-1}.  orbjoin(orbits,map,n) joins the cells of orbits[]      *
*  together to the minimum extent such that for each i, i and map[i] are in  *
*  the same cell.  The function value returned is the new number of cells.   *
*                                                                            *
*  GLOBALS ACCESSED: NONE                                                    *
*                                                                            *
*****************************************************************************/

int
orbjoin(int *orbits, int *map, int n)
{
  int i, j1, j2;

  for (i = 0; i < n; ++i)
    if (map[i] != i) {
      j1 = orbits[i];
      while (orbits[j1] != j1) j1 = orbits[j1];
      j2 = orbits[map[i]];
      while (orbits[j2] != j2) j2 = orbits[j2];

      if (j1 < j2) orbits[j2] = j1;
      else if (j1 > j2) orbits[j1] = j2;
    }

  j1 = 0;
  for (i = 0; i < n; ++i)
    if ((orbits[i] = orbits[orbits[i]]) == i) ++j1;

  return j1;
}



/*****************************************************************************
*                                                                            *
*  writeperm(f,perm,cartesian,linelength,n) writes the permutation perm to   *
*  the file f.  The cartesian representation (i.e. perm itself) is used if   *
*  cartesian != FALSE; otherwise the cyclic representation is used.  No      *
*  more than linelength characters (not counting '\n') are written on each   *
*  line, unless linelength is ridiculously small.  linelength<=0 causes no   *
*  line breaks at all to be made.  The global int labelorg is added to each  *
*  vertex number.                                                            *
*                                                                            *
*****************************************************************************/

void writeperm(FILE *f, int *perm, boolean cartesian, int linelength, int n)
{
  int i, k, l, curlen, intlen;
  char s[30];

  /* CONDNL(x) writes end-of-line and 3 spaces if x characters
     won't fit on the current line. */
#define CONDNL(x) if (linelength > 0 && curlen + (x) > linelength) \
  { fprintf(f, "\n   "); curlen = 3; }

  curlen = 0;
  if (cartesian) {
    for (i = 0; i < n; ++i) {
      sprintf(s, "%d", perm[i] + labelorg);
      intlen = strlen(s);
      CONDNL(intlen + 1);
      PUTC(' ', f);
      fputs(s, f);
      curlen += intlen + 1;
    }
    PUTC('\n', f);
  } else {
    for (i = n; --i >= 0; ) workperm0[i] = 0;

    for (i = 0; i < n; ++i) {
      if (workperm0[i] == 0 && perm[i] != i) {
        l = i;
        sprintf(s, "%d", l + labelorg);
        intlen = strlen(s);
        if (curlen > 3) CONDNL(2 * intlen + 4);
        PUTC('(', f);
        do {
          fputs(s, f);
          curlen += intlen + 1;
          k = l;
          l = perm[l];
          workperm0[k] = 1;
          if (l != i) {
            sprintf(s, "%d", l + labelorg);
            intlen = strlen(s);
            CONDNL(intlen + 2);
            PUTC(' ', f);
          }
        } while (l != i);
        PUTC(')', f);
        ++curlen;
      }
    }

    if (curlen == 0) fputs("(1)\n", f);
    else PUTC('\n', f);
  }
}



/*****************************************************************************
*                                                                            *
*  fmperm(perm,fix,mcr,m,n) uses perm to construct fix and mcr.  fix         *
*  contains those points are fixed by perm, while mcr contains the set of    *
*  those points which are least in their orbits.                             *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>                                                  *
*                                                                            *
*****************************************************************************/

void fmperm(int *perm, set *fix, set *mcr, int m, int n)
{
  int i, k, l;

#if !MAXN
  DYNALLOC1(int, workperm0, workperm0_sz, n, "writeperm");
#endif

  EMPTYSET(fix, m);
  EMPTYSET(mcr, m);

  for (i = n; --i >= 0; ) workperm0[i] = 0;

  for (i = 0; i < n; ++i)
    if (perm[i] == i) {
      ADDELEMENT(fix, i);
      ADDELEMENT(mcr, i);
    } else if (workperm0[i] == 0) {
      l = i;
      do {
        k = l;
        l = perm[l];
        workperm0[k] = 1;
      } while (l != i);

      ADDELEMENT(mcr, i);
    }
}



/*****************************************************************************
*                                                                            *
*  fmptn(lab,ptn,level,fix,mcr,m,n) uses the partition at the specified      *
*  level in the partition nest (lab,ptn) to make sets fix and mcr.  fix      *
*  represents the points in trivial cells of the partition, while mcr        *
*  represents those points which are least in their cells.                   *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>                                                  *
*                                                                            *
*****************************************************************************/

void fmptn(int *lab, int *ptn, int level, set *fix, set *mcr, int m, int n)
{
  int i, lmin;

  EMPTYSET(fix, m);
  EMPTYSET(mcr, m);

  for (i = 0; i < n; ++i)
    if (ptn[i] <= level) {
      ADDELEMENT(fix, lab[i]);
      ADDELEMENT(mcr, lab[i]);
    } else {
      lmin = lab[i];
      do
        if (lab[++i] < lmin) lmin = lab[i];
      while (ptn[i] > level);
      ADDELEMENT(mcr, lmin);
    }
}



#ifndef SORTTEMPLATES_C__
#define SORTTEMPLATES_C__

/* sorttemplates.c version 1.0, May 11, 2010.
 * Author: Brendan McKay; bdm@cs.anu.edu.au
 *
 * This file contains templates for creating in-place sorting procedures
 * for different data types.  It cannot be compiled separately but
 * should be #included after defining a few preprocessor variables.
 *
 * Creates a procedure
 *         static void sort_parallel(SORT_TYPE1 *x, SORT_TYPE2 *y, int n)
 *     which permutes x[0..n-1] so that x[0] <= ... <= x[n-1]
 *     and also permutes y[0..n-1] by the same permutation.
 *
 *   SORT_TYPE1 = type of the first or only array (default "int")
 *       This can be any numeric type for SORT_OF_SORT=1,2, but
 *       should be an integer type for SORT_OF_SORT=3.
 *   SORT_TYPE2 = type of the second array if needed (default "int")
 *       This can be any assignable type (including a structure) for
 *       SORT_OF_SORT=2, but must be a numeric type for SORT_OF_SORT=3.
 *
 *   SORT_MINPARTITION = least number of elements for using quicksort
 *           partitioning, otherwise insertion sort is used (default "11")
 *   SORT_MINMEDIAN9 = least number of elements for using the median of 3
 *           medians of 3 for partitioning (default "320")
 *   SORT_FUNCTYPE = type of sort function (default "static void")
 *
 */

#define SORT_MEDIAN_OF_3(a, b, c) \
  ((a) <= (b) ? ((b) <= (c) ? (b) : (c) <= (a) ? (a) : (c)) \
   : ((a) <= (c) ? (a) : (c) <= (b) ? (b) : (c)))

#ifndef SORT_TYPE1
#define SORT_TYPE1 int
#endif

#ifndef SORT_TYPE2
#define SORT_TYPE2 int
#endif

#ifndef SORT_MINPARTITION
#define SORT_MINPARTITION 10
#endif

#ifndef SORT_MINMEDIAN9
#define SORT_MINMEDIAN9 320
#endif

#define SORT_SWAP1(x, y) tmp1 = x; x = y; y = tmp1;
#define SORT_SWAP2(x, y) tmp2 = x; x = y; y = tmp2;

/*******************************************************************/

static void
sortparallel(SORT_TYPE1 *x, SORT_TYPE2 *y, int n)
{
  int i, j;
  int a, d, ba, dc, s, nn;
  SORT_TYPE2 tmp2, *y0, *ya, *yb, *yc, *yd, *yl, *yh;
  SORT_TYPE1 tmp1, v, v1, v2, v3;
  SORT_TYPE1 *x0, *xa, *xb, *xc, *xd, *xh, *xl;
  struct { SORT_TYPE1 *addr; int len; } stack[40];
  int top;

  top = 0;
  if (n > 1) {
    stack[top].addr = x;
    stack[top].len = n;
    ++top;
  }

  while (top > 0) {
    --top;
    x0 = stack[top].addr;
    y0 = y + (x0 - x);
    nn = stack[top].len;

    if (nn < SORT_MINPARTITION) {
      for (i = 1; i < nn; ++i) {
        tmp1 = x0[i];
        tmp2 = y0[i];
        for (j = i; x0[j - 1] > tmp1; ) {
          x0[j] = x0[j - 1];
          y0[j] = y0[j - 1];
          if (--j == 0) break;
        }
        x0[j] = tmp1;
        y0[j] = tmp2;
      }
      continue;
    }

    if (nn < SORT_MINMEDIAN9)
      v = SORT_MEDIAN_OF_3(x0[0], x0[nn / 2], x0[nn - 1]);
    else {
      v1 = SORT_MEDIAN_OF_3(x0[0], x0[1], x0[2]);
      v2 = SORT_MEDIAN_OF_3(x0[nn / 2 - 1], x0[nn / 2], x0[nn / 2 + 1]);
      v3 = SORT_MEDIAN_OF_3(x0[nn - 3], x0[nn - 2], x0[nn - 1]);
      v = SORT_MEDIAN_OF_3(v1, v2, v3);
    }

    xa = xb = x0;  xc = xd = x0 + (nn - 1);
    ya = yb = y0;  yc = yd = y0 + (nn - 1);
    for (;; ) {
      while (xb <= xc && *xb <= v) {
        if (*xb == v) {
          *xb = *xa; *xa = v; ++xa;
          SORT_SWAP2(*ya, *yb); ++ya;
        }
        ++xb; ++yb;
      }
      while (xc >= xb && *xc >= v) {
        if (*xc == v) {
          *xc = *xd; *xd = v; --xd;
          SORT_SWAP2(*yc, *yd); --yd;
        }
        --xc; --yc;
      }
      if (xb > xc) break;
      SORT_SWAP1(*xb, *xc);
      SORT_SWAP2(*yb, *yc);
      ++xb; ++yb;
      --xc; --yc;
    }

    a = xa - x0;
    ba = xb - xa;
    if (ba > a) s = a; else s = ba;
    for (xl = x0, xh = xb - s, yl = y0, yh = yb - s; s > 0; --s) {
      *xl = *xh; *xh = v; ++xl; ++xh;
      SORT_SWAP2(*yl, *yh); ++yl; ++yh;
    }
    d = xd - x0;
    dc = xd - xc;
    if (dc > nn - 1 - d) s = nn - 1 - d; else s = dc;
    for (xl = xb, xh = x0 + (nn - s), yl = yb, yh = y0 + (nn - s); s > 0; --s) {
      *xh = *xl; *xl = v; ++xl; ++xh;
      SORT_SWAP2(*yl, *yh); ++yl; ++yh;
    }

    if (ba > dc) {
      if (ba > 1) {
        stack[top].addr = x0; stack[top].len = ba; ++top;
      }
      if (dc > 1) {
        stack[top].addr = x0 + (nn - dc); stack[top].len = dc; ++top;
      }
    } else {
      if (dc > 1) {
        stack[top].addr = x0 + (nn - dc); stack[top].len = dc; ++top;
      }
      if (ba > 1) {
        stack[top].addr = x0; stack[top].len = ba; ++top;
      }
    }
  }
}



#endif /* SORTTEMPLATES_C__ */



/*****************************************************************************
*                                                                            *
*  doref(g,lab,ptn,level,numcells,qinvar,invar,active,code,refproc,          *
*        invarproc,mininvarlev,maxinvarlev,invararg,digraph,m,n)             *
*  is used to perform a refinement on the partition at the given level in    *
*  (lab,ptn).  The number of cells is *numcells both for input and output.   *
*  The input active is the active set for input to the refinement procedure  *
*  (*refproc)(), which must have the argument list of refine().              *
*  active may be arbitrarily changed.  invar is used for working storage.    *
*  First, (*refproc)() is called.  Then, if invarproc!=NULL and              *
*  |mininvarlev| <= level <= |maxinvarlev|, the routine (*invarproc)() is    *
*  used to compute a vertex-invariant which may refine the partition         *
*  further.  If it does, (*refproc)() is called again, using an active set   *
*  containing all but the first fragment of each old cell.  Unless g is a    *
*  digraph, this guarantees that the final partition is equitable.  The      *
*  arguments invararg and digraph are passed to (*invarproc)()               *
*  uninterpretted.  The output argument code is a composite of the codes     *
*  from all the calls to (*refproc)().  The output argument qinvar is set    *
*  to 0 if (*invarproc)() is not applied, 1 if it is applied but fails to    *
*  refine the partition, and 2 if it succeeds.                               *
*  See the file nautinv.c for a further discussion of vertex-invariants.     *
*  Note that the dreadnaut I command generates a call to  this procedure     *
*  with level = mininvarlevel = maxinvarlevel = 0.                           *
*                                                                            *
*****************************************************************************/

void doref(graph *g, int *lab, int *ptn, int level, int *numcells,
           int *qinvar, int *invar, set *active, int *code,
           void (*refproc)(graph*, int*, int*, int, int*, int*, set*, int*, int, int),
           void (*invarproc)(graph*, int*, int*, int, int, int, int*,
                             int, boolean, int, int),
           int mininvarlev, int maxinvarlev, int invararg,
           boolean digraph, int m, int n)
{
  int pw;
  int i, cell1, cell2, nc, tvpos, minlev, maxlev;
  long longcode;
  boolean same;

#if !MAXN
  DYNALLOC1(int, workperm0, workperm0_sz, n, "doref");
#endif

  if ((tvpos = nextelement(active, M, -1)) < 0) tvpos = 0;

  (*refproc)(g, lab, ptn, level, numcells, invar, active, code, M, n);

  minlev = (mininvarlev < 0 ? -mininvarlev : mininvarlev);
  maxlev = (maxinvarlev < 0 ? -maxinvarlev : maxinvarlev);
  if (invarproc != NULL && *numcells < n
      && level >= minlev && level <= maxlev) {
    (*invarproc)(g, lab, ptn, level, *numcells, tvpos, invar, invararg,
                 digraph, M, n);
    EMPTYSET(active, m);
    for (i = n; --i >= 0; ) workperm0[i] = invar[lab[i]];
    nc = *numcells;
    for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
      pw = workperm0[cell1];
      same = TRUE;
      for (cell2 = cell1; ptn[cell2] > level; ++cell2)
        if (workperm0[cell2 + 1] != pw) same = FALSE;

      if (same) continue;

      sortparallel(workperm0 + cell1, lab + cell1, cell2 - cell1 + 1);

      for (i = cell1 + 1; i <= cell2; ++i)
        if (workperm0[i] != workperm0[i - 1]) {
          ptn[i - 1] = level;
          ++*numcells;
          ADDELEMENT(active, i);
        }
    }

    if (*numcells > nc) {
      *qinvar = 2;
      longcode = *code;
      (*refproc)(g, lab, ptn, level, numcells, invar, active, code, M, n);
      longcode = MASH(longcode, *code);
      *code = CLEANUP(longcode);
    } else
      *qinvar = 1;
  } else
    *qinvar = 0;
}



/*****************************************************************************
*                                                                            *
*  maketargetcell(g,lab,ptn,level,tcell,tcellsize,&cellpos,                  *
*                 tc_level,hint,targetcell,m,n)                      *
*  calls targetcell() to determine the target cell at the specified level    *
*  in the partition nest (lab,ptn).  It must be a nontrivial cell (if not,   *
*  the first cell.  The intention of hint is that, if hint >= 0 and there    *
*  is a suitable non-trivial cell starting at position hint in lab,          *
*  that cell is chosen.                                                      *
*  tc_level and digraph are input options.                                   *
*  When a cell is chosen, tcell is set to its contents, *tcellsize to its    *
*  size, and cellpos to its starting position in lab.                        *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>                                                  *
*                                                                            *
*****************************************************************************/

void maketargetcell(graph *g, int *lab, int *ptn, int level, set *tcell,
                    int *tcellsize, int *cellpos, int tc_level,
                    int hint,
                    int (*targetcell)(graph*, int*, int*, int, int, int, int),
                    int m, int n)
{
  int i, j, k;

  i = (*targetcell)(g, lab, ptn, level, tc_level, hint, n);
  for (j = i + 1; ptn[j] > level; ++j) {
  }

  *tcellsize = j - i + 1;

  EMPTYSET(tcell, m);
  for (k = i; k <= j; ++k) ADDELEMENT(tcell, lab[k]);

  *cellpos = i;
}



/*****************************************************************************
*                                                                            *
*  shortprune(set1,set2,m) ANDs the contents of set set2 into set set1.      *
*                                                                            *
*  GLOBALS ACCESSED: NONE                                                    *
*                                                                            *
*****************************************************************************/

void shortprune(set *set1, set *set2, int m)
{
  int i;

  for (i = 0; i < M; ++i) INTERSECT(set1[i], set2[i]);
}



/*****************************************************************************
*                                                                            *
*  breakout(lab,ptn,level,tc,tv,active,m) operates on the partition at       *
*  the specified level in the partition nest (lab,ptn).  It finds the        *
*  element tv, which is in the cell C starting at index tc in lab (it had    *
*  better be) and splits C in the two cells {tv} and C\{tv}, in that order.  *
*  It also sets the set active to contain just the element tc.               *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>                                                  *
*                                                                            *
*****************************************************************************/

void
breakout(int *lab, int *ptn, int level, int tc, int tv,
         set *active, int m)
{
  int i, prev, next;

  EMPTYSET(active, m);
  ADDELEMENT(active, tc);

  i = tc;
  prev = tv;

  do {
    next = lab[i];
    lab[i++] = prev;
    prev = next;
  } while (prev != tv);

  ptn[tc] = level;
}



/*****************************************************************************
*                                                                            *
*  longprune(tcell,fix,bottom,top,m) removes zero or elements of the set     *
*  tcell.  It is assumed that addresses bottom through top-1 contain         *
*  contiguous pairs of sets (f1,m1),(f2,m2), ... .  tcell is intersected     *
*  with each mi such that fi is a subset of fix.                             *
*                                                                            *
*  GLOBALS ACCESSED: NONE                                                    *
*                                                                            *
*****************************************************************************/

void longprune(set *tcell, set *fix, set *bottom, set *top, int m)
{
  int i;

  while (bottom < top) {
    for (i = 0; i < M; ++i)
      if (NOTSUBSET(fix[i], bottom[i])) break;
    bottom += M;

    if (i == M)
      for (i = 0; i < M; ++i) INTERSECT(tcell[i], bottom[i]);
    bottom += M;
  }
}



/*****************************************************************************
*                                                                            *
*  writegroupsize(f,gpsize1,gpsize2) writes a real number gpsize1*10^gpsize2 *
*  It is assumed that gpsize1 >= 1 and that gpsize1 equals an integer in the *
*  case that gpsize2==0.  These assumptions are true for group sizes         *
*  computed by nauty.                                                        *
*                                                                            *
*****************************************************************************/

void writegroupsize(FILE *f, double gpsize1, int gpsize2)
{
  if (gpsize2 == 0)
    fprintf(f, "%.0f", gpsize1 + 0.1);
  else {
    while (gpsize1 >= 10.0) {
      gpsize1 /= 10.0;
      ++gpsize2;
    }
    fprintf(f, "%14.12fe%d", gpsize1, gpsize2);
  }
}



/*****************************************************************************
*                                                                            *
*  alloc_error() writes a message and exits.  Used by DYNALLOC? macros.      *
*                                                                            *
*****************************************************************************/

void alloc_error(char *s)
{
  fprintf(ERRFILE, "Dynamic allocation failed: %s\n", s);
  exit(2);
}



/*****************************************************************************
*                                                                            *
*  nautil_freedyn() - free the dynamic memory in this module                 *
*                                                                            *
*****************************************************************************/

void nautil_freedyn(void)
{
#if !MAXN
  DYNFREE(workperm0, workperm0_sz);
#endif
}



#endif /* NAUTIL_H__ */


#ifndef NAUTY_C__
#define NAUTY_C__

/*****************************************************************************
*                                                                            *
*  Main source file for version 2.5 of nauty.                                *
*                                                                            *
*   Copyright (1984-2013) Brendan McKay.  All rights reserved.  Permission   *
*   Subject to the waivers and disclaimers in nauty.h.                       *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       10-Nov-87 : final changes for version 1.2                            *
*        5-Dec-87 : renamed to version 1.3 (no changes to this file)         *
*       28-Sep-88 : renamed to version 1.4 (no changes to this file)         *
*       23-Mar-89 : changes for version 1.5 :                                *
*                   - add use of refine1 instead of refine for m==1          *
*                   - changes for new optionblk syntax                       *
*                   - disable tc_level use for digraphs                      *
*                   - interposed doref() interface to refine() so that       *
*                        options.invarproc can be supported                  *
*                   - declared local routines static                         *
*       28-Mar-89 : - implemented mininvarlevel/maxinvarlevel < 0 options    *
*        2-Apr-89 : - added invarproc fields in stats                        *
*        5-Apr-89 : - modified error returns from nauty()                    *
*                   - added error message to ERRFILE                         *
*                   - changed MAKEEMPTY uses to EMPTYSET                     *
*       18-Apr-89 : - added MTOOBIG and CANONGNIL                            *
*        8-May-89 : - changed firstcode[] and canoncode[] to short           *
*       10-Nov-90 : changes for version 1.6 :                                *
*                   - added dummy routine nauty_null (see dreadnaut.c)       *
*        2-Sep-91 : changes for version 1.7 :                                *
*                   - moved MULTIPLY into nauty.h                            *
*       27-Mar-92 : - changed 'n' into 'm' in error message in nauty()       *
*        5-Jun-93 : renamed to version 1.7+ (no changes to this file)        *
*       18-Aug-93 : renamed to version 1.8 (no changes to this file)         *
*       17-Sep-93 : renamed to version 1.9 (no changes to this file)         *
*       13-Jul-96 : changes for version 2.0 :                                *
*                   - added dynamic allocation                               *
*       21-Oct-98 : - made short into shortish for BIGNAUTY as needed        *
*        7-Jan-00 : - allowed n=0                                            *
*                   - added nauty_check() and a call to it                   *
*       12-Feb-00 : - used better method for target cell memory allocation   *
*                   - did a little formating of the code                     *
*       27-May-00 : - fixed error introduced on Feb 12.                      *
*                   - dynamic allocations in nauty() are now deallocated     *
*                     before return if n >= 320.                             *
*       16-Nov-00 : - use function prototypes, change UPROC to void.         *
*                   - added argument to tcellproc(), removed nvector         *
*                   - now use options.dispatch, options.groupopts is gone.   *
*       22-Apr-01 : - Added code for compilation into Magma                  *
*                   - Removed nauty_null() and EXTDEFS                       *
*        2-Oct-01 : - Improved error message for bad dispatch vector         *
*       21-Nov-01 : - use NAUTYREQUIRED in nauty_check()                     *
*       20-Dec-02 : changes for version 2.2:                                 *
*                   - made tcnode0 global                                    *
*                   - added nauty_freedyn()                                  *
*       17-Nov-03 : changed INFINITY to NAUTY_INFINITY                       *
*       14-Sep-04 : extended prototypes even to recursive functions          *
*       16-Oct-04 : disallow NULL dispatch vector                            *
*       11-Nov-05 : changes for version 2.3:                                 *
*                   - init() and cleanup() optional calls                    *
*       23-Nov-06 : changes for version 2.4:                                 *
*                   - use maketargetcell() instead of tcellproc()            *
*       29-Nov-06 : add extra_autom, extra_level, extra_options              *
*       10-Dec-06 : remove BIGNAUTY                                          *
*       10-Nov-09 : remove shortish and permutation types                    *
*       16-Nov-11 : added Shreier option                                     *
*       15-Jan012 : added TLS_ATTR to static declarations                    *
*                                                                            *
*****************************************************************************/

typedef struct tcnode_struct {
  struct tcnode_struct *next;
  set *tcellptr;
} tcnode;

/* aproto: header new_nauty_protos.h */

#if !MAXN
static int firstpathnode0(int*, int*, int, int, tcnode*);
static int othernode0(int*, int*, int, int, tcnode*);
#else
static int firstpathnode(int*, int*, int, int);
static int othernode(int*, int*, int, int);
#endif
static void firstterminal(int*, int);
static int processnode(int*, int*, int, int);
static void recover(int*, int);
static void writemarker(int, int, int, int, int, int);

#define OPTCALL(proc) if (proc != NULL) (*proc)

/* copies of some of the options: */
static boolean getcanon, digraph, writeautoms, domarkers, cartesian;
#ifdef NAU_SCHREIER
static boolean doschreier;
#endif
static int linelength, tc_level, mininvarlevel, maxinvarlevel, invararg;
static void (*usernodeproc)(graph*, int*, int*, int, int, int, int, int, int);
static void (*userautomproc)(int, int*, int*, int, int, int);
static void (*userlevelproc)
(int*, int*, int, int*, statsblk *, int, int, int, int, int, int);
static void (*invarproc)
(graph *, int*, int*, int, int, int, int*, int, boolean, int, int);
static FILE *outfile;
static dispatchvec dispatch;

/* local versions of some of the arguments: */
static int m, n;
static graph *g, *canong;
static int *orbits;
static statsblk *stats;
/* temporary versions of some stats: */
static unsigned long invapplics, invsuccesses;
static int invarsuclevel;

/* working variables: <the "bsf leaf" is the leaf which is best guess so
                           far at the canonical leaf>  */
static int gca_first, /* level of greatest common ancestor of
                                  current node and first leaf */
           gca_canon, /* ditto for current node and bsf leaf */
           noncheaplevel, /* level of greatest ancestor for which cheapautom==FALSE */
           allsamelevel, /* level of least ancestor of first leaf for
                            which all descendant leaves are known to be
                            equivalent */
           eqlev_first, /* level to which codes for this node match those
                           for first leaf */
           eqlev_canon, /* level to which codes for this node match those
                           for the bsf leaf. */
           comp_canon, /* -1,0,1 according as code at eqlev_canon+1 is
                           <,==,> that for bsf leaf.  Also used for
                           similar purpose during leaf processing */
           samerows, /* number of rows of canong which are correct for
                        the bsf leaf  BDM:correct description? */
           canonlevel, /* level of bsf leaf */
           stabvertex, /* point fixed in ancestor of first leaf at level
                          gca_canon */
           cosetindex; /* the point being fixed at level gca_first */

static boolean needshortprune;  /* used to flag calls to shortprune */

#if !MAXN
DYNALLSTAT(set, defltwork, defltwork_sz);
/* cz: this variable must be renamed to avoid the conflict
 * with the variable of the same name in nautil.h */
DYNALLSTAT(int, workperm1, workperm1_sz);
DYNALLSTAT(set, fixedpts, fixedpts_sz);
DYNALLSTAT(int, firstlab, firstlab_sz);
DYNALLSTAT(int, canonlab, canonlab_sz);
DYNALLSTAT(short, firstcode, firstcode_sz);
DYNALLSTAT(short, canoncode, canoncode_sz);
DYNALLSTAT(int, firsttc, firsttc_sz);
DYNALLSTAT(set, active, active_sz);

/* In the dynamically allocated case (MAXN=0), each level of recursion
   needs one set (tcell) to represent the target cell.  This is
   implemented by using a linked list of tcnode anchored at the root
   of the search tree.  Each node points to its child (if any) and to
   the dynamically allocated tcell.  Apart from the the first node of
   the list, each node always has a tcell good for m up to alloc_m.
   tcnodes and tcells are kept between calls to nauty, except that
   they are freed and reallocated if m gets bigger than alloc_m.  */

static tcnode tcnode0 = { NULL, NULL };
static int alloc_m = 0;

#else
static set defltwork[2 * MAXM];   /* workspace in case none provided */
/* cz: this variable must be renamed to avoid the conflict
 * with the variable of the same name in nautil.h */
static int workperm1[MAXN];   /* various scratch uses */
static set fixedpts[MAXM];      /* points which were explicitly
                                    fixed to get current node */
static int firstlab[MAXN],   /* label from first leaf */
           canonlab[MAXN];       /* label from bsf leaf */
static short firstcode[MAXN + 2],      /* codes for first leaf */
             canoncode[MAXN + 2]; /* codes for bsf leaf */
static int firsttc[MAXN + 2];  /* index of target cell for left path */
static set active[MAXM];     /* used to contain index to cells now
                                    active for refinement purposes */
#endif

static set *workspace, *worktop;  /* first and just-after-last
                                     addresses of work area to hold automorphism data */
static set *fmptr;                   /* pointer into workspace */

#ifdef NAU_SCHREIER
static schreier *gp;       /* These two for Schreier computations */
static permnode *gens;
#endif

/*****************************************************************************
*                                                                            *
*  This procedure finds generators for the automorphism group of a           *
*  vertex-coloured graph and optionally finds a canonically labelled         *
*  isomorph.  A description of the data structures can be found in           *
*  nauty.h and in the "nauty User's Guide".  The Guide also gives            *
*  many more details about its use, and implementation notes.                *
*                                                                            *
*  Parameters - <r> means read-only, <w> means write-only, <wr> means both:  *
*           g <r>  - the graph                                               *
*     lab,ptn <rw> - used for the partition nest which defines the colouring *
*                  of g.  The initial colouring will be set by the program,  *
*                  using the same colour for every vertex, if                *
*                  options->defaultptn!=FALSE.  Otherwise, you must set it   *
*                  yourself (see the Guide). If options->getcanon!=FALSE,    *
*                  the contents of lab on return give the labelling of g     *
*                  corresponding to canong.  This does not change the        *
*                  initial colouring of g as defined by (lab,ptn), since     *
*                  the labelling is consistent with the colouring.           *
*     active  <r>  - If this is not NULL and options->defaultptn==FALSE,     *
*                  it is a set indicating the initial set of active colours. *
*                  See the Guide for details.                                *
*     orbits  <w>  - On return, orbits[i] contains the number of the         *
*                  least-numbered vertex in the same orbit as i, for         *
*                  i=0,1,...,n-1.                                            *
*    options  <r>  - A list of options.  See nauty.h and/or the Guide        *
*                  for details.                                              *
*      stats  <w>  - A list of statistics produced by the procedure.  See    *
*                  nauty.h and/or the Guide for details.                     *
*  workspace  <w>  - A chunk of memory for working storage.                  *
*  worksize   <r>  - The number of setwords in workspace.  See the Guide     *
*                  for guidance.                                             *
*          m  <r>  - The number of setwords in sets.  This must be at        *
*                  least ceil(n / WORDSIZE) and at most MAXM.                *
*          n  <r>  - The number of vertices.  This must be at least 1 and    *
*                  at most MAXN.                                             *
*     canong  <w>  - The canononically labelled isomorph of g.  This is      *
*                  only produced if options->getcanon!=FALSE, and can be     *
*                  given as NULL otherwise.                                  *
*                                                                            *
*  FUNCTIONS CALLED: firstpathnode(),updatecan()                             *
*                                                                            *
*****************************************************************************/

void nauty(graph * RESTRICT g_arg,
           int * RESTRICT lab, int * RESTRICT ptn,
           set * RESTRICT active_arg, int * RESTRICT orbits_arg,
           optionblk * RESTRICT options, statsblk * RESTRICT stats_arg,
           set * RESTRICT ws_arg, int worksize, int m_arg, int n_arg,
           graph * RESTRICT canong_arg)
{
  int i;
  int numcells;
  int initstatus;
#if !MAXN
  tcnode *tcp, *tcq;
#endif

  /* determine dispatch vector */

  if (options->dispatch == NULL) {
    fprintf(ERRFILE, ">E nauty: null dispatch vector\n");
    fprintf(ERRFILE, "Maybe you need to recompile\n");
    exit(1);
  } else
    dispatch = *(options->dispatch);

  if (options->userrefproc)
    dispatch.refine = options->userrefproc;
  else if (dispatch.refine1 && m_arg == 1)
    dispatch.refine = dispatch.refine1;

  if (dispatch.refine == NULL || dispatch.updatecan == NULL
      || dispatch.targetcell == NULL || dispatch.cheapautom == NULL) {
    fprintf(ERRFILE, ">E bad dispatch vector\n");
    exit(1);
  }

  /* check for excessive sizes: */

#if !MAXN
  if (m_arg > NAUTY_INFINITY / WORDSIZE + 1) {
    stats_arg->errstatus = MTOOBIG;
    fprintf(ERRFILE, "nauty: need m <= %d, but m=%d\n\n",
            NAUTY_INFINITY / WORDSIZE + 1, m_arg);
    return;
  }
  if (n_arg > NAUTY_INFINITY - 2 || n_arg > WORDSIZE * m_arg) {
    stats_arg->errstatus = NTOOBIG;
    fprintf(ERRFILE, "nauty: need n <= min(%d,%d*m), but n=%d\n\n",
            NAUTY_INFINITY - 2, WORDSIZE, n_arg);
    return;
  }
#else
  if (m_arg > MAXM) {
    stats_arg->errstatus = MTOOBIG;
    fprintf(ERRFILE, "nauty: need m <= %d\n\n", MAXM);
    return;
  }
  if (n_arg > MAXN || n_arg > WORDSIZE * m_arg) {
    stats_arg->errstatus = NTOOBIG;
    fprintf(ERRFILE,
            "nauty: need n <= min(%d,%d*m)\n\n", MAXM, WORDSIZE);
    return;
  }
#endif
  if (n_arg == 0) {   /* Special code for zero-sized graph */
    stats_arg->grpsize1 = 1.0;
    stats_arg->grpsize2 = 0;
    stats_arg->numorbits = 0;
    stats_arg->numgenerators = 0;
    stats_arg->errstatus = 0;
    stats_arg->numnodes = 1;
    stats_arg->numbadleaves = 0;
    stats_arg->maxlevel = 1;
    stats_arg->tctotal = 0;
    stats_arg->canupdates = (options->getcanon != 0);
    stats_arg->invapplics = 0;
    stats_arg->invsuccesses = 0;
    stats_arg->invarsuclevel = 0;
    return;
  }

  /* take copies of some args, and options: */
  m = m_arg;
  n = n_arg;

  nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);
  OPTCALL(dispatch.check) (WORDSIZE, m, n, NAUTYVERSIONID);

#if !MAXN
  DYNALLOC1(set, defltwork, defltwork_sz, 2 * m, "nauty");
  DYNALLOC1(set, fixedpts, fixedpts_sz, m, "nauty");
  DYNALLOC1(set, active, active_sz, m, "nauty");
  DYNALLOC1(int, workperm1, workperm1_sz, n, "nauty");
  DYNALLOC1(int, firstlab, firstlab_sz, n, "nauty");
  DYNALLOC1(int, canonlab, canonlab_sz, n, "nauty");
  DYNALLOC1(short, firstcode, firstcode_sz, n + 2, "nauty");
  DYNALLOC1(short, canoncode, canoncode_sz, n + 2, "nauty");
  DYNALLOC1(int, firsttc, firsttc_sz, n + 2, "nauty");
  if (m > alloc_m) {
    tcp = tcnode0.next;
    while (tcp != NULL) {
      tcq = tcp->next;
      FREES(tcp->tcellptr);
      FREES(tcp);
      tcp = tcq;
    }
    alloc_m = m;
    tcnode0.next = NULL;
  }
#endif

  /* OLD g = g_arg; */
  orbits = orbits_arg;
  stats = stats_arg;

  getcanon = options->getcanon;
  digraph = options->digraph;
  writeautoms = options->writeautoms;
  domarkers = options->writemarkers;
  cartesian = options->cartesian;
#ifdef NAU_SCHREIER
  doschreier = options->schreier;
  if (doschreier) schreier_check(WORDSIZE, m, n, NAUTYVERSIONID);
#endif
  linelength = options->linelength;
  if (digraph) tc_level = 0;
  else tc_level = options->tc_level;
  outfile = (options->outfile == NULL ? stdout : options->outfile);
  usernodeproc = options->usernodeproc;
  userautomproc = options->userautomproc;
  userlevelproc = options->userlevelproc;

  invarproc = options->invarproc;
  if (options->mininvarlevel < 0 && options->getcanon)
    mininvarlevel = -options->mininvarlevel;
  else
    mininvarlevel = options->mininvarlevel;
  if (options->maxinvarlevel < 0 && options->getcanon)
    maxinvarlevel = -options->maxinvarlevel;
  else
    maxinvarlevel = options->maxinvarlevel;
  invararg = options->invararg;

  if (getcanon)
    if (canong_arg == NULL) {
      stats_arg->errstatus = CANONGNIL;
      fprintf(ERRFILE,
              "nauty: canong=NULL but options.getcanon=TRUE\n\n");
      return;
    }

  /* initialize everything: */

  if (options->defaultptn) {
    for (i = 0; i < n; ++i) {     /* give all verts same colour */
      lab[i] = i;
      ptn[i] = NAUTY_INFINITY;
    }
    ptn[n - 1] = 0;
    EMPTYSET(active, m);
    ADDELEMENT(active, 0);
    numcells = 1;
  } else {
    ptn[n - 1] = 0;
    numcells = 0;
    for (i = 0; i < n; ++i)
      if (ptn[i] != 0) ptn[i] = NAUTY_INFINITY;
      else ++numcells;
    if (active_arg == NULL) {
      EMPTYSET(active, m);
      for (i = 0; i < n; ++i) {
        ADDELEMENT(active, i);
        while (ptn[i]) ++i;
      }
    } else
      for (i = 0; i < M; ++i) active[i] = active_arg[i];
  }

  g = canong = NULL;
  initstatus = 0;
  OPTCALL(dispatch.init) (g_arg, &g, canong_arg, &canong,
                          lab, ptn, active, options, &initstatus, m, n);
  if (initstatus) {
    stats->errstatus = initstatus;
    return;
  }

  if (g == NULL) g = g_arg;
  if (canong == NULL) canong = canong_arg;

#ifdef NAU_SCHREIER
  if (doschreier) newgroup(&gp, &gens, n);
#endif

  for (i = 0; i < n; ++i) orbits[i] = i;
  stats->grpsize1 = 1.0;
  stats->grpsize2 = 0;
  stats->numgenerators = 0;
  stats->numnodes = 0;
  stats->numbadleaves = 0;
  stats->tctotal = 0;
  stats->canupdates = 0;
  stats->numorbits = n;
  EMPTYSET(fixedpts, m);
  noncheaplevel = 1;
  eqlev_canon = -1;         /* needed even if !getcanon */

  if (worksize >= 2 * m)
    workspace = ws_arg;
  else {
    workspace = defltwork;
    worksize = 2 * m;
  }
  worktop = workspace + (worksize - worksize % (2 * m));
  fmptr = workspace;

  /* here goes: */
  stats->errstatus = 0;
  needshortprune = FALSE;
  invarsuclevel = NAUTY_INFINITY;
  invapplics = invsuccesses = 0;

#if !MAXN
  firstpathnode0(lab, ptn, 1, numcells, &tcnode0);
#else
  firstpathnode(lab, ptn, 1, numcells);
#endif

  {
    if (getcanon) {
      (*dispatch.updatecan)(g, canong, canonlab, samerows, M, n);
      for (i = 0; i < n; ++i) lab[i] = canonlab[i];
    }
    stats->invarsuclevel =
      (invarsuclevel == NAUTY_INFINITY ? 0 : invarsuclevel);
    stats->invapplics = invapplics;
    stats->invsuccesses = invsuccesses;
  }

#if !MAXN
  if (n >= 320) {
    nautil_freedyn();
    OPTCALL(dispatch.freedyn) ();
    nauty_freedyn();
  }
#endif
  OPTCALL(dispatch.cleanup) (g_arg, &g, canong_arg, &canong,
                             lab, ptn, options, stats, m, n);

#ifdef NAU_SCHREIER
  if (doschreier) {
    freeschreier(&gp, &gens);
    if (n >= 320) schreier_freedyn();
  }
#endif
}



/*****************************************************************************
*                                                                            *
*  firstpathnode(lab,ptn,level,numcells) produces a node on the leftmost     *
*  path down the tree.  The parameters describe the level and the current    *
*  colour partition.  The set of active cells is taken from the global set   *
*  'active'.  If the refined partition is not discrete, the leftmost child   *
*  is produced by calling firstpathnode, and the other children by calling   *
*  othernode.                                                                *
*  For MAXN=0 there is an extra parameter: the address of the parent tcell   *
*  structure.                                                                *
*  The value returned is the level to return to.                             *
*                                                                            *
*  FUNCTIONS CALLED: (*usernodeproc)(),doref(),cheapautom(),                 *
*                    firstterminal(),nextelement(),breakout(),               *
*                    firstpathnode(),othernode(),recover(),writestats(),     *
*                    (*userlevelproc)(),(*tcellproc)(),shortprune()          *
*                                                                            *
*****************************************************************************/

static int
#if !MAXN
firstpathnode0(int *lab, int *ptn, int level, int numcells,
               tcnode *tcnode_parent)
#else
firstpathnode(int *lab, int *ptn, int level, int numcells)
#endif
{
  int tv;
  int tv1, index, rtnlevel, tcellsize, tc, childcount, qinvar, refcode;
#if !MAXN
  set *tcell;
  tcnode *tcnode_this;

  tcnode_this = tcnode_parent->next;
  if (tcnode_this == NULL) {
    if ((tcnode_this = (tcnode*)ALLOCS(1, sizeof(tcnode))) == NULL ||
        (tcnode_this->tcellptr
           = (set*)ALLOCS(alloc_m, sizeof(set))) == NULL)
      alloc_error("tcell");
    tcnode_parent->next = tcnode_this;
    tcnode_this->next = NULL;
  }
  tcell = tcnode_this->tcellptr;
#else
  set tcell[MAXM];
#endif

  ++stats->numnodes;

  /* refine partition : */
  doref(g, lab, ptn, level, &numcells, &qinvar, workperm1,
        active, &refcode, dispatch.refine, invarproc,
        mininvarlevel, maxinvarlevel, invararg, digraph, M, n);
  firstcode[level] = (short)refcode;
  if (qinvar > 0) {
    ++invapplics;
    if (qinvar == 2) {
      ++invsuccesses;
      if (mininvarlevel < 0) mininvarlevel = level;
      if (maxinvarlevel < 0) maxinvarlevel = level;
      if (level < invarsuclevel) invarsuclevel = level;
    }
  }

  tc = -1;
  if (numcells != n) {
    /* locate new target cell, setting tc to its position in lab, tcell
                     to its contents, and tcellsize to its size: */
    maketargetcell(g, lab, ptn, level, tcell, &tcellsize,
                   &tc, tc_level, -1, dispatch.targetcell, M, n);
    stats->tctotal += tcellsize;
  }
  firsttc[level] = tc;

  /* optionally call user-defined node examination procedure: */
  OPTCALL(usernodeproc)
    (g, lab, ptn, level, numcells, tc, (int)firstcode[level], M, n);

  if (numcells == n) {      /* found first leaf? */
    firstterminal(lab, level);
    OPTCALL(userlevelproc) (lab, ptn, level, orbits, stats, 0, 1, 1, n, 0, n);
    return level - 1;
  }

  if (noncheaplevel >= level
      && !(*dispatch.cheapautom)(ptn, level, digraph, n))
    noncheaplevel = level + 1;

  /* use the elements of the target cell to produce the children: */
  index = 0;
  for (tv1 = tv = nextelement(tcell, M, -1); tv >= 0;
       tv = nextelement(tcell, M, tv)) {
    if (orbits[tv] == tv) {     /* ie, not equiv to previous child */
      breakout(lab, ptn, level + 1, tc, tv, active, M);
      ADDELEMENT(fixedpts, tv);
      cosetindex = tv;
      if (tv == tv1) {
#if !MAXN
        rtnlevel = firstpathnode0(lab, ptn, level + 1, numcells + 1,
                                  tcnode_this);
#else
        rtnlevel = firstpathnode(lab, ptn, level + 1, numcells + 1);
#endif
        childcount = 1;
        gca_first = level;
        stabvertex = tv1;
      } else {
#if !MAXN
        rtnlevel = othernode0(lab, ptn, level + 1, numcells + 1,
                              tcnode_this);
#else
        rtnlevel = othernode(lab, ptn, level + 1, numcells + 1);
#endif
        ++childcount;
      }
      DELELEMENT(fixedpts, tv);
      if (rtnlevel < level)
        return rtnlevel;
      if (needshortprune) {
        needshortprune = FALSE;
        shortprune(tcell, fmptr - M, M);
      }
      recover(ptn, level);
    }
    if (orbits[tv] == tv1)      /* ie, in same orbit as tv1 */
      ++index;
  }
  MULTIPLY(stats->grpsize1, stats->grpsize2, index);

  if (tcellsize == index && allsamelevel == level + 1)
    --allsamelevel;

  if (domarkers)
    writemarker(level, tv1, index, tcellsize, stats->numorbits, numcells);
  OPTCALL(userlevelproc) (lab, ptn, level, orbits, stats, tv1, index, tcellsize,
                          numcells, childcount, n);
  return level - 1;
}

/*****************************************************************************
*                                                                            *
*  othernode(lab,ptn,level,numcells) produces a node other than an ancestor  *
*  of the first leaf.  The parameters describe the level and the colour      *
*  partition.  The list of active cells is found in the global set 'active'. *
*  The value returned is the level to return to.                             *
*                                                                            *
*  FUNCTIONS CALLED: (*usernodeproc)(),doref(),refine(),recover(),           *
*                    processnode(),cheapautom(),(*tcellproc)(),shortprune(), *
*                    nextelement(),breakout(),othernode(),longprune()        *
*                                                                            *
*****************************************************************************/

static int
#if !MAXN
othernode0(int *lab, int *ptn, int level, int numcells,
           tcnode *tcnode_parent)
#else
othernode(int *lab, int *ptn, int level, int numcells)
#endif
{
  int tv;
  int tv1, refcode, rtnlevel, tcellsize, tc, qinvar;
  short code;
#if !MAXN
  set *tcell;
  tcnode *tcnode_this;

  tcnode_this = tcnode_parent->next;
  if (tcnode_this == NULL) {
    if ((tcnode_this = (tcnode*)ALLOCS(1, sizeof(tcnode))) == NULL ||
        (tcnode_this->tcellptr
           = (set*)ALLOCS(alloc_m, sizeof(set))) == NULL)
      alloc_error("tcell");
    tcnode_parent->next = tcnode_this;
    tcnode_this->next = NULL;
  }
  tcell = tcnode_this->tcellptr;
#else
  set tcell[MAXM];
#endif

  ++stats->numnodes;

  /* refine partition : */
  doref(g, lab, ptn, level, &numcells, &qinvar, workperm1, active,
        &refcode, dispatch.refine, invarproc, mininvarlevel, maxinvarlevel,
        invararg, digraph, M, n);
  code = (short)refcode;
  if (qinvar > 0) {
    ++invapplics;
    if (qinvar == 2) {
      ++invsuccesses;
      if (level < invarsuclevel) invarsuclevel = level;
    }
  }

  if (eqlev_first == level - 1 && code == firstcode[level])
    eqlev_first = level;
  if (getcanon) {
    if (eqlev_canon == level - 1) {
      if (code < canoncode[level])
        comp_canon = -1;
      else if (code > canoncode[level])
        comp_canon = 1;
      else {
        comp_canon = 0;
        eqlev_canon = level;
      }
    }
    if (comp_canon > 0) canoncode[level] = code;
  }

  tc = -1;
  /* If children will be required, find new target cell and set tc to its
     position in lab, tcell to its contents, and tcellsize to its size: */

  if (numcells < n && (eqlev_first == level ||
                       (getcanon && comp_canon >= 0))) {
    if (!getcanon || comp_canon < 0) {
      maketargetcell(g, lab, ptn, level, tcell, &tcellsize, &tc,
                     tc_level, firsttc[level], dispatch.targetcell, M, n);
      if (tc != firsttc[level]) eqlev_first = level - 1;
    } else
      maketargetcell(g, lab, ptn, level, tcell, &tcellsize, &tc,
                     tc_level, -1, dispatch.targetcell, M, n);
    stats->tctotal += tcellsize;
  }

  /* optionally call user-defined node examination procedure: */
  OPTCALL(usernodeproc) (g, lab, ptn, level, numcells, tc, (int)code, M, n);

  /* call processnode to classify the type of this node: */

  rtnlevel = processnode(lab, ptn, level, numcells);
  if (rtnlevel < level)     /* keep returning if necessary */
    return rtnlevel;
  if (needshortprune) {
    needshortprune = FALSE;
    shortprune(tcell, fmptr - M, M);
  }

  if (!(*dispatch.cheapautom)(ptn, level, digraph, n))
    noncheaplevel = level + 1;

  /* use the elements of the target cell to produce the children: */
  for (tv1 = tv = nextelement(tcell, M, -1); tv >= 0;
       tv = nextelement(tcell, M, tv)) {
    breakout(lab, ptn, level + 1, tc, tv, active, M);
    ADDELEMENT(fixedpts, tv);
#if !MAXN
    rtnlevel = othernode0(lab, ptn, level + 1, numcells + 1, tcnode_this);
#else
    rtnlevel = othernode(lab, ptn, level + 1, numcells + 1);
#endif
    DELELEMENT(fixedpts, tv);

    if (rtnlevel < level) return rtnlevel;
    /* use stored automorphism data to prune target cell: */
    if (needshortprune) {
      needshortprune = FALSE;
      shortprune(tcell, fmptr - M, M);
    }
    if (tv == tv1) {
      longprune(tcell, fixedpts, workspace, fmptr, M);
#ifdef NAU_SCHREIER
      if (doschreier) pruneset(fixedpts, gp, &gens, tcell, M, n);
#endif
    }

    recover(ptn, level);
  }

  return level - 1;
}

/*****************************************************************************
*                                                                            *
*  Process the first leaf of the tree.                                       *
*                                                                            *
*  FUNCTIONS CALLED: NONE                                                    *
*                                                                            *
*****************************************************************************/

static void
firstterminal(int *lab, int level)
{
  int i;

  stats->maxlevel = level;
  gca_first = allsamelevel = eqlev_first = level;
  firstcode[level + 1] = 077777;
  firsttc[level + 1] = -1;

  for (i = 0; i < n; ++i) firstlab[i] = lab[i];

  if (getcanon) {
    canonlevel = eqlev_canon = gca_canon = level;
    comp_canon = 0;
    samerows = 0;
    for (i = 0; i < n; ++i) canonlab[i] = lab[i];
    for (i = 0; i <= level; ++i) canoncode[i] = firstcode[i];
    canoncode[level + 1] = 077777;
    stats->canupdates = 1;
  }
}



/*****************************************************************************
*                                                                            *
*  Process a node other than the first leaf or its ancestors.  It is first   *
*  classified into one of five types and then action is taken appropriate    *
*  to that type.  The types are                                              *
*                                                                            *
*  0:   Nothing unusual.  This is just a node internal to the tree whose     *
*         children need to be generated sometime.                            *
*  1:   This is a leaf equivalent to the first leaf.  The mapping from       *
*         firstlab to lab is thus an automorphism.  After processing the     *
*         automorphism, we can return all the way to the closest invocation  *
*         of firstpathnode.                                                  *
*  2:   This is a leaf equivalent to the bsf leaf.  Again, we have found an  *
*         automorphism, but it may or may not be as useful as one from a     *
*         type-1 node.  Return as far up the tree as possible.               *
*  3:   This is a new bsf node, provably better than the previous bsf node.  *
*         After updating canonlab etc., treat it the same as type 4.         *
*  4:   This is a leaf for which we can prove that no descendant is          *
*         equivalent to the first or bsf leaf or better than the bsf leaf.   *
*         Return up the tree as far as possible, but this may only be by     *
*         one level.                                                         *
*                                                                            *
*  Types 2 and 3 can't occur if getcanon==FALSE.                             *
*  The value returned is the level in the tree to return to, which can be    *
*  anywhere up to the closest invocation of firstpathnode.                   *
*                                                                            *
*  FUNCTIONS CALLED:    isautom(),updatecan(),testcanlab(),fmperm(),         *
*                       writeperm(),(*userautomproc)(),orbjoin(),            *
*                       shortprune(),fmptn()                                 *
*                                                                            *
*****************************************************************************/

static int
processnode(int *lab, int *ptn, int level, int numcells)
{
  int i, code, save, newlevel;
  boolean ispruneok;
  int sr;

  code = 0;
  if (eqlev_first != level && (!getcanon || comp_canon < 0))
    code = 4;
  else if (numcells == n) {
    if (eqlev_first == level) {
      for (i = 0; i < n; ++i) workperm1[firstlab[i]] = lab[i];

      if (gca_first >= noncheaplevel ||
          (*dispatch.isautom)(g, workperm1, digraph, M, n))
        code = 1;
    }
    if (code == 0) {
      if (getcanon) {
        sr = 0;
        if (comp_canon == 0) {
          if (level < canonlevel)
            comp_canon = 1;
          else {
            (*dispatch.updatecan)
              (g, canong, canonlab, samerows, M, n);
            samerows = n;
            comp_canon
              = (*dispatch.testcanlab)(g, canong, lab, &sr, M, n);
          }
        }
        if (comp_canon == 0) {
          for (i = 0; i < n; ++i) workperm1[canonlab[i]] = lab[i];
          code = 2;
        } else if (comp_canon > 0)
          code = 3;
        else
          code = 4;
      } else
        code = 4;
    }
  }

  if (code != 0 && level > stats->maxlevel) stats->maxlevel = level;

  switch (code) {
  case 0:                   /* nothing unusual noticed */
    return level;

  case 1:                   /* lab is equivalent to firstlab */
    if (fmptr == worktop) fmptr -= 2 * M;
    fmperm(workperm1, fmptr, fmptr + M, M, n);
    fmptr += 2 * M;
    if (writeautoms)
      writeperm(outfile, workperm1, cartesian, linelength, n);
    stats->numorbits = orbjoin(orbits, workperm1, n);
    ++stats->numgenerators;
    OPTCALL(userautomproc) (stats->numgenerators, workperm1, orbits,
                            stats->numorbits, stabvertex, n);
#ifdef NAU_SCHREIER
    if (doschreier) addgenerator(&gp, &gens, workperm1, n);
#endif
    return gca_first;

  case 2:                   /* lab is equivalent to canonlab */
    if (fmptr == worktop) fmptr -= 2 * M;
    fmperm(workperm1, fmptr, fmptr + M, M, n);
    fmptr += 2 * M;
    save = stats->numorbits;
    stats->numorbits = orbjoin(orbits, workperm1, n);
    if (stats->numorbits == save) {
      if (gca_canon != gca_first) needshortprune = TRUE;
      return gca_canon;
    }
    if (writeautoms)
      writeperm(outfile, workperm1, cartesian, linelength, n);
    ++stats->numgenerators;
    OPTCALL(userautomproc) (stats->numgenerators, workperm1, orbits,
                            stats->numorbits, stabvertex, n);
#ifdef NAU_SCHREIER
    if (doschreier) addgenerator(&gp, &gens, workperm1, n);
#endif
    if (orbits[cosetindex] < cosetindex)
      return gca_first;
    if (gca_canon != gca_first)
      needshortprune = TRUE;
    return gca_canon;

  case 3:                   /* lab is better than canonlab */
    ++stats->canupdates;
    for (i = 0; i < n; ++i) canonlab[i] = lab[i];
    canonlevel = eqlev_canon = gca_canon = level;
    comp_canon = 0;
    canoncode[level + 1] = 077777;
    samerows = sr;
    break;

  case 4:                  /* non-automorphism terminal node */
    ++stats->numbadleaves;
    break;
  }    /* end of switch statement */

  /* only cases 3 and 4 get this far: */
  if (level != noncheaplevel) {
    ispruneok = TRUE;
    if (fmptr == worktop) fmptr -= 2 * M;
    fmptn(lab, ptn, noncheaplevel, fmptr, fmptr + M, M, n);
    fmptr += 2 * M;
  } else
    ispruneok = FALSE;

  save = (allsamelevel > eqlev_canon ? allsamelevel - 1 : eqlev_canon);
  newlevel = (noncheaplevel <= save ? noncheaplevel - 1 : save);

  if (ispruneok && newlevel != gca_first) needshortprune = TRUE;
  return newlevel;
}



/*****************************************************************************
*                                                                            *
*  Recover the partition nest at level 'level' and update various other      *
*  parameters.                                                               *
*                                                                            *
*  FUNCTIONS CALLED: NONE                                                    *
*                                                                            *
*****************************************************************************/

static void
recover(int *ptn, int level)
{
  int i;

  for (i = 0; i < n; ++i)
    if (ptn[i] > level) ptn[i] = NAUTY_INFINITY;

  if (level < noncheaplevel) noncheaplevel = level + 1;
  if (level < eqlev_first) eqlev_first = level;
  if (getcanon) {
    if (level < gca_canon) gca_canon = level;
    if (level <= eqlev_canon) {
      eqlev_canon = level;
      comp_canon = 0;
    }
  }
}



/*****************************************************************************
*                                                                            *
*  Write statistics concerning an ancestor of the first leaf.                *
*                                                                            *
*  level = its level                                                         *
*  tv = the vertex fixed to get the first child = the smallest-numbered      *
*               vertex in the target cell                                    *
*  cellsize = the size of the target cell                                    *
*  index = the number of vertices in the target cell which were equivalent   *
*               to tv = the index of the stabiliser of tv in the group       *
*               fixing the colour partition at this level                    *
*                                                                            *
*  numorbits = the number of orbits of the group generated by all the        *
*               automorphisms so far discovered                              *
*                                                                            *
*  numcells = the total number of cells in the equitable partition at this   *
*               level                                                        *
*                                                                            *
*****************************************************************************/

static void writemarker(int level, int tv, int index, int tcellsize,
                        int numorbits, int numcells)
{
  fprintf(outfile, "level %d: ", level);
  if (numcells != numorbits) {
    fprintf(outfile, "%d cell%s", numcells, (numcells > 1) ? "s" : "");
  }
  fprintf(outfile, "%d orbit%s;", numorbits, (numorbits > 1) ? "s" : "");
  fprintf(outfile, "%d fixed; index %d", tv + labelorg, index);
  if (tcellsize != index) {
    fprintf(outfile, "/%d", tcellsize);
  }
  fprintf(outfile, "\n");
}



/*****************************************************************************
*                                                                            *
*  nauty_check() checks that this file is compiled compatibly with the       *
*  given parameters.   If not, call exit(1).                                 *
*                                                                            *
*****************************************************************************/

void
nauty_check(int wordsize, int m, int n, int version)
{
  if (wordsize != WORDSIZE) {
    fprintf(ERRFILE, "Error: WORDSIZE mismatch in nauty.c\n");
    exit(1);
  }

#if MAXN
  if (m > MAXM) {
    fprintf(ERRFILE, "Error: MAXM inadequate in nauty.c\n");
    exit(1);
  }

  if (n > MAXN) {
    fprintf(ERRFILE, "Error: MAXN inadequate in nauty.c\n");
    exit(1);
  }
#endif

  if (version < NAUTYREQUIRED) {
    fprintf(ERRFILE, "Error: nauty.c version mismatch\n");
    exit(1);
  }
}



/*****************************************************************************
*                                                                            *
*  extra_autom(p,n)  - add an extra automophism, hard to do correctly        *
*                                                                            *
*****************************************************************************/

void
extra_autom(int *p, int n)
{
  if (writeautoms)
    writeperm(outfile, p, cartesian, linelength, n);
  stats->numorbits = orbjoin(orbits, p, n);
  ++stats->numgenerators;
  OPTCALL(userautomproc) (stats->numgenerators, p, orbits,
                          stats->numorbits, stabvertex, n);
}



/*****************************************************************************
*                                                                            *
*  extra_level(level,lab,ptn,numcells,tv1,index,tcellsize,childcount)        *
*     creates an artificial level in the search.  This is dangerous.         *
*                                                                            *
*****************************************************************************/

void extra_level(int level, int *lab, int *ptn, int numcells, int tv1, int index,
                 int tcellsize, int childcount, int n)
{
  MULTIPLY(stats->grpsize1, stats->grpsize2, index);
  if (domarkers)
    writemarker(level, tv1, index, tcellsize, stats->numorbits, numcells);
  OPTCALL(userlevelproc) (lab, ptn, level, orbits, stats, tv1, index, tcellsize,
                          numcells, childcount, n);
}



/*****************************************************************************
*                                                                            *
*  nauty_freedyn() frees all the dynamic memory used in this module.         *
*                                                                            *
*****************************************************************************/

void nauty_freedyn(void)
{
#if !MAXN
  tcnode *tcp, *tcq;

  tcp = tcnode0.next;
  while (tcp != NULL) {
    tcq = tcp->next;
    FREES(tcp->tcellptr);
    FREES(tcp);
    tcp = tcq;
  }
  alloc_m = 0;
  tcnode0.next = NULL;
  DYNFREE(firsttc, firsttc_sz);
  DYNFREE(canoncode, canoncode_sz);
  DYNFREE(firstcode, firstcode_sz);
  DYNFREE(workperm1, workperm1_sz);
  DYNFREE(canonlab, canonlab_sz);
  DYNFREE(firstlab, firstlab_sz);
  DYNFREE(defltwork, defltwork_sz);
  DYNFREE(fixedpts, fixedpts_sz);
  DYNFREE(active, active_sz);
#endif
}



#endif /* NAUTY_C__ */



#ifndef NAUGRAPH_C__
#define NAUGRAPH_C__


/* macros for hash-codes: */
#define MASH(l, i) ((((l) ^ 065435) + (i)) & 077777)
/* : expression whose long value depends only on long l and int/long i.
   Anything goes, preferably non-commutative. */

#define CLEANUP(l) ((int)((l) % 077777))
/* : expression whose value depends on long l and is less than 077777
   when converted to int then short.  Anything goes. */

#if !MAXN
DYNALLSTAT(set, workset, workset_sz);
DYNALLSTAT(int, workperm2, workperm2_sz);
DYNALLSTAT(int, bucket, bucket_sz);
DYNALLSTAT(set, dnwork, dnwork_sz);
#else
static set workset[MAXM];   /* used for scratch work */
static int workperm2[MAXN];
static int bucket[MAXN + 2];
static set dnwork[40 * MAXM];
#endif



/*****************************************************************************
*                                                                            *
*  isautom(g,perm,digraph,m,n) = TRUE iff perm is an automorphism of g       *
*  (i.e., g^perm = g).  Symmetry is assumed unless digraph = TRUE.           *
*                                                                            *
*****************************************************************************/

boolean
isautom(graph *g, int *perm, boolean digraph, int m, int n)
{
  set *pg;
  int pos;
  set *pgp;
  int posp, i;

  for (pg = g, i = 0; i < n; pg += M, ++i) {
    pgp = GRAPHROW(g, perm[i], M);
    pos = (digraph ? -1 : i);

    while ((pos = nextelement(pg, M, pos)) >= 0) {
      posp = perm[pos];
      if (!ISELEMENT(pgp, posp)) return FALSE;
    }
  }
  return TRUE;
}



/*****************************************************************************
*                                                                            *
*  testcanlab(g,canong,lab,samerows,m,n) compares g^lab to canong,           *
*  using an ordering which is immaterial since it's only used here.  The     *
*  value returned is -1,0,1 if g^lab <,=,> canong.  *samerows is set to      *
*  the number of rows (0..n) of canong which are the same as those of g^lab. *
*                                                                            *
*  GLOBALS ACCESSED: workset<rw>,permset(),workperm2<rw>                      *
*                                                                            *
*****************************************************************************/

int
testcanlab(graph *g, graph *canong, int *lab, int *samerows, int m, int n)
{
  int i, j;
  set *ph;

#if !MAXN
  DYNALLOC1(int, workperm2, workperm2_sz, n, "testcanlab");
  DYNALLOC1(set, workset, workset_sz, m, "testcanlab");
#endif

  for (i = 0; i < n; ++i) workperm2[lab[i]] = i;

  for (i = 0, ph = canong; i < n; ++i, ph += M) {
    permset(GRAPHROW(g, lab[i], M), workset, M, workperm2);
    for (j = 0; j < M; ++j)
      if (workset[j] < ph[j]) {
        *samerows = i;
        return -1;
      } else if (workset[j] > ph[j]) {
        *samerows = i;
        return 1;
      }
  }

  *samerows = n;
  return 0;
}



/*****************************************************************************
*                                                                            *
*  updatecan(g,canong,lab,samerows,m,n) sets canong = g^lab, assuming        *
*  the first samerows of canong are ok already.                              *
*                                                                            *
*  GLOBALS ACCESSED: permset(),workperm2<rw>                                  *
*                                                                            *
*****************************************************************************/

void
updatecan(graph *g, graph *canong, int *lab, int samerows, int m, int n)
{
  int i;
  set *ph;

#if !MAXN
  DYNALLOC1(int, workperm2, workperm2_sz, n, "updatecan");
#endif

  for (i = 0; i < n; ++i) workperm2[lab[i]] = i;

  for (i = samerows, ph = GRAPHROW(canong, samerows, M);
       i < n; ++i, ph += M)
    permset(GRAPHROW(g, lab[i], M), ph, M, workperm2);
}



/*****************************************************************************
*                                                                            *
*  refine(g,lab,ptn,level,numcells,count,active,code,m,n) performs a         *
*  refinement operation on the partition at the specified level of the       *
*  partition nest (lab,ptn).  *numcells is assumed to contain the number of  *
*  cells on input, and is updated.  The initial set of active cells (alpha   *
*  in the paper) is specified in the set active.  Precisely, x is in active  *
*  iff the cell starting at index x in lab is active.                        *
*  The resulting partition is equitable if active is correct (see the paper  *
*  and the Guide).                                                           *
*  *code is set to a value which depends on the fine detail of the           *
*  algorithm, but which is independent of the labelling of the graph.        *
*  count is used for work space.                                             *
*                                                                            *
*  GLOBALS ACCESSED:  workset<w>,bit<r>,nextelement(),bucket<w>,workperm2<w>  *
*                                                                            *
*****************************************************************************/

void refine(graph *g, int *lab, int *ptn, int level, int *numcells,
            int *count, set *active, int *code, int m, int n)
{
#if MAXM == 1
  refine1(g, lab, ptn, level, numcells, count, active, code, m, n);
#else
  int i, c1, c2, labc1;
  setword x;
  set *set1, *set2;
  int split1, split2, cell1, cell2;
  int cnt, bmin, bmax;
  long longcode;
  set *gptr;
  int maxcell, maxpos, hint;

#if !MAXN
  DYNALLOC1(int, workperm2, workperm2_sz, n, "refine");
  DYNALLOC1(set, workset, workset_sz, m, "refine");
  DYNALLOC1(int, bucket, bucket_sz, n + 2, "refine");
#endif

  longcode = *numcells;
  split1 = -1;
  hint = 0;
  while (*numcells < n && ((split1 = hint, ISELEMENT(active, split1))
                           || (split1 = nextelement(active, M, split1)) >= 0
                           || (split1 = nextelement(active, M, -1)) >= 0)) {
    DELELEMENT(active, split1);
    for (split2 = split1; ptn[split2] > level; ++split2) {
    }
    longcode = MASH(longcode, split1 + split2);
    if (split1 == split2) {         /* trivial splitting cell */
      gptr = GRAPHROW(g, lab[split1], M);
      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > level; ++cell2) {
        }
        if (cell1 == cell2) continue;
        c1 = cell1;
        c2 = cell2;
        while (c1 <= c2) {
          labc1 = lab[c1];
          if (ISELEMENT(gptr, labc1))
            ++c1;
          else {
            lab[c1] = lab[c2];
            lab[c2] = labc1;
            --c2;
          }
        }
        if (c2 >= cell1 && c1 <= cell2) {
          ptn[c2] = level;
          longcode = MASH(longcode, c2);
          ++*numcells;
          if (ISELEMENT(active, cell1) || c2 - cell1 >= cell2 - c1) {
            ADDELEMENT(active, c1);
            if (c1 == cell2) hint = c1;
          } else {
            ADDELEMENT(active, cell1);
            if (c2 == cell1) hint = cell1;
          }
        }
      }
    } else {        /* nontrivial splitting cell */
      EMPTYSET(workset, m);
      for (i = split1; i <= split2; ++i)
        ADDELEMENT(workset, lab[i]);
      longcode = MASH(longcode, split2 - split1 + 1);

      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > level; ++cell2) {
        }
        if (cell1 == cell2) continue;
        i = cell1;
        set1 = workset;
        set2 = GRAPHROW(g, lab[i], m);
        cnt = 0;
        for (c1 = m; --c1 >= 0; )
          if ((x = (*set1++) & (*set2++)) != 0)
            cnt += POPCOUNT(x);

        count[i] = bmin = bmax = cnt;
        bucket[cnt] = 1;
        while (++i <= cell2) {
          set1 = workset;
          set2 = GRAPHROW(g, lab[i], m);
          cnt = 0;
          for (c1 = m; --c1 >= 0; )
            if ((x = (*set1++) & (*set2++)) != 0)
              cnt += POPCOUNT(x);

          while (bmin > cnt) bucket[--bmin] = 0;
          while (bmax < cnt) bucket[++bmax] = 0;
          ++bucket[cnt];
          count[i] = cnt;
        }
        if (bmin == bmax) {
          longcode = MASH(longcode, bmin + cell1);
          continue;
        }
        c1 = cell1;
        maxcell = -1;
        for (i = bmin; i <= bmax; ++i)
          if (bucket[i]) {
            c2 = c1 + bucket[i];
            bucket[i] = c1;
            longcode = MASH(longcode, i + c1);
            if (c2 - c1 > maxcell) {
              maxcell = c2 - c1;
              maxpos = c1;
            }
            if (c1 != cell1) {
              ADDELEMENT(active, c1);
              if (c2 - c1 == 1) hint = c1;
              ++*numcells;
            }
            if (c2 <= cell2) ptn[c2 - 1] = level;
            c1 = c2;
          }
        for (i = cell1; i <= cell2; ++i)
          workperm2[bucket[count[i]]++] = lab[i];
        for (i = cell1; i <= cell2; ++i) lab[i] = workperm2[i];
        if (!ISELEMENT(active, cell1)) {
          ADDELEMENT(active, cell1);
          DELELEMENT(active, maxpos);
        }
      }
    }
  }

  longcode = MASH(longcode, *numcells);
  *code = CLEANUP(longcode);
#endif /* else case of MAXM==1 */
}



/*****************************************************************************
*                                                                            *
*  refine1(g,lab,ptn,level,numcells,count,active,code,m,n) is the same as    *
*  refine(g,lab,ptn,level,numcells,count,active,code,m,n), except that       *
*  m==1 is assumed for greater efficiency.  The results are identical in all *
*  respects.  See refine (above) for the specs.                              *
*                                                                            *
*****************************************************************************/

void refine1(graph *g, int *lab, int *ptn, int level, int *numcells,
             int *count, set *active, int *code, int m, int n)
{
  int i, c1, c2, labc1;
  setword x;
  int split1, split2, cell1, cell2;
  int cnt, bmin, bmax;
  long longcode;
  set *gptr, workset0;
  int maxcell, maxpos, hint;

  if (m != 1) exit(1);
#if !MAXN
  DYNALLOC1(int, workperm2, workperm2_sz, n, "refine1");
  DYNALLOC1(int, bucket, bucket_sz, n + 2, "refine1");
#endif

  longcode = *numcells;
  split1 = -1;

  hint = 0;
  while (*numcells < n && ((split1 = hint, ISELEMENT1(active, split1))
                           || (split1 = nextelement(active, 1, split1)) >= 0
                           || (split1 = nextelement(active, 1, -1)) >= 0)) {
    DELELEMENT1(active, split1);
    for (split2 = split1; ptn[split2] > level; ++split2) {
    }
    longcode = MASH(longcode, split1 + split2);
    if (split1 == split2) {         /* trivial splitting cell */
      gptr = GRAPHROW(g, lab[split1], 1);
      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > level; ++cell2) {
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
          ptn[c2] = level;
          longcode = MASH(longcode, c2);
          ++*numcells;
          if (ISELEMENT1(active, cell1) || c2 - cell1 >= cell2 - c1) {
            ADDELEMENT1(active, c1);
            if (c1 == cell2) hint = c1;
          } else {
            ADDELEMENT1(active, cell1);
            if (c2 == cell1) hint = cell1;
          }
        }
      }
    } else {        /* nontrivial splitting cell */
      workset0 = 0;
      for (i = split1; i <= split2; ++i)
        ADDELEMENT1(&workset0, lab[i]);
      longcode = MASH(longcode, split2 - split1 + 1);

      for (cell1 = 0; cell1 < n; cell1 = cell2 + 1) {
        for (cell2 = cell1; ptn[cell2] > level; ++cell2) {
        }
        if (cell1 == cell2) continue;
        i = cell1;
        if ((x = workset0 & g[lab[i]]) != 0)
          cnt = POPCOUNT(x);
        else
          cnt = 0;
        count[i] = bmin = bmax = cnt;
        bucket[cnt] = 1;
        while (++i <= cell2) {
          if ((x = workset0 & g[lab[i]]) != 0)
            cnt = POPCOUNT(x);
          else
            cnt = 0;
          while (bmin > cnt) bucket[--bmin] = 0;
          while (bmax < cnt) bucket[++bmax] = 0;
          ++bucket[cnt];
          count[i] = cnt;
        }
        if (bmin == bmax) {
          longcode = MASH(longcode, bmin + cell1);
          continue;
        }
        c1 = cell1;
        maxcell = -1;
        for (i = bmin; i <= bmax; ++i)
          if (bucket[i]) {
            c2 = c1 + bucket[i];
            bucket[i] = c1;
            longcode = MASH(longcode, i + c1);
            if (c2 - c1 > maxcell) {
              maxcell = c2 - c1;
              maxpos = c1;
            }
            if (c1 != cell1) {
              ADDELEMENT1(active, c1);
              if (c2 - c1 == 1) hint = c1;
              ++*numcells;
            }
            if (c2 <= cell2) ptn[c2 - 1] = level;
            c1 = c2;
          }
        for (i = cell1; i <= cell2; ++i)
          workperm2[bucket[count[i]]++] = lab[i];
        for (i = cell1; i <= cell2; ++i) lab[i] = workperm2[i];
        if (!ISELEMENT1(active, cell1)) {
          ADDELEMENT1(active, cell1);
          DELELEMENT1(active, maxpos);
        }
      }
    }
  }

  longcode = MASH(longcode, *numcells);
  *code = CLEANUP(longcode);
}



/*****************************************************************************
*                                                                            *
*  cheapautom(ptn,level,digraph,n) returns TRUE if the partition at the      *
*  specified level in the partition nest (lab,ptn) {lab is not needed here}  *
*  satisfies a simple sufficient condition for its cells to be the orbits of *
*  some subgroup of the automorphism group.  Otherwise it returns FALSE.     *
*  It always returns FALSE if digraph!=FALSE.                                *
*                                                                            *
*  nauty assumes that this function will always return TRUE for any          *
*  partition finer than one for which it returns TRUE.                       *
*                                                                            *
*****************************************************************************/

boolean
cheapautom(int *ptn, int level, boolean digraph, int n)
{
  int i, k, nnt;

  if (digraph) return FALSE;

  k = n;
  nnt = 0;
  for (i = 0; i < n; ++i) {
    --k;
    if (ptn[i] > level) {
      ++nnt;
      while (ptn[++i] > level) {
      }
    }
  }

  return(k <= nnt + 1 || k <= 4);
}



/*****************************************************************************
*                                                                            *
*  bestcell(g,lab,ptn,level,n) returns the index in lab of the    *
*  start of the "best non-singleton cell" for fixing.  If there is no        *
*  non-singleton cell it returns n.                                          *
*  This implementation finds the first cell which is non-trivially joined    *
*  to the greatest number of other cells.                                    *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>,workperm2<rw>,workset<rw>,bucket<rw>              *
*                                                                            *
*****************************************************************************/

static int bestcell(graph *g, int *lab, int *ptn, int level, int n)
{
  int i;
  set *gp;
  setword setword1, setword2;
  int v1, v2, nnt;

#if !MAXN
  DYNALLOC1(int, workperm2, workperm2_sz, n, "bestcell");
  DYNALLOC1(set, workset, workset_sz, m, "bestcell");
  DYNALLOC1(int, bucket, bucket_sz, n + 2, "bestcell");
#endif

  /* find non-singleton cells: put starts in workperm2[0..nnt-1] */

  i = nnt = 0;

  while (i < n) {
    if (ptn[i] > level) {
      workperm2[nnt++] = i;
      while (ptn[i] > level) ++i;
    }
    ++i;
  }

  if (nnt == 0) return n;

  /* set bucket[i] to # non-trivial neighbours of n.s. cell i */

  for (i = nnt; --i >= 0; ) bucket[i] = 0;

  for (v2 = 1; v2 < nnt; ++v2) {
    EMPTYSET(workset, m);
    i = workperm2[v2] - 1;
    do {
      ++i;
      ADDELEMENT(workset, lab[i]);
    } while (ptn[i] > level);
    for (v1 = 0; v1 < v2; ++v1) {
      gp = GRAPHROW(g, lab[workperm2[v1]], m);
#if  MAXM == 1
      setword1 = *workset & *gp;
      setword2 = *workset & ~*gp;
#else
      setword1 = setword2 = 0;
      for (i = m; --i >= 0; ) {
        setword1 |= workset[i] & gp[i];
        setword2 |= workset[i] & ~gp[i];
      }
#endif
      if (setword1 != 0 && setword2 != 0) {
        ++bucket[v1];
        ++bucket[v2];
      }
    }
  }

  /* find first greatest bucket value */

  v1 = 0;
  v2 = bucket[0];
  for (i = 1; i < nnt; ++i)
    if (bucket[i] > v2) {
      v1 = i;
      v2 = bucket[i];
    }

  return (int)workperm2[v1];
}



/*****************************************************************************
*                                                                            *
*  targetcell(g,lab,ptn,level,tc_level,hint,m,n) returns the index   *
*  in lab of the next cell to split.                                         *
*  hint is a suggestion for the answer, which is obeyed if it is valid.      *
*  Otherwise we use bestcell() up to tc_level and the first non-trivial      *
*  cell after that.                                                          *
*                                                                            *
*****************************************************************************/

int targetcell(graph *g, int *lab, int *ptn, int level, int tc_level,
               int hint, int n)
{
  int i;

  if (hint >= 0 && ptn[hint] > level &&
      (hint == 0 || ptn[hint - 1] <= level))
    return hint;
  else if (level <= tc_level)
    return bestcell(g, lab, ptn, level, n);
  else {
    for (i = 0; i < n && ptn[i] <= level; ++i) {
    }
    return(i == n ? 0 : i);
  }
}



void naugraph_freedyn(void)
{
#if !MAXN
  DYNFREE(workset, workset_sz);
  DYNFREE(workperm2, workperm2_sz);
  DYNFREE(bucket, bucket_sz);
  DYNFREE(dnwork, dnwork_sz);
#endif
}



dispatchvec dispatch_graph =
{ isautom, testcanlab, updatecan, refine, refine1, cheapautom, targetcell,
  naugraph_freedyn, nauty_check, NULL, NULL };



void densenauty(graph * RESTRICT g,
                int * RESTRICT lab, int * RESTRICT ptn,
                int * RESTRICT orbits,
                optionblk *options, statsblk * RESTRICT stats,
                int m, int n, graph * RESTRICT h)
{
  if (options->dispatch != &dispatch_graph) {
    fprintf(ERRFILE, "Error: densenauty() needs standard options block\n");
    exit(1);
  }

#if !MAXN
  DYNALLOC1(set, dnwork, dnwork_sz, 40 * m, "densenauty malloc");
#endif

  nauty(g, lab, ptn, NULL, orbits,
        options, stats, dnwork, 40 * m, m, n, h);
}



#endif /* NAUGRAPH_C__ */



#ifdef __INTEL_COMPILER
  #pragma warning pop
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) \
  && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
  /* diagnostic push and pop are added in GCC 4.6 */
  #pragma GCC diagnostic pop
#endif



#endif /* NAU0_H__ */

