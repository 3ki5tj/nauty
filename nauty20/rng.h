/* rng.h : definitions for using Don Knuth's random number generator.

   To use it:
     1.  Call ran_init(seed) with any long seed.  (Optional,
	   but you will always get the same sequence otherwise.)
     2.  Use NEXTRAN to get the next number (0..2^30-1).
         Alternatively, use KRAN(k) to get a random number 0..k-1.
	 For large k, KRAN(k) is not quite uniform.  In that case
         use GETKRAN(k,var) to set the variable var to a better
         random number 0..k-1.

   Some of these definitions are also in naututil.h.
*/

extern long *ran_arr_ptr;
long ran_arr_cycle();
void ran_init(long seed);
void ran_array(long *aa, int n);

#define MAXRAN (0x3FFFFFFFL)    /* Values are 0..MAXRAN-1 */
#define NEXTRAN (*ran_arr_ptr>=0 ? *ran_arr_ptr++ : ran_arr_cycle())
#define KRAN(k) (NEXTRAN%(k))

#define MAXSAFE(k) ((MAXRAN/(k))*(k))
#define GETKRAN(k,var) {long __getkran; \
    do {__getkran = NEXTRAN;} while (__getkran >= MAXSAFE(k)); \
    var = __getkran % (k);}
