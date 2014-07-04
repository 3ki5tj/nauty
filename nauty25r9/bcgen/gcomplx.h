#ifndef PLUGIN_INIT
#define PLUGIN_INIT gcomplx_init(maxn);
#endif
#ifndef PLUGIN_DONE
#define PLUGIN_DONE gcomplx_done();
#endif
#ifndef OUTPROC
#define OUTPROC     gcomplx_dograph
#endif



#include <stdio.h>
#include <math.h>



#define XDBL_TYPE_DBL   0
#define XDBL_TYPE_LDBL  1
#define XDBL_TYPE_QUAD  2
#define XDBL_TYPE_F128  2
#define XDBL_TYPE_MPQ   10

#if defined(MPQ) || defined(GMP) || defined(MP)

#include <gmp.h>
typedef mpq_t xdouble;
typedef xdouble xdblptr;
#define XDBL_CANONICALIZE(x)  mpq_canonicalize(x)
#define XDBL_INIT(x)          { mpq_init(x); mpq_set_si(x, 0, 1); }
#define XDBL_CLEAR(x)         mpq_clear(x)
#define XDBL_SET(y, x)        mpq_set(y, x)
#define XDBL_SET_PTR(y, x)    mpq_set(y, x)
#define XDBL_SET_SI(y, x)     mpq_set_si(y, x, 1)
#define XDBL_SWAP(y, x)       mpq_swap(y, x)
#define XDBL_SET_DBL(y, x)    mpq_set_d(y, x)
#define XDBL_GET_DBL(x)       mpq_get_d(x)
#define XDBL_CMP(y, x)        mpq_cmp(y, x)
#define XDBL_ADD(c, a, b)     mpq_add(c, a, b)
#define XDBL_SUB(c, a, b)     mpq_sub(c, a, b)
#define XDBL_MUL(c, a, b)     mpq_mul(c, a, b)
#define XDBL_DIV(c, a, b)     mpq_div(c, a, b)
#define XDBL_ADDX(y, x)       mpq_add(y, y, x)
#define XDBL_SUBX(y, x)       mpq_sub(y, y, x)
#define XDBL_MULX(y, x)       mpq_mul(y, y, x)
#define XDBL_DIVX(y, x)       mpq_div(y, y, x)
#define XDBL_NEG(y, x)        mpq_neg(y, x)
#define XDBL_ABS(y, x)        mpq_abs(y, x)
#define XDBL_INV(y, x)        mpq_inv(y, x)
#define XDBL_ROUND(d, x)      mpz_tdiv_q(d, mpq_getnum(x), mpq_getden(x))
#define XDBL_SGN(x)           mpq_sgn(x)
#define XDBL_PRINTF           gmp_printf
#define XDBL_FPRINTF          gmp_fprintf
#define XDBL_PRND             "%Qd"
#define XDBL_PRNE             "%Qd"
#define XDBL_STRPREC          "mpq"
#define XDBL_TYPE             XDBL_TYPE_MPQ

#else

#if defined(QUAD) || defined(F128)

  #include <quadmath.h>
  typedef __float128 xdouble;
  #define XDBL_ABS(y, x)      y = (((x) < 0) ? -(x) : (x))
  #define XDBL_SQRT(y, x)     y = sqrtq(x)
  #define XDBL_ROUND(y, x)    y = roundq(x)
  #define XDBL_SNPRINTF       quadmath_snprintf
  #define XDBL_PRN            "Q"
  #define XDBL_PRND           "%.0Qf"
  #define XDBL_PRNE           "%41.32Qe"
  #define XDBL_STRPREC        "f128"
  #define XDBL_TYPE           XDBL_TYPE_QUAD

#elif defined(LDBL) || defined(LONG)

  typedef long double xdouble;
  #define XDBL_ABS(y, x)      y = fabsl(x)
  #define XDBL_SQRT(y, x)     y = sqrtl(x)
  #define XDBL_ROUND(y, x)    y = roundl(x)
  #define XDBL_SNPRINTF       snprintf
  #define XDBL_PRN            "L"
  #define XDBL_PRND           "%.0Lf"
  #define XDBL_PRNE           "%27.18Le"
  #define XDBL_STRPREC        "ldbl"
  #define XDBL_TYPE           XDBL_TYPE_LDBL

#else

  typedef double xdouble;
  #define XDBL_ABS(y, x)      y = fabs(x)
  #define XDBL_SQRT(y, x)     y = sqrt(x)
  #define XDBL_ROUND(y, x)    y = round(x)
  #define XDBL_SNPRINTF       snprintf
  #define XDBL_PRN            ""
  #define XDBL_PRND           "%.0f"
  #define XDBL_PRNE           "%22.14e"
  #define XDBL_STRPREC        ""
  #define XDBL_TYPE           XDBL_TYPE_DBL

#endif /* QUAD/LDBL/DBL */

