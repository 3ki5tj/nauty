int mp_prec = 256;
int max_dim = 100; /* maximal dimension */

#define PLUGIN_INIT gaussf_init(maxn);
#define PLUGIN_DONE gaussf_done();
#define OUTPROC gaussf_vir
#define PLUGIN_SWITCHES \
  else if ( sw == 'p' ) { arg_int(&arg, &mp_prec, "-p"); } \
  else if ( sw == 'Z' ) { arg_int(&arg, &max_dim, "-Z"); }

#include "gcomplx.h"



#define HP_TYPE_DBL  0
#define HP_TYPE_LDBL 1
#define HP_TYPE_F128 2
#define HP_TYPE_QUAD 2
#define HP_TYPE_MPF  10


/* define high precision type */
#if defined(HP_MP) || defined(HP_MPF) || defined (HP_GMP)

#include <gmp.h>
typedef mpf_t hp_t;
#define HP_TYPE             HP_TYPE_MPF
#define HP_INIT(x)          mpf_init_set_si(x, 0)
#define HP_CLEAR(x)         mpf_clear(x)
#define HP_SET_SI(x, d)     mpf_set_si(x, d)
#define HP_ADD(c, a, b)     mpf_add(c, a, b)
#define HP_SUB(c, a, b)     mpf_sub(c, a, b)
#define HP_MUL(c, a, b)     mpf_mul(c, a, b)
#define HP_DIV(c, a, b)     mpf_div(c, a, b)
#define HP_ADDX(y, x)       mpf_add(y, y, x)
#define HP_SUBX(y, x)       mpf_sub(y, y, x)
#define HP_MULX(y, x)       mpf_mul(y, y, x)
#define HP_DIVX(y, x)       mpf_div(y, y, x)
#define HP_INV(y, x)        mpf_ui_div(y, 1, x)
#define HP_GET_DBL(x)       mpf_get_d(x)
#define HP_SQRT(y, x)       mpf_sqrt(y, x)
#define HP_FPRINTF          gmp_fprintf
#define HP_PRN              "F"
#define HP_PRNE             "%+.76Fe"
#define HP_STRPREC          "mpf"

#else /* native data types */

#if defined(HP_QUAD) || defined(HP_F128)

  #include <quadmath.h>
  typedef __float128 hp_t;
  #define HP_TYPE           HP_TYPE_QUAD
  #define HP_SQRT(y, x)     (y) = sqrtq(x)
  #define HP_PRN            "Q"
  #define HP_PRNE           "%+.32Qe"
  #define HP_STRPREC        "f128"

#elif defined(HP_LDBL) || defined(HP_LONG)

  typedef long double hp_t;
  #define HP_TYPE           HP_TYPE_LDBL
  #define HP_SQRT(y, x)     (y) = sqrtl(x)
  #define HP_PRN            "L"
  #define HP_PRNE           "%+.18Le"
  #define HP_STRPREC        "ldbl"

#else

  typedef double hp_t;
  #define HP_TYPE           HP_TYPE_DBL
  #define HP_SQRT(y, x)     (y) = sqrt(x)
  #define HP_PRN            ""
  #define HP_PRNE           "%+.14e"
  #define HP_STRPREC        ""

#endif

#define HP_INIT(x)          x = 0
#define HP_CLEAR(x)
#define HP_SET_SI(x, d)     (x) = (hp_t) (d)
#define HP_ADD(c, a, b)     (c) = ((a) + (b))
#define HP_SUB(c, a, b)     (c) = ((a) - (b))
#define HP_MUL(c, a, b)     (c) = ((a) * (b))
#define HP_DIV(c, a, b)     (c) = ((a) / (b))
#define HP_ADDX(y, x)       (y) += (x)
#define HP_SUBX(y, x)       (y) -= (x)
#define HP_MULX(y, x)       (y) *= (x)
#define HP_DIVX(y, x)       (y) /= (x)
#define HP_INV(y, x)        (y) = (1/(x))
#define HP_FPRINTF          fprintf
#define HP_GET_DBL(x)       (double) (x)

#endif



hp_t *virsum;



