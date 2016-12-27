#include "stdlib.h"
#include "stdio.h"
#include "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/fpdebug.h"
#include <gsl/gsl_sf.h>
int main(int argc, const char * argv[]) {
unsigned long int hexdouble;
int a;
a = atoi(argv[1]);
double b;
sscanf(argv[2], "%lX", &hexdouble);
b = *(double*)(&hexdouble);
double c;
sscanf(argv[3], "%lX", &hexdouble);
c = *(double*)(&hexdouble);
double result = gsl_sf_laguerre_n(a, b, c);
//printf("%.15f\n", result);
VALGRIND_PRINT_ERROR("result", &result);
return 0;
}