#ifndef PLUGIN_INIT
#define PLUGIN_INIT gcomplx_init(maxn);
#endif
#ifndef PLUGIN_DONE
#define PLUGIN_DONE gcomplx_done();
#endif
#ifndef OUTPROC
#define OUTPROC     gcomplx_dograph
#endif



#include <math.h>


#define PRECDBL   0
#define PRECLDBL  1
#define PRECF128  2
#define PRECMPQ   10

#if defined(MPQ) || defined(GMP) || defined(MP)

#include <gmp.h>
typedef mpq_t xdouble;
typedef xdouble xdblptr;

#define CANONICALIZE(x) mpq_canonicalize(x)
#define INIT(x)         { mpq_init(x); mpq_set_si(x, 0, 1); }
#define CLEAR(x)        mpq_clear(x)
#define SET(y, x)       mpq_set(y, x)
#define SETPTR(y, x)    mpq_set(y, x)
#define SET_SI(y, x)    mpq_set_si(y, x, 1)
#define SWAP(y, x)      mpq_swap(y, x)
#define SETDBL(y, x)    mpq_set_d(y, x)
#define GETDBL(x)       mpq_get_d(x)
#define CMP(y, x)       mpq_cmp(y, x)
#define ADD(c, a, b)    mpq_add(c, a, b)
#define SUB(c, a, b)    mpq_sub(c, a, b)
#define MUL(c, a, b)    mpq_mul(c, a, b)
#define DIV(c, a, b)    mpq_div(c, a, b)
#define NEG(y, x)       mpq_neg(y, x)
#define ABS(y, x)       mpq_abs(y, x)
#define INV(y, x)       mpq_inv(y, x)
#define SGN(x)          mpq_sgn(x)
#define PRINTF          gmp_printf
#define FPRINTF         gmp_fprintf
#define PRNREALE        "%Qd"
#define STRPREC         "mpq"
#define PRECTYPE        PRECMPQ

#else

#if defined(QUAD) || defined(F128)

  #include <quadmath.h>
  typedef __float128 xdouble;
  #define ABS(y, x)   y = (((x) < 0) ? -(x) : (x))
  #define SQRT(y, x)  y = sqrtq(x)
  #define PRNREALE    "%41.32Qe"
  #define STRPREC     "f128"
  #define PRECTYPE    PRECF128

#elif defined(LDBL) || defined(LONG)

  typedef long double xdouble;
  #define ABS(y, x)   y = fabsl(x)
  #define SQRT(y, x)  y = sqrtl(x)
  #define PRNREALE    "%27.18Le"
  #define STRPREC     "ldbl"
  #define PRECTYPE    PRECLDBL

#else

  typedef double xdouble;
  #define ABS(y, x)   y = fabs(x)
  #define SQRT(y, x)  y = sqrt(x)
  #define PRNREALE    "%22.14e"
  #define STRPREC     ""
  #define PRECTYPE    PRECDBL

#endif /* F128/LDBL/DBL */

typedef xdouble *xdblptr;

/* the following are common to native data types */
#define CANONICALIZE(x)
#define INIT(x)         x = 0
#define CLEAR(x)
#define SET(y, x)       (y) = (x)
#define SETPTR(y, x)    (*(y)) = (x)
#define SET_SI(y, x)    (y) = (xdouble) (x)
xdouble swapper_;
#define SWAP(y, x)      swapper_ = (y), (y) = (x), (x) = swapper_
#define SETDBL(y, x)    (y) = (xdouble) (x)
#define GETDBL(x)       ((double) x)
#define CMP(y, x)       ((y) - (x))
#define ADD(c, a, b)    (c) = ((a) + (b))
#define SUB(c, a, b)    (c) = ((a) - (b))
#define MUL(c, a, b)    (c) = ((a) * (b))
#define DIV(c, a, b)    (c) = ((a) / (b))
#define NEG(y, x)       (y) = (-(x))
#define INV(y, x)       (y) = ((xdouble)1/(x))
#define SGN(x)          ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define PRINTF          printf
#define FPRINTF         fprintf


#endif /* MPQ */



int mat_n;
xdouble **mat;
long **imat;

/* properties of the current graph */
xdouble det;
xdouble sym;
int nedges;


