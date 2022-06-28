#include <stdio.h>
#include <math.h>
#include "cordic_error.h"
#include <time.h>
#include <stdlib.h>
#include "cordic_verilog.h"

#define MAX_CASE_NUMBER (unsigned long)-1

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
    uint32_t arg1,arg2;
    int targ1,targ2;
    uint32_t i,j, k; 
    int large_error_count[9] = {0};
    double error_atan2, error_modulus;
    int seed = time(NULL);
    unsigned long phase_sign_error = 0, mod_sign_error = 0;
    srand(seed);
    error_stats error_stat_atan2[9] = {0, 0, 0, 0, 0, 0};
    error_stats error_stat_modulus[9] = {0, 0, 0, 0, 0, 0};
    for (i = 0; i < 9; i++)
    {
        error_stat_atan2[i].min_error = 100;
        error_stat_modulus[i].min_error = 100;
    }
    
    printf("18 same MSB test cases for cordic phase modulus:\n");



    for(i=0;i<0x04000;i++)
    {
        printf("\ri=%02x",i);
        fflush(stdout);
        for (j=0;j<0x04000;j++)
        {
            for (k=0;k<4;k++)
            {
                switch(k)
                {
                    case 0: 
                        arg1 = i; // q1.31 arg1, range = [-1, 1]
                        arg2 = j; // q1.31 arg2, range = [-1, 1]
                        break;
                    case 1:
                        arg1 = i; // q1.31 arg1, range = [-1, 1]
                        arg2 = 0xffffc000 | j; // q1.31 arg2, range = [-1, 1]
                        break;
                    case 2:
                        arg1 = 0xffffc000 | i; // q1.31 arg1, range = [-1, 1]
                        arg2 = 0xffffc000 | j; // q1.31 arg2, range = [-1, 1]
                        break;
                    case 3:
                        arg1 = 0xffffc000 | i; // q1.31 arg1, range = [-1, 1]
                        arg2 = j; // q1.31 arg2, range = [-1, 1]
                        break;
                }
                double x = q131_to_float(arg1);
                double y = q131_to_float(arg2);
                
                for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
                {
                    MUL=1<<POINT_POS;
                    targ1 = arg1;
                    targ2 = arg2;
                    cordic(&targ1, &targ2, 1, 0, 0, 1);
                    
                    double expected_atan2_result = atan2(y, x)/M_PI;
                    double expected_mod_result = (sqrt(x*x+y*y) >= 1.0 ? 1.0 : sqrt(x*x+y*y));
                    error_atan2 = targ1/MUL131-expected_atan2_result;
                    error_modulus = targ2/MUL131-expected_mod_result;
                    int need_break = 0;
                    if ((((unsigned)targ1>>31)&&expected_atan2_result>=0)||!((unsigned)targ1>>31)&&expected_atan2_result<0)
                    {
                        printf("phase Sign error detected\n");
                        printf("expected result=%.20f, cordic result=%.20f\n", expected_atan2_result, targ1/MUL131);
                        printf("cordic result=%x\n", targ1);
                        printf("arg1=%x, arg2=%x\n", arg1, arg2);
                        printf("x=%f, y=%f\n", x, y);
                        phase_sign_error++;
                        return 0;
                    }
                    if ((((unsigned)targ2>>31)&&expected_mod_result>=0)||!((unsigned)targ2>>31)&&expected_mod_result<0)
                    {
                        printf("mod Sign error detected\n");
                        printf("expected result=%.20f, cordic result=%.20f\n", expected_mod_result, targ2/MUL131);
                        printf("cordic result=%x\n", targ2);
                        printf("arg1=%x, arg2=%x\n", arg1, arg2);
                        printf("x=%f, y=%f\n", x, y);
                        mod_sign_error++;
                        need_break=1;
                    }
                    if (need_break) break;
                    int inputs[2] = {arg1, arg2};
                    update_error_stat(&(error_stat_atan2[POINT_POS-21]), error_atan2, inputs, targ1);
                    update_error_stat(&(error_stat_modulus[POINT_POS-21]), error_modulus, inputs, targ2);
                }
            }
        }
    }
    // File setup
    char file_str[100];
    sprintf(file_str, "./error_analysis/phmod-18sameMSB.txt");
    char *p = file_str;
    for (; *p; ++p)
    {
        if (*p == ' ' || *p == '\n')
              *p = '-';
    }
    printf("\nData will be stored into: %s\n", file_str);
    if (!freopen(file_str, "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }
    
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atan2:\n");
        printf("phase_sign_error=%lu\n", phase_sign_error);
        print_error_information(&error_stat_atan2[i]);
        printf("error_stat_modulus:\n");
        printf("mod_sign_error=%lu\n", mod_sign_error);
        print_error_information(&error_stat_modulus[i]);
    }
    printf("seed=%d\n", seed);
} 
