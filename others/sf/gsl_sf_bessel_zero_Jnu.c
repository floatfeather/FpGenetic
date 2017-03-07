#include "stdlib.h"
#include "stdio.h"
#include "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/fpdebug.h"
#include <gsl/gsl_sf.h>
int main(int argc, const char * argv[]) {
unsigned long int hexdouble;
double a;
sscanf(argv[1], "%lX", &hexdouble);
a = *(double*)(&hexdouble);
unsigned int b;
b = atoi(argv[2]);
double result = gsl_sf_bessel_zero_Jnu(a, b);
//printf("%.15f\n", result);
VALGRIND_PRINT_VALUES("result", 1, &result);
return 0;
}