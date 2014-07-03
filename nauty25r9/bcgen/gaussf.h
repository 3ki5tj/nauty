#define PLUGIN_INIT gaussf_init(maxn);
#define PLUGIN_DONE gaussf_done();
#define OUTPROC gaussf_vir



#include "gcomplx.h"



int dim = 100; /* maximal dimension */
xdouble *virsum;



static int gaussf_init(int n)
{
  int i;

  xnew(virsum, dim + 1);
  for ( i = 0; i <= dim; i++ ) {
    INIT(virsum[i]);
  }
  return gcomplx_init(n);
}



static void gaussf_vir(FILE *fp, graph *g, int n)
{
  int d, ned = 0;
  xdouble sqrtdet, px;

  gcomplx_dograph(NULL, g, n);
  SQRT(sqrtdet, det);
  px = ((1 << mat_n) * (-mat_n) * (ned % 2 ? -1 : 1)) / sqrtdet;
  for ( d = 1; d <= dim; d++, px /= sqrtdet )
    virsum[d] += px/sym;
  if (verbose) {
    fprintf(fp, "ned %d, det %g, SI %g, virsum %g (3D)\n",
      ned, GETDBL(det), GETDBL(sym), (double) virsum[3]);
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

  sprintf(fn, "gaussfn%d%s.dat", mat_n + 1, STRPREC);
  if ( (fp = fopen(fn, "w")) == NULL ) {
    fprintf(stderr, "cannot write %s\n", fn);
    return -1;
  }
  fprintf(fp, "# D B%d\n", mat_n + 1);
  for ( d = 1; d <= dim; d++ ) {
    fprintf(fp, "%4d " PRNREALE "\n", d, virsum[d]);
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

