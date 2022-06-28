#include <stdio.h>
#include "cordic_verilog.h"
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>

#define TEST_NUMBER 1
#define SHOW_BAD_INPUT 1

int float_to_q131(double src)
{
    return (int)(src*MUL131);
}

double q131_to_float(int src)
{
    return src/MUL131;
}

inline int q131_pos_to_neg(int src)
{
    return ~src+1;
}

int main(int argc, char **argv)
{
    double arg1,arg2;
    int targ1,targ2=0;
    int i;
    double error_sqrt;
    int seed = time(0); // generate random number seed.
    srand(seed); 
    error_stats error_stat_sqrt[9] = {0};
    for (i=0; i < 9; i++)
        error_stat_sqrt[i].min_error = 100;
    for(i=0;i<TEST_NUMBER;i++)
    {
        int q131_arg1 = 0x4587e7c0;
        int n=1;
        double x = q131_to_float(q131_arg1)* pow(2, n);
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = q131_arg1;
            cordic(&targ1, &targ2, -1, 0, n, 1);  // res1 = targ1 = sqrt(x)/(2^n)

            error_sqrt = (targ1/MUL131)-sqrt(x)/(pow(2, n));
            int inputs[2] = {q131_arg1, n};
            update_error_stat(&error_stat_sqrt[POINT_POS-21], error_sqrt, inputs, targ1);

            if (SHOW_BAD_INPUT && fabs(error_sqrt) > 1e-5)
            {
                printf("------Big error detected.------\n");
                printf("format:q3.%d\n", POINT_POS);
                printf("x=%.20f, arg1= %.20f, n= %d\n", x, arg1, n);
                printf("q1.31 input, arg1: %x\n", (int)(arg1*MUL131));
                printf("sqrt(%f) / (2^(n+1)) = %f : %f\n", x, targ1/MUL131, sqrt(x)/(pow(2, n)));
                printf("=%x : %x\n", targ1, (int)((sqrt(x)/(pow(2, n)))*MUL131));
                printf("error=%E\n", error_sqrt);
                printf("error(in terms of 2^N)=%d\n", log_for_any_base(error_sqrt, 2));
            }
        }
    }
    for (i = 0; i < 9; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_sqrt:\n");
        print_error_information(&error_stat_sqrt[i]);
    }
    printf("seed=%d\n", seed);
}