typedef xdouble *xdblptr;
/* the following are common to native data types */
#define XDBL_CANONICALIZE(x)
#define XDBL_INIT(x)          x = 0
#define XDBL_CLEAR(x)
#define XDBL_SET(y, x)        (y) = (x)
#define XDBL_SET_PTR(y, x)    (*(y)) = (x)
#define XDBL_SET_SI(y, x)     (y) = (xdouble) (x)
xdouble xdbl_swapper_;
#define XDBL_SWAP(y, x)       xdbl_swapper_ = (y), (y) = (x), (x) = xdbl_swapper_
#define XDBL_SET_DBL(y, x)    (y) = (xdouble) (x)
#define XDBL_GET_DBL(x)       ((double) x)
#define XDBL_CMP(y, x)        ((y) - (x))
#define XDBL_ADD(c, a, b)     (c) = ((a) + (b))
#define XDBL_SUB(c, a, b)     (c) = ((a) - (b))
#define XDBL_MUL(c, a, b)     (c) = ((a) * (b))
#define XDBL_DIV(c, a, b)     (c) = ((a) / (b))
#define XDBL_ADDX(y, x)       (y) += (x)
#define XDBL_SUBX(y, x)       (y) -= (x)
#define XDBL_MULX(y, x)       (y) *= (x)
#define XDBL_DIVX(y, x)       (y) /= (x)
#define XDBL_NEG(y, x)        (y) = (-(x))
#define XDBL_INV(y, x)        (y) = ((xdouble)1/(x))
#define XDBL_SGN(x)           ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define XDBL_PRINTF           printf
#define XDBL_FPRINTF          fprintf

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
    XDBL_INIT(mat[0][i]);
  }
  for ( i = 1; i < mat_n; i++ ) {
    mat[i] = mat[0] + i * mat_n;
    imat[i] = imat[0] + i * mat_n;
  }
  XDBL_INIT(det);
  XDBL_INIT(sym);

#if XDBL_TYPE == XDBL_TYPE_QUAD
  { /* call quadmath_snprintf() to enable the printf hook */
    __float128 q = 1000;
    char s[99];
    quadmath_snprintf(s, sizeof s, "%20.0Qf\n", q);
  }
#endif
  return 0;
}



static int mat_det(xdblptr detptr, xdouble **a, int n)
{
  int i, j, k, im, sgn = 1;
  xdouble am, x, det;

  XDBL_INIT(am);
  XDBL_INIT(x);
  XDBL_INIT(det);
  XDBL_SET_SI(det, 1);
  for ( k = 0; k < n; k++ ) {
    /* choose a pivot */
    im = k;
    XDBL_ABS(am, a[im][k]);
    for ( i = k + 1; i < n; i++ ) {
      XDBL_ABS(x, a[i][k]);
      if ( XDBL_CMP(x, am) > 0 ) {
        XDBL_SET(am, x);
        im = i;
      }
    }
    /* swap row im and k */
    if ( im != k ) {
      for ( j = k; j < n; j++ ) {
        XDBL_SWAP(a[im][j], a[k][j]);
      }
      if ( (im - k) % 2 ) sgn *= -1;
    }
    /* normalize row k */
    XDBL_ABS(x, a[k][k]);
    if ( XDBL_GET_DBL(x) < 1e-10 ) {
      fprintf(stderr, "zero encounted! %d, am %g\n",
          k, XDBL_GET_DBL(a[k][k]));
      return -1;
    }
    XDBL_INV(x, a[k][k]);
    XDBL_MULX(det, a[k][k]);
    for ( j = k; j < n; j++ ) {
      XDBL_MULX(a[k][j], x);
      //XDBL_CANONICALIZE(a[k][j]);
    }
    /* eliminate the rest of the row */
    for ( i = k + 1; i < n; i++ ) {
      for ( j = k + 1; j < n; j++ ) {
        XDBL_MUL(x, a[i][k], a[k][j]);
        XDBL_SUBX(a[i][j], x);
      }
    }
  }
  if ( sgn < 0 ) XDBL_NEG(det, det);
  XDBL_CANONICALIZE(det);
  XDBL_SET_PTR(detptr, det);
  XDBL_CLEAR(am);
  XDBL_CLEAR(x);
  XDBL_CLEAR(det);
  return 0;
}



static void gcomplx_dograph(FILE *fp, graph *g, int n)
{
  int i, j, n2 = mat_n * mat_n;
  static double count = 0;

  nedges = 0;
  if ( n != mat_n + 1 ) {
    fprintf(stderr, "n %d vs %d mismatch\n", n, mat_n);
    exit(1);
  }

  for ( i = 0; i < n2; i++ )
    imat[0][i] = 0;

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

  for ( i = 0; i < n2; i++ )
    XDBL_SET_SI(mat[0][i], imat[0][i]);

#if XDBL_TYPE == XDBL_TYPE_MPQ
  mat_det(det, mat, mat_n);
  /* assure that the det is an integer */
  {
    mpq_t x;
    mpz_t den;

    mpz_init(den);
    mpq_get_den(den, det);
    if ( mpz_cmp_si(den, 1) != 0 ) {
      fprintf(stderr, "denominator %ld\n", mpz_get_si(den));
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
  XDBL_ROUND(det, det); /* the determinant is an integer */
  sym = (xdouble) nauty_stats.grpsize1;
  for ( i = 0; i < nauty_stats.grpsize2; i++ )
    sym *= 10;
#endif

  if ( XDBL_SGN(det) <= 0 ) {
    printf("bad determinant %g\n", XDBL_GET_DBL(det));
  }
  count++;
  if ( fp != NULL ) {
    XDBL_FPRINTF(fp, "%d " XDBL_PRND " %.0f\n",
        nedges, det, XDBL_GET_DBL(sym));
  }
  /* vir = sum_G 2^(n-1) (1-n) (-)^E / sqrt(det)^dim / sym */
}



static int gcomplx_done(void)
{
  XDBL_CLEAR(det);
  XDBL_CLEAR(sym);
  if ( mat != NULL ) {
    if ( mat[0] != NULL ) {
      int i;
      for ( i = 0; i < mat_n * mat_n; i++)
        XDBL_CLEAR(mat[0][i]);
      free(mat[0]);
    }
    free(mat);
  }
  return 0;
}

