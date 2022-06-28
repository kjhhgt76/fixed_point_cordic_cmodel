#ifndef MY_CORDIC_ERROR
#define MY_CORDIC_ERROR

#define MUL131e 2147483648.0
#define MAX_BAD_INPUT_NUM 10
/**
*    @struct error_stats 
*    @brief 
*    a struct storing the needed data for error calculation of simulation of one cordic algorithm.
*    @param sum_of_errors The sum of error.
*    @param sum_of_errors_square the sum of the square of each iteration. It is used to calculate the std of errors.
*    @param max_error The biggest error.
*    @param min_error The smallest error.
*    @param negative_error_count The number of negative errors.
*    @param data_count The total number of errors.
*    @param bad_index It is the index for accessing MAX_BAD_INPUT_NUM bad cases.
*    @param bad_input1[MAX_BAD_INPUT_NUM] It is a circular buffer which stores MAX_BAD_INPUT_NUM bad sample input arg1.
*    @param bad_input2[MAX_BAD_INPUT_NUM] It is a circular buffer which stores MAX_BAD_INPUT_NUM bad sample input arg2 or n.
*    @param bad_output[MAX_BAD_INPUT_NUM] It is a circular buffer which stores MAX_BAD_INPUT_NUM bad sample output.
*/

typedef struct error_stats{
    double sum_of_errors;
    double sum_of_errors_square;
    double max_error;
    double min_error;
    unsigned long negative_error_count;
    unsigned long data_count;
    int bad_index;
    int bad_input1[MAX_BAD_INPUT_NUM];
    int bad_input2[MAX_BAD_INPUT_NUM];
    int bad_output[MAX_BAD_INPUT_NUM];
    double bad_error[MAX_BAD_INPUT_NUM];
} error_stats;

void update_error_stat(error_stats* error_stat, double error, const int inputs[], int output);
void print_error_information(const error_stats* error_stat);
int log_for_any_base(double x, int base);
#endif
