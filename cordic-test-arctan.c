#include <stdio.h>
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"

#define TEST_NUMBER 16
//Print out sin(x) vs fp CORDIC sin(x)
int main(int argc, char **argv)
{
    double arg1,arg2;
    int targ1,targ2;
    int i;
    double error_atan, error_length;
    srand(time(0));
    error_stats error_stat_atan = {0, 0, 0, 0, 0, 0};
    error_stats error_stat_length = {0, 0, 0, 0, 0, 0};
    int test_q131_arg1[TEST_NUMBER] = {0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f};
    int test_n[TEST_NUMBER] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
    int output[TEST_NUMBER] = {0};
    POINT_POS=23;
    BIT=26;
    for(i=0;i<TEST_NUMBER;i++)
    {
        int q131_arg1 = test_q131_arg1[i];
        int n = 1;
        double x = q131_to_float(q131_arg1)*pow(2,n);

        cordic(&targ1, &targ2, 1, 0, n, 0);

        error_atan = targ1/MUL131*M_PI-atan(x);
        error_length = targ2/MUL131-(sqrt(arg1*arg1+arg2*arg2) >= 1.0 ? 1.0 : sqrt(arg1*arg1+arg2*arg2));
        inputs
        update_error_stat(&error_stat_atan, error_atan);
        update_error_stat(&error_stat_length, error_length);
        output[i] = targ1;
    }
    printf("----------------------------------\n");
    printf("%d bit  %d iteration    q%d.%d:\n", BIT, ITERATION, BIT-POINT_POS, POINT_POS);
    printf("error_stat_atan:\n");
    print_error_information(&error_stat_atan);
    printf("error_stat_length:\n");
    print_error_information(&error_stat_length);
}
