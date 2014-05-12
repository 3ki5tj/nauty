/* combine all essential nauty functions and definitions
 *  nauty.h nautil.c sorttemplates.c nauty.c naugraph.c
 * Mimimal version:
 *  o disables schreier
 *  o removes dynamic allocation
 *  o remove user functions
 *  o remove writemarker, writeperm
 * This file was edited manually */
#ifndef NAU0S_H__
#define NAU0S_H__



#ifdef __INTEL_COMPILER
  #pragma warning push
/* 1418: external function
 * 981:  evaluated unspecified order
 * 161:  unrecognized pragma
 * 869:  unused variables */
  #pragma warning disable 1418 981 161 869
#elif defined(__GNUC__) && defined(__GNUC__MINOR__)
  #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
    /* diagnostic push and pop are added in GCC 4.6 */
    #pragma GCC diagnostic push
  #endif
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunknown-pragmas"
#elif defined(_MSC_VER)
  /* disable CRT security warning for "_s" versions */
  #ifndef _CRT_SECURE_NO_DEPRECATE
  #define _CRT_SECURE_NO_DEPRECATE 1
  #endif
  #ifndef _CRT_SECURE_NO_WARNINGS
  #define _CRT_SECURE_NO_WARNINGS 1
  #endif
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

#ifndef ONE_WORD_SETS
#define ONE_WORD_SETS
#endif

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
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*   16-bit, 32-bit and 64-bit versions can be selected by defining WORDSIZE. *
*   The largest graph that can be handled has MAXN vertices.                 *
*   Both WORDSIZE and MAXN can be defined on the command line.               *
*   WORDSIZE must be 16, 32 or 64; MAXN must be <= NAUTY_INFINITY-2;         *
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



/* define uint16_t/uint32_t/uint64_t, etc. */
#if ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) \
  || defined(__GNUC__) || defined(__INTEL_COMPILER))
/* C99 compatible compilers support int64_t etc.
 * but GCC and other compilers has the header even in C89/C90 mode
 * So we need to include more compilers here, see the list on
 * http://sourceforge.net/p/predef/wiki/Compilers/ */
  #include <inttypes.h>
#elif (defined(_MSC_VER) \
  || (defined(__BORLANDC__) && (__BORLANDC__ >= 0x520)))
/* tested for Visual C++ 6.0 and Borland C++ 5.5 */
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#elif defined(__unix__)
/* a modern unix compiler is likely to have inttypes.h  */
  #include <inttypes.h>
#else
/* note the following is a guess, long long is not supported
 * until a later version of visual C++ */
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef unsigned long long uint64_t;
#endif

#if WORDSIZE == 16
typedef uint16_t setword;
  #define SETWORD_SHORT
#endif

#if WORDSIZE == 32
typedef uint32_t setword;
  #if SIZEOF_INT >= 4
  #define SETWORD_INT
  #else
  #define SETWORD_LONG
  #endif
#endif

#if WORDSIZE == 64
typedef uint64_t setword;
  #if SIZEOF_INT >= 8
    #define SETWORD_INT
  #elif SIZEOF_LONG >= 8
    #define SETWORD_LONG
  #else
    #define SETWORD_LONGLONG
  #endif /* SIZEOF_INT >= 8 */
#endif /* WORDSIZE == 64 */

#define NAUTYVERSIONID (25480)  /* 10000*version */
#define NAUTYREQUIRED NAUTYVERSIONID  /* Minimum compatible version */

#if WORDSIZE == 16
#define NAUTYVERSION "2.5 (16 bits)"
#elif WORDSIZE == 32
#define NAUTYVERSION "2.5 (32 bits)"
#elif WORDSIZE == 64
#define NAUTYVERSION "2.5 (64 bits)"
#endif

#ifndef MAXN  /* maximum allowed n value; use 0 for dynamic sizing. */
#define MAXN WORDSIZE
#define MAXM 1
#else
#define MAXM ((MAXN + WORDSIZE - 1) / WORDSIZE)  /* max setwords in a set */
#endif  /* MAXN */

#if MAXM == 1
#define NAUTY_M_ 1
#define NAUTY_DEFM_(x)
#else
#define NAUTY_M_ m
#define NAUTY_DEFM_(x) int m = x;
#endif

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
#define SETWD(pos) ((pos) >> 5)       /* pos / WORDSIZE */
#define SETBT(pos) ((pos) & 0x1F)     /* pos % WORDSIZE */
#define TIMESWORDSIZE(w) ((w) << 5)   /* w * WORDSIZE */
#define SETWORDSNEEDED(n) ((((n) - 1) >> 5) + 1)
#endif

