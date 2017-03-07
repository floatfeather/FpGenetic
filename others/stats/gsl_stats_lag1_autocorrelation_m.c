#include "stdlib.h"
#include "stdio.h"
#include "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/fpdebug.h"
#include <gsl/gsl_statistics.h>
int main(int argc, const char * argv[]) {
unsigned long int hexdouble;
int a;
a = atoi(argv[1]);
int argv_cnt = 2;
double b[5];
for(int i = 0; i < a; i++) {
sscanf(argv[argv_cnt++], "%lX", &hexdouble);
b[i] = *(double*)(&hexdouble);
}
unsigned long c = 1;
double e;
sscanf(argv[argv_cnt++], "%lX", &hexdouble);
e = *(double*)(&hexdouble);
double result = gsl_stats_lag1_autocorrelation_m(b, c, a, e);
//printf("%.15f\n", result);
VALGRIND_PRINT_VALUES("result", 1, &result);
return 0;
}