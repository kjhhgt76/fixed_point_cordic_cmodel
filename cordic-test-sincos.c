// many random input
#include <stdio.h>
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"

#define MAX_CASE_NUMBER 1

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
    int arg1,arg2;
    int targ1,targ2;
    double error1, error2;
    long i;
    int seed = time(NULL);
    srand(seed);
    error_stats error_stat_sin[9] = {0};
    error_stats error_stat_cos[9] = {0};
    for (i = 0; i < 9; i++)
    {
        error_stat_sin[i].min_error = 100;
        error_stat_cos[i].min_error = 100;
    }
    time_t start_time = time(NULL);
    time_t target_seconds = 60;
    time_t time_passed = 0;
    for(i=0;i<MAX_CASE_NUMBER && time_passed<target_seconds;i++)
    {
        arg1 = 0x64db598c; // q1.31 arg1, range = [-1, 1]
        arg2 = 0x5; // q1.31 arg2, range = [0, 1]
        double x = q131_to_float(arg1);
        double y = q131_to_float(arg2);
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1;
            targ2 = arg2;
            cordic(&targ1, &targ2, 1, 1, 0, 0);

            error1 = targ1/MUL131-y*cos(x*M_PI);
            error2 = targ2/MUL131-y*sin(x*M_PI);
            int inputs[2] = {arg1, arg2};
            update_error_stat(&error_stat_cos[POINT_POS-21], error1, inputs, targ1);
            update_error_stat(&error_stat_sin[POINT_POS-21], error2, inputs, targ2);
        }
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_cos:\n");
        print_error_information(&error_stat_cos[i]);
        printf("error_stat_sin:\n");
        print_error_information(&error_stat_sin[i]);
    }
    printf("seed=%d\n", seed);
}