#if  WORDSIZE == 64
#define SETWD(pos) ((pos) >> 6)       /* pos / WORDSIZE */
#define SETBT(pos) ((pos) & 0x3F)     /* pos % WORDSIZE */
#define TIMESWORDSIZE(w) ((w) << 6)    /* w * WORDSIZE */
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
  int numorbits;            /* number of orbits in group */
  int numgenerators;        /* number of generators found */
  int errstatus;            /* if non-zero : an error code */
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
} dispatchvec_t;

typedef struct optionstruct {
  int getcanon;               /* make canong and canonlab? */
  boolean digraph;            /* multiple edges or loops? */
  boolean defaultptn;         /* set lab,ptn,active for single cell? */
  void (*invarproc)           /* procedure to compute vertex-invariant */
  (graph *, int*, int*, int, int, int, int*, int, boolean, int, int);
  int tc_level;               /* max level for smart target cell choosing */
  int mininvarlevel;          /* min level for invariant computation */
  int maxinvarlevel;          /* max level for invariant computation */
  int invararg;               /* value passed to (*invarproc)() */
  dispatchvec_t *dispatch;    /* vector of object-specific routines */
} optionblk;



#define DEFAULTOPTIONS_GRAPH(options) optionblk options = \
{ 0, FALSE, TRUE, \
  NULL, 100, 0, 1, 0, &dispatch_graph }



#define DEFAULTOPTIONS_DIGRAPH(options) optionblk options = \
{ 0, TRUE, TRUE, \
  adjacencies, 100, 0, 999, 0, &dispatch_graph }



#ifndef DEFAULTOPTIONS
#define DEFAULTOPTIONS DEFAULTOPTIONS_GRAPH
#endif

/* array giving setwords with single 1-bit */
#if  WORDSIZE == 64
#ifdef SETWORD_LONGLONG
static const
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
static const
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
static const
setword bit[] = { 020000000000, 010000000000, 04000000000, 02000000000,
                  01000000000, 0400000000, 0200000000, 0100000000, 040000000,
                  020000000, 010000000, 04000000, 02000000, 01000000, 0400000,
                  0200000, 0100000, 040000, 020000, 010000, 04000, 02000, 01000,
                  0400, 0200, 0100, 040, 020, 010, 04, 02, 01 };
#endif

#if WORDSIZE == 16
static const
setword bit[] = { 0100000, 040000, 020000, 010000, 04000, 02000, 01000, 0400, 0200,
                  0100, 040, 020, 010, 04, 02, 01 };
#endif

/*  array giving number of 1-bits in bytes valued 0..255: */
static const
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
static const
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

extern void permset(set*, set*, int, int*);
extern void doref(graph *, int*, int*, int, int*, int*, int*, set *, int*,
                  void (*)(graph*, int*, int*, int, int*, int*, set*, int*, int, int),
                  void (*)(graph*, int*, int*, int, int, int, int*, int, boolean, int, int),
                  int, int, int, boolean, int, int);

boolean isautom(graph *, int*, boolean, int, int);
int testcanlab(graph*, graph*, int*, int*, int, int);
void updatecan(graph*, graph*, int*, int, int, int);
void refine1(graph *g, int *lab, int *ptn, int level, int *numcells,
             int *count, set *active, int *code, int m, int n);
void refine(graph *g, int *lab, int *ptn, int level, int *numcells,
            int *count, set *active, int *code, int m, int n);
boolean cheapautom(int*, int, boolean, int);
int targetcell(graph *g, int *lab, int *ptn, int level, int tc_level,
               int hint, int n);
dispatchvec_t dispatch_graph =
{ isautom, testcanlab, updatecan, refine, refine1, cheapautom, targetcell };




#endif /* NAUTY_H__ */




#ifndef NAUTIL_H__
#define NAUTIL_H__


/* macros for hash-codes: */
/* Don't use NAUTY_INFINITY here as that would make the canonical
 * labelling depend on whether BIGNAUTY is in operation */
#define MASH(l, i) ((((l) ^ 065435) + (i)) & 077777)
/* : expression whose long value depends only on long l and int/long i.
     Anything goes, preferably non-commutative. */

