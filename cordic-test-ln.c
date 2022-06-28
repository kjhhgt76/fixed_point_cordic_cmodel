#include <stdio.h>
#include "cordic_verilog.h"
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>

#define TEST_NUMBER 12
#define SHOW_BAD_INPUT 1

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
    int targ1,targ2=0;
    int i;
    double error_ln;
    int seed = 1654152741; // time(0);
    srand(seed);
    error_stats error_stat_ln[9] = {0};
    for (i = 0; i < 9; i++)
        error_stat_ln[i].min_error = 100;
    int test_q131_arg1[TEST_NUMBER] = {0x6d91687, 0x3fffffff, 0x20000000, 0x5fffffff, 0x30000000, 0x6fffffff, 0x38000000, 0x4ac08311, 0x12345678, 0x3456789a, 0x456789ab, 0x456789ab};
    int test_n[TEST_NUMBER] = {1, 1, 2, 2, 3, 3, 4, 4, 1, 2, 3, 4};
    int output[TEST_NUMBER] = {0};
    for(i=0;i<TEST_NUMBER;i++)
    {
        int q131_arg1 = test_q131_arg1[i];
        int n = test_n[i];
        double x = q131_to_float(q131_arg1)*pow(2,n);

        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            
            MUL=1<<POINT_POS;
            targ1 = q131_arg1;
            cordic(&targ1, &targ2, -1, 0, n, 2);

            error_ln = (targ1/MUL131)-log(x)/(pow(2, n+1)); // res1 = targ1 = ln(x) / (2^(n+1))
            int inputs[2] = {q131_arg1, n};
            update_error_stat(&error_stat_ln[POINT_POS-21], error_ln, inputs, targ1);
            output[i]=targ1;
        }
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_ln:\n");
        print_error_information(&error_stat_ln[i]);
    }
    printf("-----FIRST FEW OUTPUT-----\n");
    for (i = 0; i < TEST_NUMBER && i < 20 ; i++)
    {
        printf("output[%d]=%x\n", i, output[i]);
    }
    printf("seed=%d\n", seed);
}
