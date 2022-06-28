// random input
#include <stdio.h>
#include <math.h>
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"

#define TEST_NUMBER 16

int float_to_q131(double src)
{
    return (int)(src*MUL131);
}

double q131_to_float(int src)
{
    return src/MUL131;
}

int main(int argc, char **argv)
{
    double arg1,arg2;
    int targ1,targ2;
    int n;
    int i; // q1.21
    int large_error_count[9] = {0};
    double error_atan;
    int seed = 1649398432;
    srand(seed);
    error_stats error_stat_atan[9] = {0, 0, 0, 0, 0, 0};
    for (i = 0; i < 9; i++)
    {
        error_stat_atan[i].min_error = 100;
    }
    int test_q131_arg1[TEST_NUMBER] = {0, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0xabcdef12, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f, 0x7654321f};
    int test_n[TEST_NUMBER] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};
    int output[TEST_NUMBER] = {0};
    for(i=0;i<TEST_NUMBER;i++)
    {

        int q131_arg1 = test_q131_arg1[i];
        n = test_n[i];
        double x = q131_to_float(q131_arg1)*pow(2,n);
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = q131_arg1;
            cordic(&targ1, &targ2, 1, 0, n, 0);
            error_atan = targ1/MUL131-pow(2, -n)*atan(x)/M_PI;
            int inputs[2] = {q131_arg1, n};
            if ((((unsigned)targ1>>31)&&(pow(2,-n)*atan(x)/M_PI)>=0)||!((unsigned)targ1>>31)&&(pow(2,-n)*atan(x)/M_PI)<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", pow(2,-n)*atan(x)/M_PI, targ1/MUL131);
                    printf("cordic result=%x\n", targ1);
                    printf("arg1=%x, arg2=%x\n", q131_arg1, n);
                    printf("x=%f\n", q131_to_float(q131_arg1));
                    return 0;
                }
            update_error_stat(&(error_stat_atan[POINT_POS-21]), error_atan, inputs, targ1);
            output[i] = targ1;
        }
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atan:\n");
        print_error_information(&error_stat_atan[i]);
        printf("large_error_count=%d\n", large_error_count[i]);
    }
    printf("-----FIRST FEW OUTPUT-----\n");
    for (i = 0; i < TEST_NUMBER && i < 20 ; i++)
    {
        printf("output[%d]=%x\n", i, output[i]);
    }
    printf("seed=%d\n", seed);
}
