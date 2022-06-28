// all possible q1.31 input 
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
    double arg1,arg2;
    int targ1,targ2;
    int i;
    int atanh_sign_error = 0;
    double error_atanh;
    int seed = 1649824994; // time(0);
    srand(seed);
    error_stats error_stat_atanh[9] = {0};
    for (i = 0; i < 9; i++)
        error_stat_atanh[i].min_error = 100;
    
    for(i=0;i<=0x33958106;i+=(1<<6)) // 0x33958106=q1.31 0.403
    {
        

        arg1 = q131_to_float(i);
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1*MUL131;
            cordic(&targ1, &targ2, -1, 0, 1, 0);
            int j;
            
            for (j=0; (i+j)<=0x33958106&&j<(1<<6); j++) // change the last 6 bits and input to the floating point algorithm.
            {
                int inputs[2] ={i, 1};
                arg1=q131_to_float(i+j);
                error_atanh = targ1/MUL131-atanh(arg1*2)/2;  
                inputs[0] = i+j;
                inputs[1] = 1;
                if ((((unsigned)targ1>>31)&&atanh(arg1*2)/2>=0)||!((unsigned)targ1>>31)&&atanh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", atanh(arg1*2)/2, targ1/MUL131);
                    printf("cordic result=%x\n", targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    atanh_sign_error++;
                }
                update_error_stat(&error_stat_atanh[POINT_POS-21], error_atanh, inputs, targ1);
            }
            
        }
        printf("\rtested case=%.20f", q131_to_float(i));
    }
    // negative arg1
    for (i=0xcc6a7ec0; i<0; i+=(1<<6)) // 0xcc6a7efa = q1.31 -0.402
    {
        arg1 = q131_to_float(i); // arg1=x/2, x= arg1*2, range=[-0.559, 0.559]
        for (POINT_POS=23, BIT=26; POINT_POS <= 23; POINT_POS++, BIT++)
        {
            MUL=1<<POINT_POS;
            targ1 = arg1*MUL131;

            cordic(&targ1, &targ2, -1, 0, 1, 0);  // targ1 = res1 = cosh(x)/2 = cosh(arg1*2)/2, targ2 = res2 = sinh(x)/2 = sinh(arg1*2)/2
            int j;
            
            for (j=0x03f; (i+j)>=0xcc6a7efa&&j>=0; j--) // change the last 6 bits and input to the floating point algorithm.
            {
                int inputs[2] = {i, 1};
                arg1=q131_to_float(i+j);
                error_atanh = targ1/MUL131-atanh(arg1*2)/2;  
                inputs[0] = i+j;
                inputs[1] = 1;
                if ((((unsigned)targ1>>31)&&atanh(arg1*2)/2>=0)||!((unsigned)targ1>>31)&&atanh(arg1*2)/2<0)
                {
                    printf("Sign error detected\n");
                    printf("expected result=%.20f, cordic result=%.20f\n", atanh(arg1*2)/2, targ1/MUL131);
                    printf("cordic result=%x\n", atanh(arg1*2)/2, targ1);
                    printf("arg1=%x, arg2=%x\n", i+j, 1);
                    printf("x=%f", q131_to_float(i+j));
                    atanh_sign_error++;
                }
                update_error_stat(&error_stat_atanh[POINT_POS-21], error_atanh, inputs, targ1);
            }
        }
        printf("\rtested case=%.20f", q131_to_float(i));
    }
    printf("\nStore the data into: ./error_analysis/arctanh_error_analysis_all.txt\n");
    if (!freopen("./error_analysis/arctanh_error_analysis_all.txt", "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_atanh:\n");
        printf("atanh_sign_error=%d\n", atanh_sign_error);
        print_error_information(&error_stat_atanh[i]);
    }
    printf("seed=%d\n", seed);

}
