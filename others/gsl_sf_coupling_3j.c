#include "stdlib.h"
#include "stdio.h"
#include "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/fpdebug.h"
#include <gsl/gsl_sf.h>
int main(int argc, const char * argv[]) {
unsigned long int hexdouble;
int a;
a = atoi(argv[1]);
int b;
b = atoi(argv[2]);
int c;
c = atoi(argv[3]);
int d;
d = atoi(argv[4]);
int e;
e = atoi(argv[5]);
int f;
f = atoi(argv[6]);
double result = gsl_sf_coupling_3j(a, b, c, d, e, f);
//printf("%.15f\n", result);
VALGRIND_PRINT_VALUES("result", 1, &result);
return 0;
}