#include "cordic_verilog.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>

int float_to_q131(double src)
{
    return (int)(src*MUL131);
}

double q131_to_float(int src)
{
    return src/MUL131;
}
int log_for_any_base(double x, int base)
{
    int result = log(fabs(x))/log(base);
    if (result > 100)
        return 100;
    else if (result < -100)
        return -100;
    return result;
}
int main()
{
    BIT=26;
    POINT_POS=23;
    MUL=1<<POINT_POS;
    uint32_t arg1, arg2;
    printf("Enter arg1: ");
    scanf("%x", &arg1);
    printf("\nEnter arg2: ");
    scanf("%x", &arg2);
    double x = q131_to_float((signed)arg1);
    double y = q131_to_float((signed)arg2);
    int32_t targ1=arg1, targ2=arg2;
    
    cordic(&targ1, &targ2, 1, 0, 0, 1);
    double expected_atan2_result = atan2(y, x)/M_PI;
    double expected_mod_result = (sqrt(x*x+y*y) >= 1.0 ? 1.0 : sqrt(x*x+y*y));
    double error_atan2 = targ1/MUL131-expected_atan2_result;
    double error_modulus = targ2/MUL131-expected_mod_result;
    printf("phase expected result=%.20f, cordic result=%.20f\n", expected_atan2_result, targ1/MUL131);
    printf("cordic result=%x\n", targ1);
    printf("mod expected result=%.20f, cordic result=%.20f\n", expected_mod_result, targ2/MUL131);
    printf("cordic result=%x\n", targ2);
    printf("arg1=%x, arg2=%x\n", arg1, arg2);
    printf("x=%f, y=%f\n", x, y);
    printf("error_atan2=%E\n", error_atan2);
    printf("error_modulus=%E\n", error_modulus);
    printf("error_atan2(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(error_atan2, 2));
    printf("error_modulus(in terms of N, 2^N >= error > 2^(N-1))=%d\n", log_for_any_base(error_modulus, 2));
    return 0;
}
