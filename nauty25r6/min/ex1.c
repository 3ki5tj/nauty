#define MAXN 132
#define ONE_WORD_SETS /* try to use one word sets when possible */
#define WORDSIZE 32
/* #include "nau0.h" */
#include "nau0.h"


static void printgr(graph *g, int n, const char *name)
{
  int u, v;

  printf("%s:\n", name);
  for (u = 0; u < n; u++) {
    for (v = 0; v < n; v++)
      printf("%c ", ISELEMENT(g + u, v) ? '*' : ' ');
    printf("\n");
  }
  printf("\n");
}



int main(void)
{
  int n = 12, m, v, u;
  graph g[MAXN * MAXM], ng[MAXN * MAXM], ng1[MAXN * MAXM];
  int lab[MAXN], ptn[MAXN], orbits[MAXN];
  static DEFAULTOPTIONS_GRAPH(options);
  statsblk stats;

  options.getcanon = TRUE;
  m = SETWORDSNEEDED(n);
  printf("WORDSIZE %d, MAXN %d, n %d, m %d\n",
         (int) sizeof(setword), MAXN, n, m);

  nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

  EMPTYGRAPH(g, m, n);
  for (v = 0; v < n; v++)
    ADDONEEDGE(g, v, (v + 1) % n, m);
  ADDONEEDGE(g, 2, 5, m);
  printgr(g, n, "original");
  densenauty(g, lab, ptn, orbits, &options, &stats, m, n, ng);
  printf("Automorphism group size = ");
  writegroupsize(stdout, stats.grpsize1, stats.grpsize2);
  printf("\n");

  printf("New label:\n");
  for (v = 0; v < n; v++)
    printf("%2d %2d\n", v, lab[v]);

  EMPTYGRAPH(ng1, m, n);
  for (u = 0; u < n; u++)
    for (v = u + 1; v < n; v++) {
      if (ISELEMENT(g + lab[u], lab[v]))
        ADDONEEDGE(ng1, u, v, m);
    }
  printgr(ng1, n, "from lab:");
  printgr(ng, n, "output");
  return 0;
}



