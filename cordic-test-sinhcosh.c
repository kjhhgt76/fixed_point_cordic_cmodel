#include <stdio.h>
#include "cordic_verilog.h"
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>

#define TEST_NUMBER 1

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
    double error1, error2;
    int i;
    int seed = time(NULL); 
    srand(seed);
    error_stats error_stat_cosh[9] = {0};
    error_stats error_stat_sinh[9] = {0};
    for (i = 0; i < 9; i++)
    {
        error_stat_cosh[i].min_error = 100;
        error_stat_sinh[i].min_error = 100;
    }
    for(i=0;i<TEST_NUMBER;i++)
    {
        int q131_arg1 = 0;
        arg1 = q131_to_float(q131_arg1);
        if (debug)
        {
            printf("RAND_MAX=%d\n", RAND_MAX);
            printf("arg1: %f\n", arg1);
        }
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1*MUL131;

            cordic(&targ1, &targ2, -1, 1, 1, 0);  // targ1 = res1 = cosh(x)/2 = cosh(arg1*2)/2, targ2 = res2 = sinh(x)/2 = sinh(arg1*2)/2
            error1 = targ1/MUL131-cosh(arg1*2)/2;  
            error2 = targ2/MUL131-sinh(arg1*2)/2;
            int inputs[2] = {q131_arg1, 1};
            update_error_stat(&error_stat_cosh[POINT_POS-21], error1, inputs, targ1);
            update_error_stat(&error_stat_sinh[POINT_POS-21], error2, inputs, targ2);

            if (fabs(error1) > 1e-5)
            {
                printf("arg1, arg2: %.20f : %.20f\n", arg1, arg2);
                printf("q1.31 input, arg1: %x\n", (int)(arg1*MUL131));
                printf("cosh %f/2 = %f : %f\n", arg1*2, targ1/MUL131, cosh(arg1*2)/2);
                printf("=%x : %x\n", targ1, (int)((cosh(arg1*2)/2)*MUL131));
                printf("error=%E\n", error1);
                printf("error(in terms of 2^N)=%d\n", log_for_any_base(error1, 2));
            }
            if (fabs(error2) > 1e-5)
            {
                printf("sinh %f/2 = %f : %f\n", arg1*2, targ2/MUL131, sinh(arg1*2)/2);
            }
        }
    }
    for (i = 0; i < 9; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_cosh:\n");
        print_error_information(&error_stat_cosh[i]);
        printf("error_stat_sinh:\n");
        print_error_information(&error_stat_sinh[i]);
    }
    printf("seed=%d\n", seed);
}
