// many random input
#include <stdio.h>
#include <math.h> // for testing only!
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
    // variable setup
    int arg1,arg2;
    int targ1,targ2;
    double error1, error2;
    unsigned long i;
    int seed = time(NULL);
    long cos_sign_error_count = 0, sin_sign_error_count = 0;
    srand(seed);
    error_stats error_stat_sin[9] = {0};
    error_stats error_stat_cos[9] = {0};
    POINT_POS=23;
    BIT=26;
    for (i = 0; i < 9; i++)
    {
        error_stat_sin[i].min_error = 100;
        error_stat_cos[i].min_error = 100;
    }

    // Running time setup
    double hours=0;
    printf("Enter running hours: ");
    scanf("%lf", &hours);
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Start time: %s\n", asctime (timeinfo) );
    printf ( "Run hours: %f\n", hours );
    time_t start_time = time(NULL);
    time_t target_seconds = 60*60*hours;
    time_t time_passed = 0;
    printf("Many test cases for cordic sin cos:\n");

    // File setup
    char file_str[100];
    sprintf(file_str, "./error_analysis/sincos-many-%s-%lfh.txt",asctime (timeinfo), hours);
    char *p = file_str;
    for (; *p; ++p)
    {
        if (*p == ' ')
              *p = '-';
    }
    printf("\nData will be stored into: %s\n", file_str);
    if (!freopen(file_str, "w", stdout))
    {
        perror("Unable to open file.\n");
        return -1;
    }

    // Start test cases
    for(i=0;i<MAX_CASE_NUMBER && time_passed<target_seconds;i++)
    {
        arg1 = rand() ^ ((rand() % 2) << 31); // q1.31 arg1, range = [-1, 1]
        arg2 = rand(); // q1.31 arg2, range = [0, 1]
        double x = q131_to_float(arg1);
        double y = q131_to_float(arg2);
        
            MUL=1<<POINT_POS;
            targ1 = arg1;
            targ2 = arg2;
            cordic(&targ1, &targ2, 1, 1, 0, 0);

            error1 = targ1/MUL131-y*cos(x*M_PI);
            error2 = targ2/MUL131-y*sin(x*M_PI);
            if ((((unsigned)targ1>>31)&&cos(x*M_PI)>=0)||!((unsigned)targ1>>31)&&cos(x*M_PI)<0)
            {
                printf("Sign error detected\n");
                printf("expected result=%.20f, cordic result=%.20f\n", y*cos(x*M_PI), targ1/MUL131);
                printf("cordic result=%x\n", targ1);
                printf("arg1=%x, arg2=%x\n", arg1, arg2);
                printf("x=%f, y=%f\n", x, y);
                cos_sign_error_count++;
                break;
            }
            if ((((unsigned)targ2>>31)&&sin(x*M_PI)>=0)||!((unsigned)targ2>>31)&&sin(x*M_PI)<0)
            {
                printf("Sign error detected\n");
                printf("expected result=%.20f, cordic result=%.20f\n", y*sin(x*M_PI), targ2/MUL131);
                printf("cordic result=%x\n", targ2);
                printf("arg1=%x, arg2=%x\n", arg1, arg2);
                printf("x=%f, y=%f\n", x, y);
                sin_sign_error_count++;
            }
            int inputs[2] = {arg1, arg2};
            update_error_stat(&error_stat_cos[POINT_POS-21], error1, inputs, targ1);
            update_error_stat(&error_stat_sin[POINT_POS-21], error2, inputs, targ2);
        
        
        if ((time(NULL)-start_time) != time_passed)
        {
            time_passed = time(NULL)-start_time;
           
        }
    }
    // print out error statistics.
    for (i = 2; i < 3; i++)
    {
        printf("----------------------------------\n");
        printf("%d bit  %d iteration    q3.%d:\n", 24+i, ITERATION, 21+i);
        printf("error_stat_cos:\n");
        printf("cos_sign_error_count=%lu\n", cos_sign_error_count);
        print_error_information(&error_stat_cos[i]);
        printf("error_stat_sin:\n");
        printf("sin_sign_error_count=%lu\n", sin_sign_error_count);
        print_error_information(&error_stat_sin[i]);
    }
    printf("seed=%d\n", seed);
}