#define CLEANUP(l) ((int)((l) % 077777))
/* : expression whose value depends on long l and is less than 077777
     when converted to int then short.  Anything goes. */

static int workperm0[MAXN];
#pragma omp threadprivate(workperm0)

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

INLINE int nextelement(set *set1, int m, int pos)
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
*  orbits represents a partition of {0,1,...,n-1}, by orbits[i] = the        *
*  smallest element in the same cell as i.  map[] is any array with values   *
*  in {0,1,...,n-1}.  orbjoin(orbits,map,n) joins the cells of orbits[]      *
*  together to the minimum extent such that for each i, i and map[i] are in  *
*  the same cell.  The function value returned is the new number of cells.   *
*                                                                            *
*  GLOBALS ACCESSED: NONE                                                    *
*                                                                            *
*****************************************************************************/

INLINE int orbjoin(int *orbits, int *map, int n)
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
*  fmperm(perm,fix,mcr,m,n) uses perm to construct fix and mcr.  fix         *
*  contains those points are fixed by perm, while mcr contains the set of    *
*  those points which are least in their orbits.                             *
*                                                                            *
*  GLOBALS ACCESSED: bit<r>                                                  *
*                                                                            *
*****************************************************************************/

INLINE void fmperm(int *perm, set *fix, set *mcr, int m, int n)
{
  int i, k, l;

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

INLINE void fmptn(int *lab, int *ptn, int level, set *fix, set *mcr, int m, int n)
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
 *   SORT_MINPARTITION = least number of elements for using quicksort
 *           partitioning, otherwise insertion sort is used (default "11")
 *   SORT_MINMEDIAN9 = least number of elements for using the median of 3
 *           medians of 3 for partitioning (default "320")
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

INLINE void sortparallel(SORT_TYPE1 *x, SORT_TYPE2 *y, int n)
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

  if ((tvpos = nextelement(active, NAUTY_M_, -1)) < 0) tvpos = 0;

  (*refproc)(g, lab, ptn, level, numcells, invar, active, code, NAUTY_M_, n);

  minlev = (mininvarlev < 0 ? -mininvarlev : mininvarlev);
  maxlev = (maxinvarlev < 0 ? -maxinvarlev : maxinvarlev);
  if (invarproc != NULL && *numcells < n
      && level >= minlev && level <= maxlev) {
    (*invarproc)(g, lab, ptn, level, *numcells, tvpos, invar, invararg,
                 digraph, NAUTY_M_, n);
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
      (*refproc)(g, lab, ptn, level, numcells, invar, active, code, NAUTY_M_, n);
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
                    int *tcellsize, int *cellpos, int tc_level, int hint,
                    int (*targetcell)(graph*, int*, int*, int, int, int, int),
                    int m, int n)
{
  int i, j, k;

  i = (*targetcell)(g, lab, ptn, level, tc_level, hint, n);
  for (j = i + 1; ptn[j] > level; ++j)
    ;

  *tcellsize = j - i + 1;

  EMPTYSET(tcell, m);
  for (k = i; k <= j; ++k) ADDELEMENT(tcell, lab[k]);

  *cellpos = i;
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

INLINE void breakout(int *lab, int *ptn, int level, int tc, int tv,
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

INLINE void longprune(set *tcell, set *fix, set *bottom, set *top, int m)
{
  int i;

  while (bottom < top) {
    for (i = 0; i < NAUTY_M_; ++i)
      if (NOTSUBSET(fix[i], bottom[i])) break;
    bottom += NAUTY_M_;

    if (i == NAUTY_M_)
      for (i = 0; i < NAUTY_M_; ++i) INTERSECT(tcell[i], bottom[i]);
    bottom += NAUTY_M_;
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

INLINE void writegroupsize(FILE *f, double gpsize1, int gpsize2)
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



#endif /* NAUTIL_H__ */


#ifndef NAUTY_C__
#define NAUTY_C__

typedef struct tcnode_struct {
  struct tcnode_struct *next;
  set *tcellptr;
} tcnode;

static int firstpathnode(int*, int*, int, int, graph *g, int n);
static int othernode(int*, int*, int, int, graph *g, int n);
static int processnode(int*, int*, int, int, graph *g, int n);

/* copies of some of the options: */
static boolean getcanon, digraph;
static int tc_level, mininvarlevel, maxinvarlevel, invararg;
static void (*invarproc)
(graph *, int*, int*, int, int, int, int*, int, boolean, int, int);
static dispatchvec_t dispatch;
#pragma omp threadprivate(getcanon, digraph, tc_level)
#pragma omp threadprivate(mininvarlevel, maxinvarlevel, invararg)
#pragma omp threadprivate(invarproc, dispatch)

/* local versions of some of the arguments: */
#if MAXM != 1
static int nauty_m_;
#pragma omp threadprivate(nauty_m_)
#endif
static graph *canong;
static int *orbits;
static statsblk *stats;
/* temporary versions of some stats: */
static unsigned long invapplics, invsuccesses;
static int invarsuclevel;
#pragma omp threadprivate(canong, orbits, stats)
#pragma omp threadprivate(invapplics, invsuccesses, invarsuclevel)

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
           cosetindex; /* the point being fixed at level gca_first */
#pragma omp threadprivate(gca_first, gca_canon, noncheaplevel, allsamelevel)
#pragma omp threadprivate(eqlev_first, eqlev_canon, comp_canon)
#pragma omp threadprivate(samerows, canonlevel, cosetindex)

static boolean needshortprune;  /* used to flag calls to shortprune */
#pragma omp threadprivate(needshortprune)

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
#pragma omp threadprivate(defltwork, workperm1, fixedpts, firstlab, canonlab)
#pragma omp threadprivate(firstcode, canoncode, firsttc, active)

static set *workspace, *worktop;  /* first and just-after-last
                                     addresses of work area to hold automorphism data */
static set *fmptr;                   /* pointer into workspace */
#pragma omp threadprivate(workspace, worktop, fmptr)


/*****************************************************************************
*                                                                            *
*  nauty_check() checks that this file is compiled compatibly with the       *
*  given parameters.   If not, call exit(1).                                 *
*                                                                            *
*****************************************************************************/

INLINE void nauty_check(int wordsize, int m, int n, int version)
{
  if (wordsize != WORDSIZE) {
    fprintf(stderr, "Error: WORDSIZE mismatch in nauty.c\n");
    exit(1);
  }

  if (m > MAXM) {
    fprintf(stderr, "Error: MAXM inadequate in nauty.c\n");
    exit(1);
  }

  if (n > MAXN) {
    fprintf(stderr, "Error: MAXN inadequate in nauty.c\n");
    exit(1);
  }

  if (version < NAUTYREQUIRED) {
    fprintf(stderr, "Error: nauty.c version mismatch\n");
    exit(1);
  }
}



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
  graph *g = NULL;
  int i, n, m;
  int numcells;
  int initstatus;

  /* determine dispatch vector */

  if (options->dispatch == NULL) {
    fprintf(stderr, ">E nauty: null dispatch vector\n");
    fprintf(stderr, "Maybe you need to recompile\n");
    exit(1);
  } else
    dispatch = *(options->dispatch);

  if (dispatch.refine1 && m_arg == 1)
    dispatch.refine = dispatch.refine1;

  if (dispatch.refine == NULL || dispatch.updatecan == NULL
      || dispatch.targetcell == NULL || dispatch.cheapautom == NULL) {
    fprintf(stderr, ">E bad dispatch vector\n");
    exit(1);
  }

  /* check for excessive sizes: */
  if (m_arg > MAXM) {
    stats_arg->errstatus = MTOOBIG;
    fprintf(stderr, "nauty: need m <= %d\n\n", MAXM);
    return;
  }
  if (n_arg > MAXN || n_arg > WORDSIZE * m_arg) {
    stats_arg->errstatus = NTOOBIG;
    fprintf(stderr,
            "nauty: need n <= min(%d,%d*m)\n\n", MAXM, WORDSIZE);
    return;
  }
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
#if MAXM != 1
  NAUTY_M_ = m;
#endif
  n = n_arg;

  nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

  /* OLD g = g_arg; */
  orbits = orbits_arg;
  stats = stats_arg;

  getcanon = options->getcanon;
  digraph = options->digraph;
  if (digraph) tc_level = 0;
  else tc_level = options->tc_level;

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
      fprintf(stderr,
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
      for (i = 0; i < NAUTY_M_; ++i) active[i] = active_arg[i];
  }

  initstatus = 0;
  if (initstatus) {
    stats->errstatus = initstatus;
    return;
  }

  g = g_arg;
  canong = canong_arg;

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

  firstpathnode(lab, ptn, 1, numcells, g, n);

  if (getcanon) {
    (*dispatch.updatecan)(g, canong, canonlab, samerows, NAUTY_M_, n);
    for (i = 0; i < n; ++i) lab[i] = canonlab[i];
  }
  stats->invarsuclevel =
    (invarsuclevel == NAUTY_INFINITY ? 0 : invarsuclevel);
  stats->invapplics = invapplics;
  stats->invsuccesses = invsuccesses;
}



/*****************************************************************************
*                                                                            *
*  Process the first leaf of the tree.                                       *
*                                                                            *
*****************************************************************************/

INLINE void firstterminal(int *lab, int level, int n)
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
*  Recover the partition nest at level 'level' and update various other      *
*  parameters.                                                               *
*                                                                            *
*****************************************************************************/

INLINE void recover(int *ptn, int level, int n)
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
*  shortprune(set1,set2,m) ANDs the contents of set set2 into set set1.      *
*                                                                            *
*****************************************************************************/

INLINE void shortprune(set *set1, set *set2, int m)
{
  int i;

  for (i = 0; i < NAUTY_M_; ++i) INTERSECT(set1[i], set2[i]);
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
*                    (*tcellproc)(),shortprune()                             *
*                                                                            *
*****************************************************************************/

static int firstpathnode(int *lab, int *ptn, int level, int numcells, graph *g, int n)
{
  int tv;
  int tv1, index, rtnlevel, tcellsize = 0, tc, childcount = 0, qinvar, refcode;
  set tcell[MAXM];
  NAUTY_DEFM_(nauty_m_);

  ++stats->numnodes;

  /* refine partition : */
  doref(g, lab, ptn, level, &numcells, &qinvar, workperm1,
        active, &refcode, dispatch.refine, invarproc,
        mininvarlevel, maxinvarlevel, invararg, digraph, NAUTY_M_, n);
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
                   &tc, tc_level, -1, dispatch.targetcell, NAUTY_M_, n);
    stats->tctotal += tcellsize;
  }
  firsttc[level] = tc;

  if (numcells == n) {      /* found first leaf? */
    firstterminal(lab, level, n);
    return level - 1;
  }

  if (noncheaplevel >= level
      && !(*dispatch.cheapautom)(ptn, level, digraph, n))
    noncheaplevel = level + 1;

  /* use the elements of the target cell to produce the children: */
  index = 0;
  for (tv1 = tv = nextelement(tcell, NAUTY_M_, -1); tv >= 0;
       tv = nextelement(tcell, NAUTY_M_, tv)) {
    if (orbits[tv] == tv) {     /* ie, not equiv to previous child */
      breakout(lab, ptn, level + 1, tc, tv, active, NAUTY_M_);
      ADDELEMENT(fixedpts, tv);
      cosetindex = tv;
      if (tv == tv1) {
        rtnlevel = firstpathnode(lab, ptn, level + 1, numcells + 1, g, n);
        childcount = 1;
        gca_first = level;
      } else {
        rtnlevel = othernode(lab, ptn, level + 1, numcells + 1, g, n);
        ++childcount;
      }
      DELELEMENT(fixedpts, tv);
      if (rtnlevel < level)
        return rtnlevel;
      if (needshortprune) {
        needshortprune = FALSE;
        shortprune(tcell, fmptr - NAUTY_M_, NAUTY_M_);
      }
      recover(ptn, level, n);
    }
    if (orbits[tv] == tv1)      /* ie, in same orbit as tv1 */
      ++index;
  }
  MULTIPLY(stats->grpsize1, stats->grpsize2, index);

  if (tcellsize == index && allsamelevel == level + 1)
    --allsamelevel;

  return level - 1;
}



/*****************************************************************************
*                                                                            *
*  othernode(lab, ptn, level, numcells, g, n)                                *
*  produces a node other than an ancestor                                    *
*  of the first leaf.  The parameters describe the level and the colour      *
*  partition.  The list of active cells is found in the global set 'active'. *
*  The value returned is the level to return to.                             *
*                                                                            *
*  FUNCTIONS CALLED: (*usernodeproc)(),doref(),refine(),recover(),           *
*                    processnode(),cheapautom(),(*tcellproc)(),shortprune(), *
*                    nextelement(),breakout(),othernode(),longprune()        *
*                                                                            *
*****************************************************************************/

static int othernode(int *lab, int *ptn, int level, int numcells, graph *g, int n)
{
  int tv;
  int tv1, refcode, rtnlevel, tcellsize, tc, qinvar;
  short code;
  set tcell[MAXM] = {0};
  NAUTY_DEFM_(nauty_m_);

  ++stats->numnodes;

  /* refine partition : */
  doref(g, lab, ptn, level, &numcells, &qinvar, workperm1, active,
        &refcode, dispatch.refine, invarproc, mininvarlevel, maxinvarlevel,
        invararg, digraph, NAUTY_M_, n);
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
                     tc_level, firsttc[level], dispatch.targetcell, NAUTY_M_, n);
      if (tc != firsttc[level]) eqlev_first = level - 1;
    } else
      maketargetcell(g, lab, ptn, level, tcell, &tcellsize, &tc,
                     tc_level, -1, dispatch.targetcell, NAUTY_M_, n);
    stats->tctotal += tcellsize;
  }

  /* call processnode to classify the type of this node: */

  rtnlevel = processnode(lab, ptn, level, numcells, g, n);
  if (rtnlevel < level)     /* keep returning if necessary */
    return rtnlevel;
  if (needshortprune) {
    needshortprune = FALSE;
    shortprune(tcell, fmptr - NAUTY_M_, NAUTY_M_);
  }

  if (!(*dispatch.cheapautom)(ptn, level, digraph, n))
    noncheaplevel = level + 1;

  /* use the elements of the target cell to produce the children: */
  for (tv1 = tv = nextelement(tcell, NAUTY_M_, -1); tv >= 0;
       tv = nextelement(tcell, NAUTY_M_, tv)) {
    breakout(lab, ptn, level + 1, tc, tv, active, NAUTY_M_);
    ADDELEMENT(fixedpts, tv);
    rtnlevel = othernode(lab, ptn, level + 1, numcells + 1, g, n);
    DELELEMENT(fixedpts, tv);

    if (rtnlevel < level) return rtnlevel;
    /* use stored automorphism data to prune target cell: */
    if (needshortprune) {
      needshortprune = FALSE;
      shortprune(tcell, fmptr - NAUTY_M_, NAUTY_M_);
    }
    if (tv == tv1) {
      longprune(tcell, fixedpts, workspace, fmptr, NAUTY_M_);
    }

    recover(ptn, level, n);
  }

  return level - 1;
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
*                       orbjoin(),                                           *
*                       shortprune(),fmptn()                                 *
*                                                                            *
*****************************************************************************/

static int processnode(int *lab, int *ptn, int level, int numcells, graph *g, int n)
{
  int i, code, save, newlevel;
  boolean ispruneok;
  int sr = 0;
  NAUTY_DEFM_(nauty_m_);

  code = 0;
  if (eqlev_first != level && (!getcanon || comp_canon < 0))
    code = 4;
  else if (numcells == n) {
    if (eqlev_first == level) {
      for (i = 0; i < n; ++i) workperm1[firstlab[i]] = lab[i];

      if (gca_first >= noncheaplevel ||
          (*dispatch.isautom)(g, workperm1, digraph, NAUTY_M_, n))
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
              (g, canong, canonlab, samerows, NAUTY_M_, n);
            samerows = n;
            comp_canon
              = (*dispatch.testcanlab)(g, canong, lab, &sr, NAUTY_M_, n);
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
    if (fmptr == worktop) fmptr -= 2 * NAUTY_M_;
    fmperm(workperm1, fmptr, fmptr + NAUTY_M_, NAUTY_M_, n);
    fmptr += 2 * NAUTY_M_;
    stats->numorbits = orbjoin(orbits, workperm1, n);
    ++stats->numgenerators;
    return gca_first;

  case 2:                   /* lab is equivalent to canonlab */
    if (fmptr == worktop) fmptr -= 2 * NAUTY_M_;
    fmperm(workperm1, fmptr, fmptr + NAUTY_M_, NAUTY_M_, n);
    fmptr += 2 * NAUTY_M_;
    save = stats->numorbits;
    stats->numorbits = orbjoin(orbits, workperm1, n);
    if (stats->numorbits == save) {
      if (gca_canon != gca_first) needshortprune = TRUE;
      return gca_canon;
    }
    ++stats->numgenerators;
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
    if (fmptr == worktop) fmptr -= 2 * NAUTY_M_;
    fmptn(lab, ptn, noncheaplevel, fmptr, fmptr + NAUTY_M_, NAUTY_M_, n);
    fmptr += 2 * NAUTY_M_;
  } else
    ispruneok = FALSE;

  save = (allsamelevel > eqlev_canon ? allsamelevel - 1 : eqlev_canon);
  newlevel = (noncheaplevel <= save ? noncheaplevel - 1 : save);

  if (ispruneok && newlevel != gca_first) needshortprune = TRUE;
  return newlevel;
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

static set workset[MAXM];   /* used for scratch work */
static int workperm2[MAXN];
static int bucket[MAXN + 2];
static set dnwork[40 * MAXM];
#pragma omp threadprivate(workset, workperm2, bucket, dnwork)


/*****************************************************************************
*                                                                            *
*  isautom(g,perm,digraph,m,n) = TRUE iff perm is an automorphism of g       *
*  (i.e., g^perm = g).  Symmetry is assumed unless digraph = TRUE.           *
*                                                                            *
*****************************************************************************/

boolean isautom(graph *g, int *perm, boolean digraph, int m, int n)
{
  set *pg;
  int pos;
  set *pgp;
  int posp, i;

  for (pg = g, i = 0; i < n; pg += NAUTY_M_, ++i) {
    pgp = GRAPHROW(g, perm[i], NAUTY_M_);
    pos = (digraph ? -1 : i);

    while ((pos = nextelement(pg, NAUTY_M_, pos)) >= 0) {
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

int testcanlab(graph *g, graph *canong, int *lab, int *samerows, int m, int n)
{
  int i, j;
  set *ph;

  for (i = 0; i < n; ++i) workperm2[lab[i]] = i;

  for (i = 0, ph = canong; i < n; ++i, ph += NAUTY_M_) {
    permset(GRAPHROW(g, lab[i], NAUTY_M_), workset, NAUTY_M_, workperm2);
    for (j = 0; j < NAUTY_M_; ++j)
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

void updatecan(graph *g, graph *canong, int *lab, int samerows, int m, int n)
{
  int i;
  set *ph;

  for (i = 0; i < n; ++i) workperm2[lab[i]] = i;

  for (i = samerows, ph = GRAPHROW(canong, samerows, NAUTY_M_);
       i < n; ++i, ph += NAUTY_M_)
    permset(GRAPHROW(g, lab[i], NAUTY_M_), ph, NAUTY_M_, workperm2);
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
  int maxcell, maxpos = 0, hint;

  if (m != 1) exit(1);
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

  longcode = *numcells;
  split1 = -1;
  hint = 0;
  while (*numcells < n && ((split1 = hint, ISELEMENT(active, split1))
                           || (split1 = nextelement(active, NAUTY_M_, split1)) >= 0
                           || (split1 = nextelement(active, NAUTY_M_, -1)) >= 0)) {
    DELELEMENT(active, split1);
    for (split2 = split1; ptn[split2] > level; ++split2) {
    }
    longcode = MASH(longcode, split1 + split2);
    if (split1 == split2) {         /* trivial splitting cell */
      gptr = GRAPHROW(g, lab[split1], NAUTY_M_);
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

boolean cheapautom(int *ptn, int level, boolean digraph, int n)
{
  int i, k, nnt;

  if (digraph) return FALSE;

  k = n;
  nnt = 0;
  for (i = 0; i < n; ++i) {
    --k;
    if (ptn[i] > level) {
      ++nnt;
      while (ptn[++i] > level)
        ;
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
  NAUTY_DEFM_(nauty_m_);

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
    for (i = 0; i < n && ptn[i] <= level; ++i)
      ;
    return(i == n ? 0 : i);
  }
}



#define densenauty(g, lab, ptn, orbits, options, stats, m, n, h) { \
    if ((options)->dispatch != &dispatch_graph) { \
      fprintf(stderr, "Error: densenauty() needs standard options block\n"); \
      exit(1); \
    } \
    nauty(g, lab, ptn, NULL, orbits, options, stats, dnwork, 2 * 60 * m, m, n, h); }



#endif /* NAUGRAPH_C__ */



#ifdef __INTEL_COMPILER
  #pragma warning pop
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) \
  && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
  /* diagnostic push and pop are added in GCC 4.6 */
  #pragma GCC diagnostic pop
#endif



#endif /* NAU0S_H__ */

