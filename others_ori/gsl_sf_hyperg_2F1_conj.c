#include "stdlib.h"
#include "stdio.h"
#include "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/fpdebug.h"
#include <gsl/gsl_sf.h>
int main(int argc, const char * argv[]) {
unsigned long int hexdouble;
double a;
sscanf(argv[1], "%lX", &hexdouble);
a = *(double*)(&hexdouble);
double b;
sscanf(argv[2], "%lX", &hexdouble);
b = *(double*)(&hexdouble);
double c;
sscanf(argv[3], "%lX", &hexdouble);
c = *(double*)(&hexdouble);
double d;
sscanf(argv[4], "%lX", &hexdouble);
d = *(double*)(&hexdouble);
double result = gsl_sf_hyperg_2F1_conj(a, b, c, d);
//printf("%.15f\n", result);
VALGRIND_PRINT_ERROR("result", &result);
return 0;
}