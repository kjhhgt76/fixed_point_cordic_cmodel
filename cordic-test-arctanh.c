#include <stdio.h>
#include "cordic_verilog.h"
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>

#define TEST_NUMBER 10
//Print out sin(x) vs fp CORDIC sin(x)

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
    int i;
    double error_atanh;
    int seed = 1649824994; // time(0);
    srand(seed);
    error_stats error_stat_atanh[9] = {0};
    for (i = 0; i < 9; i++)
        error_stat_atanh[i].min_error = 100;
    int test_q131_arg1[TEST_NUMBER] = {0xcc6a7efa, 0x33958106, 0x23456789, 0x12345678, 0x2468acef, 0x0001357a, 0xfedcba98, 0xedcba987, 0xdcba9876, 0xfdb97531};
    int output[TEST_NUMBER] = {0};
    for(i=0;i<TEST_NUMBER;i++)
    {
        
        int q131_arg1 = test_q131_arg1[i];
        int n = 1;
        double x = q131_to_float(q131_arg1)*pow(2,n);
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = q131_arg1;
            cordic(&targ1, &targ2, -1, 0, n, 0);
            
            
            error_atanh = targ1/MUL131-atanh(x)/2;  // targ1 = res1 = atanh(x)/2 = atanh(arg1*2)/2
            int inputs[2] = {q131_arg1, n};
            update_error_stat(&error_stat_atanh[POINT_POS-21], error_atanh, inputs, targ1);
            output[i]=targ1;
        }
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atanh:\n");
        print_error_information(&error_stat_atanh[i]);
    }
    printf("-----FIRST FEW OUTPUT-----\n");
    for (i = 0; i < TEST_NUMBER && i < 20 ; i++)
    {
        printf("output[%d]=%x\n", i, output[i]);
    }
    printf("seed=%d\n", seed);

}