static int gaussf_init(int n)
{
  int i;

  printf("n %d, D <= %d, prec %d\n", n, max_dim, mp_prec);
#if HP_TYPE == HP_TYPE_MPF
  mpf_set_default_prec(mp_prec);
#endif
  xnew(virsum, max_dim + 1);
  for ( i = 0; i <= max_dim; i++ )
    HP_INIT(virsum[i]);
  return gcomplx_init(n);
}



#if HP_TYPE == HP_TYPE_MPF

static void HP_SET_XDBL(hp_t y, xdouble x)
{
  #if XDBL_TYPE == XDBL_TYPE_MPQ
    mpf_set_q(y, x);
  #else
    char s[80];
    XDBL_SNPRINTF(s, sizeof s, "%.40" XDBL_PRN "e", x);
    mpf_set_str(y, s, 10);
  #endif
}

#else

#define HP_SET_XDBL(y, x) (y) = (hp_t) (x)

#endif



static void gaussf_vir(FILE *fp, graph *g, int n)
{
  int d;
  hp_t invsqrtdet, symf, x;

  gcomplx_dograph(NULL, g, n);
  HP_INIT(x);
  HP_INIT(symf);
  HP_INIT(invsqrtdet);
  HP_SET_XDBL(symf, sym);
  HP_SET_XDBL(x, det);
  HP_SQRT(x, x);
  HP_INV(invsqrtdet, x);
  HP_SET_SI(x, (1L << mat_n) * (-mat_n) * (nedges % 2 ? -1 : 1));
  HP_DIVX(x, symf);
  for ( d = 1; d <= max_dim; d++ ) {
    HP_MULX(x, invsqrtdet);
    HP_ADDX(virsum[d], x);
  }
  if (verbose) {
    fprintf(fp, "ned %d, det %.0f, SI %.0f, virsum %g (3D)\n",
      nedges, XDBL_GET_DBL(det), XDBL_GET_DBL(sym), HP_GET_DBL(virsum[3]));
  }
  HP_CLEAR(invsqrtdet);
  HP_CLEAR(symf);
  HP_CLEAR(x);
}



static int gaussf_save(void)
{
  FILE *fp;
  char fn[128], sdim[32] = "", sed[32] = "";
  int d, n = mat_n + 1, hp_prec;

  /* for three dimensions the data are
   * n     Bn / B2^(n-1)
   * 3     2.56600119639834e-01
   * 4    -1.25459957055045e-01
   * 5     1.33256551732054e-02
   * 6     3.84609358308697e-02
   * 7    -3.30834429031497e-02
   * 8     4.18241876968238e-03
   * 9     1.51976071955007e-02
   * 10   -1.38496541345590e-02 (54s on T60)
   * */
  for ( d = 1; d <= max_dim && d <= 10; d++ )
    printf("%4d: %18.14f\n", d, HP_GET_DBL(virsum[d]));

  if ( max_dim != 100 )
    sprintf(sdim, "D%d", max_dim);
  if ( mine != n || maxe != n * (n - 1) / 2 )
    sprintf(sed, "e%dE%d", mine, maxe);
#if HP_TYPE == HP_TYPE_MPF
  hp_prec = mp_prec;
#elif HP_TYPE == HP_TYPE_QUAD
  hp_prec = 113;
#elif HP_TYPE == HP_TYPE_LDBL
  hp_prec = 64;
#else
  hp_prec = 53;
#endif
  sprintf(fn, "gaussfn%d%s%s%s%s.dat",
      n, sdim, sed, XDBL_STRPREC, HP_STRPREC);
  if ( (fp = fopen(fn, "w")) == NULL ) {
    fprintf(stderr, "cannot write %s\n", fn);
    return -1;
  }
  fprintf(fp, "# D %d B%d %s %s %s %d\n",
      max_dim, mat_n + 1, sed, XDBL_STRPREC, HP_STRPREC, hp_prec);
  for ( d = 1; d <= max_dim; d++ ) {
    HP_FPRINTF(fp, "%4d " HP_PRNE "\n", d, virsum[d]);
  }
  fclose(fp);
  fprintf(stderr, "results saved to %s\n", fn);
  return 0;
}



static int gaussf_done(void)
{
  gaussf_save();
  gcomplx_done();
  if (virsum != NULL) free(virsum);
  return 0;
}

