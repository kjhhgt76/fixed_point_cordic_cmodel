// all possible input [0, 2^32-1] q3.23
#include <stdio.h>
#include "cordic_verilog.h"
#include <math.h> // for testing only!
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>

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
    // variable setup
    double arg1,arg2;
    int targ1,targ2;
    double error1, error2;
    int cosh_sign_error=0, sinh_sign_error=0;
    int i;
    int seed = 1649407564; 
    srand(seed);
    error_stats error_stat_cosh[9] = {0};
    error_stats error_stat_sinh[9] = {0};
    for (i = 0; i < 9; i++)
    {
        error_stat_cosh[i].min_error = 100;
        error_stat_sinh[i].min_error = 100;
    }
    
    // positive arg1
    for(i=0;i<=0x478d4fdf;i+=(1<<6)) // 0x478d4fdf=q1.31 0.559
    {
        arg1 = q131_to_float(i); // arg1=x/2, x= arg1*2, range=[-0.559, 0.559]
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1*MUL131;

            cordic(&targ1, &targ2, -1, 1, 1, 0);  // targ1 = res1 = cosh(x)/2 = cosh(arg1*2)/2, targ2 = res2 = sinh(x)/2 = sinh(arg1*2)/2
            int j;
            for (j=0; (i+j)<=0x478d4fdf&&j<(1<<6); j++) // change the last 6 bits and input to the floating point algorithm.
            {
                int inputs[2] ={i, 1};
                arg1=q131_to_float(i+j);
                error1 = targ1/MUL131-cosh(arg1*2)/2;  
                error2 = targ2/MUL131-sinh(arg1*2)/2;
                inputs[0] = i+j;
                inputs[1] = 1;
                if ((((unsigned)targ1>>31)&&cosh(arg1*2)/2>=0)||!((unsigned)targ1>>31)&&cosh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", cosh(arg1*2)/2, targ1/MUL131);
                    printf("cordic result=%x\n", cosh(arg1*2)/2, targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    cosh_sign_error++;
                }
                if ((((unsigned)targ2>>31)&&sinh(arg1*2)/2>=0)||!((unsigned)targ2>>31)&&sinh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", sinh(arg1*2)/2, targ2/MUL131);
                    printf("cordic result=%x\n", sinh(arg1*2)/2, targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    sinh_sign_error++;
                }
                update_error_stat(&error_stat_cosh[POINT_POS-21], error1, inputs, targ1);
                update_error_stat(&error_stat_sinh[POINT_POS-21], error2, inputs, targ2);
            }
        }
        printf("\rtested case=%.20f", q131_to_float(i));
    }
    // negative arg1
    for (i=0xb872b000; i<0; i+=(1<<6)) // 0xb872b000 = q1.31 -0.559
    {
        arg1 = q131_to_float(i); // arg1=x/2, x= arg1*2, range=[-0.559, 0.559]
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1*MUL131;

            cordic(&targ1, &targ2, -1, 1, 1, 0);  // targ1 = res1 = cosh(x)/2 = cosh(arg1*2)/2, targ2 = res2 = sinh(x)/2 = sinh(arg1*2)/2
            int j;
            for (j=0x03f; (i+j)>=0xb872b021&&j>=0; j--)  // change the last 6 bits and input to the floating point algorithm.
            {
                int inputs[2] ={i, 1};
                arg1=q131_to_float(i+j);
                error1 = targ1/MUL131-cosh(arg1*2)/2;  
                error2 = targ2/MUL131-sinh(arg1*2)/2;
                inputs[0] = i+j;
                inputs[1] = 1;
                if ((((unsigned)targ1>>31)&&cosh(arg1*2)/2>=0)||!((unsigned)targ1>>31)&&cosh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", cosh(arg1*2)/2, targ1/MUL131);
                    printf("cordic result=%x\n", cosh(arg1*2)/2, targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    cosh_sign_error++;
                }
                if ((((unsigned)targ2>>31)&&sinh(arg1*2)/2>=0)||!((unsigned)targ2>>31)&&sinh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", sinh(arg1*2)/2, targ2/MUL131);
                    printf("cordic result=%x\n", sinh(arg1*2)/2, targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    sinh_sign_error++;
                }
                update_error_stat(&error_stat_cosh[POINT_POS-21], error1, inputs, targ1);
                update_error_stat(&error_stat_sinh[POINT_POS-21], error2, inputs, targ2);
            }
        }
        printf("\rtested case=%.20f", q131_to_float(i));
    }
    printf("\nStore the data into: ./error_analysis/sinhcosh_error_analysis.txt\n");
    if (!freopen("./error_analysis/sinhcosh_error_analysis.txt", "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_cosh:\n");
        printf("cosh_sign_error=%d\n", cosh_sign_error);
        print_error_information(&error_stat_cosh[i]);
        printf("error_stat_sinh:\n");
        printf("sinh_sign_error=%d\n", sinh_sign_error);
        print_error_information(&error_stat_sinh[i]);
    }
    printf("seed=%d\n", seed);
}
