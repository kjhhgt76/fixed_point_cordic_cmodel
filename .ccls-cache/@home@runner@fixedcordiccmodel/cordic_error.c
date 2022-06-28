#include <stdio.h>
#include <math.h>
#include "cordic_error.h"

/*
    @brief  A function that add new error data into the error_stat.
    @param  error_stat  an error_stats pointer points to error_stat. This struct will be updated. 
    @param  error	the actual error of the current iteration.
*/
void update_error_stat(error_stats* error_stat, double error, const int inputs[], int output) { 
    error_stat->sum_of_errors += error;
    error_stat->sum_of_errors_square += (error * error);
    if (fabs(error) > fabs(error_stat->bad_error[error_stat->bad_index]))
    {
        error_stat->bad_error[error_stat->bad_index] = error;
        error_stat->bad_input1[error_stat->bad_index] = inputs[0];
        error_stat->bad_input2[error_stat->bad_index] = inputs[1];
        error_stat->bad_output[error_stat->bad_index] = output;
        if ((++error_stat->bad_index) >= MAX_BAD_INPUT_NUM)
            error_stat->bad_index = 0;
        if (fabs(error) > fabs(error_stat->max_error))
            error_stat->max_error = error;
    }
    if (fabs(error) < fabs(error_stat->min_error))
        error_stat->min_error = error;
    if (error < 0)
        error_stat->negative_error_count++;
    error_stat->data_count++;
}

/*
    @brief  A function that will print out the error of the simulation according to the data inside error_stat.
    @param  error_stat	an error_stats pointer points to error_stat.
*/
void print_error_information(const error_stats* error_stat) {
    double mean_error = (error_stat->data_count == 0) ? 0 : (error_stat->sum_of_errors / error_stat->data_count);
    double std = (error_stat->data_count == 0)  ? 0 : (sqrt(error_stat->sum_of_errors_square/error_stat->data_count-mean_error*mean_error)); // std = sqrt(E(X^2)-E(X)^2)

    printf("Sum_of_errors(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(error_stat->sum_of_errors, 2));
    printf("Mean_of_errors(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(mean_error, 2));
    printf("Sum_of_errors_sign=%s\n", error_stat->sum_of_errors < 0 ? "-" : "+");
    printf("Mean_error_sign=%s\n", mean_error < 0 ? "-" : "+");
    printf("Std_of_error(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(std, 2));
    printf("Max_error_mag(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(error_stat->max_error, 2));
    printf("Min_error_mag(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(error_stat->min_error, 2));
    printf("Negative error count: %lu\n", error_stat->negative_error_count);
    printf("Number of test: %lu\n", error_stat->data_count);
    printf("Mean_of_errors(in terms of floating point)=%E\n", mean_error);
    printf("Max_of_errors(in terms of floating point)=%E\n", error_stat->max_error);
    printf("Min_of_errors(in terms of floating point)=%E\n", error_stat->min_error);
    int i;
    for (i=0; i<MAX_BAD_INPUT_NUM; i++)
    {
        printf("bad_input1[%d](arg1)=%x\n", i, error_stat->bad_input1[i]);
        printf("bad_input2[%d](arg2 or n)=%x\n", i, error_stat->bad_input2[i]);
        printf("bad_output[%d]=%x\n", i, error_stat->bad_output[i]);
        printf("bad_error[%d](in terms of N, 2^N >= error > 2^(N-1))=%d\n", i, log_for_any_base(error_stat->bad_error[i], 2));
        printf("bad_error[%d](in terms of floating point)=%E\n", i, error_stat->bad_error[i]);
    }
    printf("----------------------------\n\n");
}

/*
    @brief  A function that returns log(x) for any base.

    @param  x       the input double.
    @param  base    the base of the log function.
*/
int log_for_any_base(double x, int base)
{
    int result = log(fabs(x))/log(base);
    if (result > 100)
        return 100;
    else if (result < -100)
        return -100;
    return result;
}
