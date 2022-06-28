#include <stdio.h>
#include <math.h>
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"

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
    int i; // q1.21
    int large_error_count[9] = {0};
    double error_atan2, error_modulus;
    int seed = 1649239132;
    srand(seed);
    error_stats error_stat_atan2[9] = {0, 0, 0, 0, 0, 0};
    error_stats error_stat_modulus[9] = {0, 0, 0, 0, 0, 0};
    for (i = 0; i < 9; i++)
    {
        error_stat_atan2[i].min_error = 100;
        error_stat_modulus[i].min_error = 100;
    }
    int q131_arg1 = 0; 
    int q131_arg2 = 0;
    MUL=1<<23;
    for (; q131_arg2 < 0x010000; q131_arg2+=(1<<8))  // -256<arg2<256
    {
        for (q131_arg1 = 0x80000000; i<2048; i++)
        {
            q131_arg1+=(i*(1<<8));
            arg1 = q131_to_float(q131_arg1);
            arg2 = q131_to_float(q131_arg2);
            
            cordic(&targ1, &targ2, 1, 0, 0, 1);

            if ((unsigned)targ1>>31)
            {
                error_atan2=0;
            }
            else
            {
                error_atan2 = targ1/MUL131-atan2(arg2, arg1)/M_PI;  
            }
            
            
            error_atan2 = targ1/MUL131-atan2(arg2, arg1)/M_PI;
            error_modulus = targ2/MUL131-(sqrt(arg1*arg1+arg2*arg2) >= 1.0 ? 1.0 : sqrt(arg1*arg1+arg2*arg2));
            int inputs[2] = {float_to_q131(arg1), float_to_q131(arg2)};
            update_error_stat(&(error_stat_atan2[2]), error_atan2, inputs, targ1);
            update_error_stat(&(error_stat_modulus[2]), error_modulus, inputs, targ2);
            if (debug)
            {
                printf("expected result=%.20f\n", atan2(arg2, arg1)/M_PI);
                printf("actual result=%.20f\n", targ1/MUL131);
                printf("arg1=%x:%.20f, arg2=%x:%.20f\n", float_to_q131(arg1), arg1, float_to_q131(arg2), arg2);
            }
            
            arg2 = q131_to_float(~q131_arg2+1);
            cordic(&targ1, &targ2, 1, 0, 0, 1);
            if (!((unsigned)targ1>>31))
            {
                error_atan2=0;
            }
            else
            {
                error_atan2 = targ1/MUL131-atan2(arg2, arg1)/M_PI;  
            }
            error_modulus = targ2/MUL131-(sqrt(arg1*arg1+arg2*arg2) >= 1.0 ? 1.0 : sqrt(arg1*arg1+arg2*arg2));
            int inputs2[2] = {float_to_q131(arg1), float_to_q131(arg2)};
            update_error_stat(&(error_stat_atan2[2]), error_atan2, inputs2, targ1);
            update_error_stat(&(error_stat_modulus[2]), error_modulus, inputs2, targ2);
        }
    }
    
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atan:\n");
        print_error_information(&error_stat_atan2[i]);
        printf("large_error_count=%d\n", large_error_count[i]);
        printf("error_stat_length:\n");
        print_error_information(&error_stat_modulus[i]);
    }
    printf("seed=%d\n", seed);
    printf("number of overflow = %d\n", overflow_count);
}
