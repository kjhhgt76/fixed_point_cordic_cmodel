// all possible inputs
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

inline int q131_pos_to_neg(int src)
{
    return ~src+1;
}

int main(int argc, char **argv)
{
    double arg1,arg2;
    int targ1,targ2=0;
    int i;
    double error_ln;
    int ln_sign_error = 0;
    int seed = 1654152741; // time(0);
    srand(seed);
    error_stats error_stat_ln[9] = {0};
    for (i = 0; i < 9; i++)
        error_stat_ln[i].min_error = 100;

    int arg1_range_lower_bound[4] = {0.0535*MUL131, 0.25*MUL131, 0.375*MUL131, 0.4375*MUL131};
    int arg1_range_upper_bound[4] = {0.5*MUL131, 0.75*MUL131, 0.875*MUL131, 0.584*MUL131};
    int n;
    for(n=1; n<=4; n++)
    {
        for (i=arg1_range_lower_bound[n-1]&(0xffffffc0); i<(arg1_range_upper_bound[n-1]+(n==4?1:0)); i+=(1<<6)) // i=q1.31 arg1
        {
            int q131_arg1=i;
            
            arg1=q131_to_float(q131_arg1);
            double x;
            for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
            {
                
                MUL=1<<POINT_POS;
                targ1 = i;
                cordic(&targ1, &targ2, -1, 0, n, 2);

                int j;
                for (j=0; (q131_arg1+j)<(arg1_range_upper_bound[n-1]+(n==3?1:0))&&j<(1<<6);j++)
                {
                    arg1=q131_to_float(i+j);
                    x=arg1 * pow(2, n);
                    error_ln = (targ1/MUL131)-log(x)/(pow(2, n+1)); // res1 = targ1 = ln(x) / (2^(n+1))
                    int inputs[2]={i+j, n};
                    if ((((unsigned)targ1>>31)&&(log(x)/(pow(2, n+1)))>=0)||!((unsigned)targ1>>31)&&(log(x)/(pow(2, n+1)))<0)
                    {
                        printf("Sign error detected\n");
                        printf("expected result=%.20f, cordic result=%.20f\n", log(x)/(pow(2, n+1)), targ1/MUL131);
                        printf("cordic result=%x\n", targ1);
                        printf("arg1=%x, arg2=%x\n", i+j, n);
                        printf("x=%f", q131_to_float(i+j));
                        ln_sign_error++;
                    }
                    update_error_stat(&error_stat_ln[POINT_POS-21], error_ln, inputs, targ1);
                }
            }
            printf("\rtested case=%.20f, n=%d", q131_to_float(i), n);
        }
    }

    printf("\nStore the data into: ./error_analysis/ln_error_analysis-all.txt\n");
    if (!freopen("./error_analysis/ln_error_analysis-all.txt", "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }
    for (i = 0; i < 9; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_ln:\n");
        printf("ln_sign_error=%d\n", ln_sign_error);
        print_error_information(&error_stat_ln[i]);
    }
    printf("seed=%d\n", seed);
}
