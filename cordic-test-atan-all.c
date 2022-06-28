// all possible input [0, 2^32-1] q3.23
#include <stdio.h>
#include <math.h>
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"


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
    unsigned int i; // q1.21
    unsigned int sign_error = 0;
    int large_error_count[9] = {0};
    double error_atan;
    int seed = 1649398432;
    srand(seed);
    error_stats error_stat_atan[9] = {0, 0, 0, 0, 0, 0};
    for (i = 0; i < 9; i++)
    {
        error_stat_atan[i].min_error = 100;
    }
    printf("atan, all possible input test\n");
    unsigned int started=0;
    for(i=0;i!=0||started==0;i+=(1<<6))
    {
      started=1;
      int n;
      arg1 = q131_to_float(i);
      for (n=0; n<=7; n++)
      {
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = float_to_q131(arg1);
            cordic(&targ1, &targ2, 1, 0, n, 0);
            int j;
            for (j=0;j<(1<<6);j++){
                arg1=q131_to_float(i+j);
                error_atan = targ1/MUL131-pow(2,-n)*atan(arg1*pow(2,n))/M_PI;
                int inputs[2]={0};
                inputs[0] = i+j;
                inputs[1] = n;
                if ((((unsigned)targ1>>31)&&(pow(2,-n)*atan(arg1*pow(2,n))/M_PI)>=0)||!((unsigned)targ1>>31)&&(pow(2,-n)*atan(arg1*pow(2,n))/M_PI)<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", pow(2,-n)*atan(arg1*pow(2,n))/M_PI, targ1/MUL131);
                    printf("cordic result=%x\n", targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, n);
                    printf("x=%f\n", q131_to_float(i+j));
                    sign_error++;
                    return 0;
                }
                update_error_stat(&(error_stat_atan[POINT_POS-21]), error_atan, inputs, targ1);
            }
        }
      }
      printf("\rtested case=%.20f", q131_to_float((signed)i));
    }
    printf("\nStore the data into: ./error_analysis/atan_error_analysis_all.txt\n");
    if (!freopen("./error_analysis/atan_error_analysis_all.txt", "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atan:\n");
        printf("sign_error=%d\n", sign_error);
        print_error_information(&error_stat_atan[i]);
    }
    // printf("seed=%d\n", seed);
}
