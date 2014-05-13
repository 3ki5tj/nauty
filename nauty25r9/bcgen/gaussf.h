#define PLUGIN_INIT gaussf_init(maxn);
#define PLUGIN_DONE gaussf_done();
#define OUTPROC gaussf_vir

#include <math.h>

#ifdef QUAD
#include <quadmath.h>
typedef __float128 xdouble;
__inline xdouble FABS(xdouble x) { return (x < 0) ? -x : x; }
#define SQRT(x) sqrtq(x)
#define PRNREAL "Q"
#define PRNREALE "%36.29Qe"
#else
typedef double xdouble;
#define FABS(x) fabs(x)
#define SQRT(x) sqrt(x)
#define PRNREAL ""
#define PRNREALE "%22.14e"
#endif

int dim = 100; /* dimension */
xdouble *virsum;
int mat_n;
xdouble **mat;


#ifndef xnew
#define xnew(x, n) \
  if (((x) = calloc(n, sizeof(*(x)))) == NULL) { \
    fprintf(stderr, "no memory for %s\n", #x); \
    exit(1); \
  }
#endif



static int gaussf_init(int n)
{
  int i;

  mat_n = n - 1;
  xnew(mat, mat_n);
  xnew(mat[0], mat_n * mat_n);
  for ( i = 1; i < mat_n; i++ )
    mat[i] = mat[0] + i * mat_n;
  xnew(virsum, dim + 1);
  return 0;
}


static xdouble mat_det(xdouble **a, int n)
{
  int i, j, k, im, sgn = 1;
  xdouble am, x, det = 1;

  for ( k = 0; k < n; k++ ) {
    /* choose a pivot */
    im = k;
    am = (a[im][k] > 0) ? a[im][k] : -a[im][k];
    for ( i = k + 1; i < n; i++ ) {
      x = (a[i][k] > 0) ? a[i][k] : -a[i][k];
      if ( x > am ) {
        am = x;
        im = i;
      }
    }
    /* swap row im and k */
    if ( im != k ) {
      for ( j = k; j < n; j++ ) {
        x = a[im][j], a[im][j] = a[k][j], a[k][j] = x;
      }
      if ( (im - k) % 2 ) sgn *= -1;
    }
    /* normalize row k */
    am = a[k][k];
    if ( FABS(am) < 1e-10 ) {
      fprintf(stderr, "zero encounted! %d, am %g\n", k, (double) am);
      return 0;
    }
    x = 1/am;
    det *= am;
    for ( j = k; j < n; j++ )
      a[k][j] *= x;
    /* eliminate the rest row */
    for ( i = k + 1; i < n; i++ ) {
      x = a[i][k];
      for ( j = k + 1; j < n; j++ ) {
        a[i][j] -= x*a[k][j];
      }
    }
  }
  return det * sgn;
}



static void gaussf_vir(FILE *fp, graph *g, int n)
{
  int i, j, d, ned = 0;
  xdouble det, sym, x, px;

  if ( n != mat_n + 1 ) {
    fprintf(stderr, "n %d vs %d mismatch\n", n, mat_n);
    exit(1);
  }

  for ( i = 0; i < mat_n; i++ )
    for ( j = 0; j < mat_n; j++ )
      mat[i][j] = 0;

  for ( i = 0; i < n; i++ ) {
    graph *gi = GRAPHROW(g, i, NAUTY_M_);
    for ( j = i + 1; j < n; j++ ) {
      if ( ISELEMENT(gi, j) ) {
        if ( i == 0 ) {
          mat[j-1][j-1] += 1;
        } else {
          mat[i-1][i-1] += 1;
          mat[j-1][j-1] += 1;
          mat[i-1][j-1] += mat[j-1][i-1] = -1;
        }
        ned++;
      }
    }
  }

  if (verbose) {
    for ( i = 0; i < mat_n; i++ ) {
      for ( j = 0; j < mat_n; j++ )
        fprintf(fp, "%8.3f\t", (double) mat[i][j]);
      fprintf(fp, "\n");
    }
  }
  det = mat_det(mat, mat_n);
  sym = (xdouble) nauty_stats.grpsize1;
  for ( i = 0; i < nauty_stats.grpsize2; i++ )
    sym *= 10;
  x = 1/SQRT(det);
  px = x * ((1 << mat_n) * (-mat_n) * (ned % 2 ? -1 : 1));
  for ( d = 1; d <= dim; d++, px *= x )
    virsum[d] += px/sym;
  if (verbose) {
    fprintf(fp, "ned %d, det %g, SI %g, virsum %g (3D)\n",
      ned, (double) det, (double) sym, (double) virsum[3]);
  }
}



static int gaussf_save(void)
{
  FILE *fp;
  char fn[128];
  int d;

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
  for ( d = 1; d <= dim && dim <= 10; d++ )
    printf("%4d: %18.14f\n", d, (double) virsum[d]);

  sprintf(fn, "gaussfn%d.dat", mat_n + 1);
  if ( (fp = fopen(fn, "w")) == NULL ) {
    fprintf(stderr, "cannot write %s\n", fn);
    return -1;
  }
  for ( d = 1; d <= dim; d++ ) {
    fprintf(fp, "%4d: " PRNREALE "\n", d, virsum[d]);
  }
  fclose(fp);
  fprintf(stderr, "results saved to %s\n", fn);
  return 0;
}



static int gaussf_done(void)
{
  gaussf_save();
  if (mat[0] != NULL) free(mat[0]);
  if (mat != NULL) free(mat);
  if (virsum != NULL) free(virsum);
  return 0;
}