#ifndef xnew
#define xnew(x, n) \
  if (((x) = calloc(n, sizeof(*(x)))) == NULL) { \
    fprintf(stderr, "no memory for %s\n", #x); \
    exit(1); \
  }
#endif



static int gcomplx_init(int n)
{
  int i;

  mat_n = n - 1;
  xnew(imat, mat_n);
  xnew(imat[0], mat_n * mat_n);
  xnew(mat, mat_n);
  xnew(mat[0], mat_n * mat_n);
  for ( i = 0; i < mat_n * mat_n; i++ ) {
    INIT(mat[0][i]);
  }
  for ( i = 1; i < mat_n; i++ ) {
    mat[i] = mat[0] + i * mat_n;
    imat[i] = imat[0] + i * mat_n;
  }
  INIT(det);
  INIT(sym);
  return 0;
}



static int mat_det(xdblptr detptr, xdouble **a, int n)
{
  int i, j, k, im, sgn = 1;
  xdouble am, x, det;

  INIT(am);
  INIT(x);
  INIT(det);
  SET_SI(det, 1);
  for ( k = 0; k < n; k++ ) {
    /* choose a pivot */
    im = k;
    ABS(am, a[im][k]);
    for ( i = k + 1; i < n; i++ ) {
      ABS(x, a[i][k]);
      if ( CMP(x, am) > 0 ) {
        SET(am, x);
        im = i;
      }
    }
    /* swap row im and k */
    if ( im != k ) {
      for ( j = k; j < n; j++ ) {
        SWAP(a[im][j], a[k][j]);
      }
      if ( (im - k) % 2 ) sgn *= -1;
    }
    /* normalize row k */
    ABS(x, a[k][k]);
    if ( GETDBL(x) < 1e-10 ) {
      fprintf(stderr, "zero encounted! %d, am %g\n", k, GETDBL(a[k][k]));
      return -1;
    }
    INV(x, a[k][k]);
    MUL(det, det, a[k][k]);
    for ( j = k; j < n; j++ )
      MUL(a[k][j], a[k][j], x);
    /* eliminate the rest row */
    for ( i = k + 1; i < n; i++ ) {
      for ( j = k + 1; j < n; j++ ) {
        MUL(x, a[i][k], a[k][j]);
        SUB(a[i][j], a[i][j], x);
      }
    }
  }
  if ( sgn < 0 ) NEG(det, det);
  CANONICALIZE(det);
  SETPTR(detptr, det);
  CLEAR(am);
  CLEAR(x);
  CLEAR(det);
  return 0;
}



static void gcomplx_dograph(FILE *fp, graph *g, int n)
{
  int i, j;
  static double count = 0;

  nedges = 0;
  if ( n != mat_n + 1 ) {
    fprintf(stderr, "n %d vs %d mismatch\n", n, mat_n);
    exit(1);
  }

  for ( i = 0; i < mat_n; i++ )
    for ( j = 0; j < mat_n; j++ )
      imat[i][j] = 0;

  for ( i = 0; i < n; i++ ) {
    graph *gi = GRAPHROW(g, i, NAUTY_M_);
    for ( j = i + 1; j < n; j++ ) {
      if ( ISELEMENT(gi, j) ) {
        if ( i == 0 ) {
          imat[j-1][j-1] += 1;
        } else {
          imat[i-1][i-1] += 1;
          imat[j-1][j-1] += 1;
          imat[i-1][j-1] = imat[j-1][i-1] = -1;
        }
        nedges++;
      }
    }
  }

  if ( verbose ) {
    for ( i = 0; i < mat_n; i++ ) {
      for ( j = 0; j < mat_n; j++ )
        fprintf(stderr, "%ld\t", imat[i][j]);
      fprintf(stderr, "\n");
    }
  }

  for ( i = 0; i < mat_n; i++ )
    for ( j = 0; j < mat_n; j++ )
      SET_SI(mat[i][j], imat[i][j]);

#if PRECTYPE == PRECMPQ
  mat_det(det, mat, mat_n);
  /* assure that the det is an integer */
  {
    mpq_t x;
    mpz_t den;

    mpz_init(den);
    mpq_get_den(den, det);
    if ( mpz_cmp_si(den, 1) != 0 ) {
      fprintf(stderr, "denominator %d\n", mpz_get_si(den));
      exit(1);
    }
    mpz_clear(den);

    mpq_init(x);
    mpq_set_d(sym, nauty_stats.grpsize1);
    mpq_set_si(x, 10, 1);
    for ( i = 0; i < nauty_stats.grpsize2; i++ )
      mpq_mul(sym, sym, x);
    mpq_clear(x);
  }
#else
  mat_det(&det, mat, mat_n);
  sym = (xdouble) nauty_stats.grpsize1;
  for ( i = 0; i < nauty_stats.grpsize2; i++ )
    sym *= 10;
#endif

  if ( SGN(det) <= 0 ) {
    printf("bad determinant %g\n", GETDBL(det));
  }
  count++;
  if ( fp != NULL )
    fprintf(fp, "%d %.0f %.0f\n", nedges, GETDBL(det), GETDBL(sym));
  /* vir = sum_G 2^(n-1) (1-n) (-)^E / sqrt(det)^dim / sym */
}



static int gcomplx_done(void)
{
  CLEAR(det);
  CLEAR(sym);
  if ( mat != NULL ) {
    if ( mat[0] != NULL ) {
      int i;
      for ( i = 0; i < mat_n * mat_n; i++)
        CLEAR(mat[0][i]);
      free(mat[0]);
    }
    free(mat);
  }
  return 0;
}

