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
double c;
sscanf(argv[3], "%lX", &hexdouble);
c = *(double*)(&hexdouble);
double d;
sscanf(argv[4], "%lX", &hexdouble);
d = *(double*)(&hexdouble);
double result = gsl_sf_hydrogenicR(a, b, c, d);
//printf("%.15f\n", result);
VALGRIND_PRINT_VALUES("result", 1, &result);
return 0;
}