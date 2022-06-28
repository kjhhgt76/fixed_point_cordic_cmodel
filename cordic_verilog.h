//Cordic in fixed point math
//Function is valid for arguments in range -pi/2 -- pi/2
//for values pi/2--pi: value = half_pi-(theta-half_pi) and similarly for values -pi---pi/2
#include <stdlib.h>
#include <math.h>
#include <stdio.h> 
#include <stdint.h>
#define ITERATION 24

int BIT;
int POINT_POS;
double MUL;
static int overflow_count = 0;
#ifndef debug
#define debug 0
#endif

#define MUL131 2147483648.0

/*
    Add two qa.b numbers. It also will detect the overflow and saturate if there is any overflow.
    @param x qa.b number.
    @param y qa.b number.
    @return addition result.
*/
int add_with_overf_detect(int x, int y)
{
    unsigned int result = x + y;
    if ((x & (1<<(POINT_POS))) == (y & (1<<(POINT_POS))) && (result & (1<<(POINT_POS))) != (x & (1<<(POINT_POS))))
    {
        if (debug)
        {
            printf("Overflow detected!\n");
            printf("overflow: x=%x, y=%x\n, result=%x\n", x, y, result);        
        }        
        return result;
    }    
    return result;
}

/*
    @param src q1.31 fixed point number.
    @return q(BIT-POINT_POS).POINT_POS fixed point number.
*/
int q131_to_qxy(const int src)
{
    int mask = ((src>>31)%2) ? (0x03 << (POINT_POS+1)) : (unsigned)0xffffffff>> (31-POINT_POS);
    if ((src>>31)%2)
    {
        return (src>>(31-POINT_POS)) | mask;
    }
    else
    {
        return (src>>(31-POINT_POS)) & mask;
    }
}

/*
    @param x a qa.b number.
    @param y a qa.b number.
    @return a qa.b x times y number.
*/
int f_multiply(const int x, const int y)
{
    return ((long long)x * y) >> POINT_POS;
}

/*
    @param src q3.21 fixed point number.
    @return q1.31 fixed point number.
*/
int qxy_to_q131(const int src)
{
    if (((src>>(BIT-1))%2) != ((src>>(POINT_POS))%2))
    {
        if (debug)
            printf("Overflow detected during q.format conversion\n");
        overflow_count++;
    }
    return src << (31-POINT_POS);
}

/*
    @param arg1 The pointer points to the arg1. res1 will be stored here.
    @param arg2 The pointer points to the arg2. res2 will be stored here.
    @param mode The cordic mode. Equals 1 in circular mode. Equals -1 in hyperbolic mode.
    @param rot  Rotation or vectoring mode. 1 for rotation mode. 0 for vectoring mode.
    @param n    The scale n. Only for atan, ln, sqrt function.
    @param atan_mode For choosing a function among 0-atan, 1-phase in circular vectoring mode and among 0-atanh, 2-ln, 1-sqrt in hyperbolic vectoring mode.
*/
void cordic(int *arg1, int *arg2, const int mode, const int rot, const int n, const int atan_mode)
{
    /*****Variables setup*****/
    if (debug)
    {
        printf("\n$$$$Enter cordic function$$$$\n");
        printf("*****Variables setup*****\n");
    }
    int i;
    int repeated_it = 4;
    int cordic_table[ITERATION]={0};
    const int cordic_1K = (mode != -1) ? (POINT_POS == 23 ? 0x4dba76: (int)(MUL*0.6072529350088812561694)) : (POINT_POS == 23 ? 0x9a8f43 : (int)(MUL*1.207497067763));
    const int PI = (POINT_POS == 23) ? 0x1921fb5 : (int)(MUL*M_PI);
    const int PI_REP = (POINT_POS == 23) ? 0x028BE60 : (int)(MUL*(1/M_PI));
    int k, d, x, y, z, tx, ty, tz, ee, quadrant;
    const int one = ((unsigned int)0xffffffff>>(32-POINT_POS));
    const uint32_t qxy_arg1=q131_to_qxy(*arg1), qxy_arg2=q131_to_qxy(*arg2);
    const int arg1_sign = ((qxy_arg1>>(BIT-1))% 2), arg2_sign = ((qxy_arg2>>(BIT-1))% 2);
    const int exp_en_arg1_6b = ((((unsigned)*arg1>>26)%0x040) == 0) || ((((unsigned)*arg1>>26)%0x040) == 0x03f);
    const int exp_en_arg2_6b = ((((unsigned)*arg2>>26)%0x040) == 0) || ((((unsigned)*arg2>>26)%0x040) == 0x03f);
    const int exp_en_arg1_12b = ((((unsigned)*arg1>>20)%0x01000) == 0) || ((((unsigned)*arg1>>20)%0x01000) == 0x0fff);
    const int exp_en_arg2_12b = ((((unsigned)*arg2>>20)%0x01000) == 0) || ((((unsigned)*arg2>>20)%0x01000) == 0x0fff);
    const int exp_en_arg1_18b = ((((unsigned)*arg1>>14)%0x040000) == 0) || ((((unsigned)*arg1>>14)%0x040000) == 0x03ffff);
    const int exp_en_arg2_18b = ((((unsigned)*arg2>>14)%0x040000) == 0) || ((((unsigned)*arg2>>14)%0x040000) == 0x03ffff);
    const int exp_en_arg1_24b = ((((unsigned)*arg1>>8)%0x01000000) == 0) || ((((unsigned)*arg1>>8)%0x01000000) == 0x0ffffff);
    const int exp_en_arg2_24b = ((((unsigned)*arg2>>8)%0x01000000) == 0) || ((((unsigned)*arg2>>8)%0x01000000) == 0x0ffffff);
    const int exp_en_arg1_5b = ((((unsigned)*arg1>>27)%0x020) == 0) || ((((unsigned)*arg1>>27)%0x020) == 0x01f);
    const int exp_en_arg2_5b = ((((unsigned)*arg2>>27)%0x020) == 0) || ((((unsigned)*arg2>>27)%0x020) == 0x01f);
    const int exp_en_arg1_11b = ((((unsigned)*arg1>>21)%0x0800) == 0) || ((((unsigned)*arg1>>21)%0x0800) == 0x07ff);
    const int exp_en_arg2_11b = ((((unsigned)*arg2>>21)%0x0800) == 0) || ((((unsigned)*arg2>>21)%0x0800) == 0x07ff);
    const int exp_en_arg1_17b = ((((unsigned)*arg1>>15)%0x020000) == 0) || ((((unsigned)*arg1>>15)%0x020000) == 0x01ffff);
    const int exp_en_arg2_17b = ((((unsigned)*arg2>>15)%0x020000) == 0) || ((((unsigned)*arg2>>15)%0x020000) == 0x01ffff);
    const int exp_en_arg1_23b = ((((unsigned)*arg1>>9)%0x0800000) == 0) || ((((unsigned)*arg1>>9)%0x0800000) == 0x07fffff);
    const int exp_en_arg2_23b = ((((unsigned)*arg2>>9)%0x0800000) == 0) || ((((unsigned)*arg2>>9)%0x0800000) == 0x07fffff);
    const int exp_en_arg1_9b = ((((unsigned)*arg1>>23)%0x0200) == 0) || ((((unsigned)*arg1>>23)%0x0200) == 0x01ff);
    const int exp_en_arg2_9b = ((((unsigned)*arg2>>23)%0x0200) == 0) || ((((unsigned)*arg2>>23)%0x0200) == 0x01ff);
    const int exp_en_arg1_22b = ((((unsigned)*arg1>>10)%0x0400000) == 0) || ((((unsigned)*arg1>>10)%0x0400000) == 0x03fffff);
    const int exp_en_arg2_22b = ((((unsigned)*arg2>>10)%0x0400000) == 0) || ((((unsigned)*arg2>>10)%0x0400000) == 0x03fffff);
    const int exp_en_arg1_16b = ((((unsigned)*arg1>>16)%0x010000) == 0) || ((((unsigned)*arg1>>16)%0x010000) == 0x0ffff);
    const int exp_en_arg2_16b = ((((unsigned)*arg2>>16)%0x010000) == 0) || ((((unsigned)*arg2>>16)%0x010000) == 0x0ffff);
    const int exp_en_arg1_7b = ((((unsigned)*arg1>>25)%0x080) == 0) || ((((unsigned)*arg1>>25)%0x080) == 0x07f);
    const int exp_en_arg2_7b = ((((unsigned)*arg2>>25)%0x080) == 0) || ((((unsigned)*arg2>>25)%0x080) == 0x07f);
    const int exp_en_arg1_8b = ((((unsigned)*arg1>>24)%0x0100) == 0) || ((((unsigned)*arg1>>24)%0x0100) == 0x0ff);
    const int exp_en_arg2_8b = ((((unsigned)*arg2>>24)%0x0100) == 0) || ((((unsigned)*arg2>>24)%0x0100) == 0x0ff);
    const int exp_en_arg1_10b = ((((unsigned)*arg1>>22)%0x0400) == 0) || ((((unsigned)*arg1>>22)%0x0400) == 0x03ff);
    const int exp_en_arg2_10b = ((((unsigned)*arg2>>22)%0x0400) == 0) || ((((unsigned)*arg2>>22)%0x0400) == 0x03ff);
    const int exp_en_arg1_13b = ((((unsigned)*arg1>>19)%0x02000) == 0) || ((((unsigned)*arg1>>19)%0x02000) == 0x01fff);
    const int exp_en_arg2_13b = ((((unsigned)*arg2>>19)%0x02000) == 0) || ((((unsigned)*arg2>>19)%0x02000) == 0x01fff);
    const int exp_en_arg1_14b = ((((unsigned)*arg1>>18)%0x04000) == 0) || ((((unsigned)*arg1>>18)%0x04000) == 0x03fff);
    const int exp_en_arg2_14b = ((((unsigned)*arg2>>18)%0x04000) == 0) || ((((unsigned)*arg2>>18)%0x04000) == 0x03fff);
    const int exp_en_arg1_15b = ((((unsigned)*arg1>>17)%0x08000) == 0) || ((((unsigned)*arg1>>17)%0x08000) == 0x07fff);
    const int exp_en_arg2_15b = ((((unsigned)*arg2>>17)%0x08000) == 0) || ((((unsigned)*arg2>>17)%0x08000) == 0x07fff);
    const int exp_en_arg1_19b = ((((unsigned)*arg1>>13)%0x080000) == 0) || ((((unsigned)*arg1>>13)%0x080000) == 0x07ffff);
    const int exp_en_arg2_19b = ((((unsigned)*arg2>>13)%0x080000) == 0) || ((((unsigned)*arg2>>13)%0x080000) == 0x07ffff);
    const int exp_en_arg1_20b = ((((unsigned)*arg1>>12)%0x0100000) == 0) || ((((unsigned)*arg1>>12)%0x0100000) == 0x0fffff);
    const int exp_en_arg2_20b = ((((unsigned)*arg2>>12)%0x0100000) == 0) || ((((unsigned)*arg2>>12)%0x0100000) == 0x0fffff);
    const int exp_en_arg1_21b = ((((unsigned)*arg1>>11)%0x0200000) == 0) || ((((unsigned)*arg1>>11)%0x0200000) == 0x01fffff);
    const int exp_en_arg2_21b = ((((unsigned)*arg2>>11)%0x0200000) == 0) || ((((unsigned)*arg2>>11)%0x0200000) == 0x01fffff);
    const int exp_en_arg1_4b = ((((unsigned)*arg1>>28)%0x010) == 0) || ((((unsigned)*arg1>>28)%0x010) == 0x0f);
    const int exp_en_arg2_4b = ((((unsigned)*arg2>>28)%0x010) == 0) || ((((unsigned)*arg2>>28)%0x010) == 0x0f);
    const int exp_en_arg1_27b = ((((unsigned)*arg1>>5)%0x08000000) == 0) || ((((unsigned)*arg1>>5)%0x08000000) == 0x07ffffff);
    const int exp_en_arg2_27b = ((((unsigned)*arg2>>5)%0x08000000) == 0) || ((((unsigned)*arg2>>5)%0x08000000) == 0x07ffffff);
    const int exp_en_arg1_26b = ((((unsigned)*arg1>>6)%0x04000000) == 0) || ((((unsigned)*arg1>>6)%0x04000000) == 0x03ffffff);
    const int exp_en_arg2_26b = ((((unsigned)*arg2>>6)%0x04000000) == 0) || ((((unsigned)*arg2>>6)%0x04000000) == 0x03ffffff);
    const int exp_en_6b = (exp_en_arg1_6b && exp_en_arg2_6b);
    const int exp_en_12b = (exp_en_arg1_12b && exp_en_arg2_12b);
    const int exp_en_18b = (exp_en_arg1_18b && exp_en_arg2_18b);
    const int exp_en_24b = (exp_en_arg1_24b && exp_en_arg2_24b);
    const int exp_en_5b = (exp_en_arg1_5b && exp_en_arg2_5b);
    const int exp_en_11b = (exp_en_arg1_11b && exp_en_arg2_11b);
    const int exp_en_17b = (exp_en_arg1_17b && exp_en_arg2_17b);
    const int exp_en_23b = (exp_en_arg1_23b && exp_en_arg2_23b);
    const int exp_en_9b = (exp_en_arg1_9b && exp_en_arg2_9b);
    const int exp_en_22b = (exp_en_arg1_22b && exp_en_arg2_22b);
    const int exp_en_16b = (exp_en_arg1_16b && exp_en_arg2_16b);
    const int exp_en_7b = (exp_en_arg1_7b && exp_en_arg2_7b);
    const int exp_en_8b = (exp_en_arg1_8b && exp_en_arg2_8b);
    const int exp_en_10b = (exp_en_arg1_10b && exp_en_arg2_10b);
    const int exp_en_13b = (exp_en_arg1_13b && exp_en_arg2_13b);
    const int exp_en_14b = (exp_en_arg1_14b && exp_en_arg2_14b);
    const int exp_en_15b = (exp_en_arg1_15b && exp_en_arg2_15b);
    const int exp_en_19b = (exp_en_arg1_19b && exp_en_arg2_19b);
    const int exp_en_20b = (exp_en_arg1_20b && exp_en_arg2_20b);
    const int exp_en_21b = (exp_en_arg1_21b && exp_en_arg2_21b);
    const int exp_en_4b = (exp_en_arg1_4b && exp_en_arg2_4b);
    const int exp_en_27b = (exp_en_arg1_27b && exp_en_arg2_27b);
    const int exp_en_26b = (exp_en_arg1_26b && exp_en_arg2_26b);
    
    if (debug)
    {
        printf("*arg1:%x\n*arg2:%x\nmode:%d\nrot:%d\nn:%d\natan_mode:%d\n", *arg1, *arg2, mode, rot, n, atan_mode);
        printf("arg1_sign=%d\narg2_sign=%d\nexp_en_arg1_6b=%d\nexp_en_arg2_6b=%d\nexp_en_6b=%d\n", arg1_sign, arg2_sign, exp_en_arg1_6b, exp_en_arg2_6b, exp_en_6b);
        printf("exp_en_arg1_12b=%d\nexp_en_arg2_12b=%d\nexp_en_12b=%d\n", exp_en_arg1_12b, exp_en_arg2_12b, exp_en_12b);
        printf("exp_en_arg1_18b=%d\nexp_en_arg2_18b=%d\nexp_en_18b=%d\n", exp_en_arg1_18b, exp_en_arg2_18b, exp_en_18b);
        printf("exp_en_arg1_23b=%d\nexp_en_arg2_23b=%d\nexp_en_23b=%d\n", exp_en_arg1_23b, exp_en_arg2_23b, exp_en_23b);
        printf("exp_en_arg1_16b=%d\nexp_en_arg2_16b=%d\nexp_en_16b=%d\n", exp_en_arg1_16b, exp_en_arg2_16b, exp_en_16b);
    }
    
    /*****LUT setup*****/
    if (debug)
        printf("*****LUT setup*****\n");
    if(POINT_POS == 21){
        if (debug)
        {
            printf("q3.21, Use liguo's table\n");
        }
        if (mode == 1)
        {
            int temp_table[24] = {0x1921fb, 0x0ed634, 0x07d6dd, 0x03fab7, 0x01ff56, 0x00ffea, 0x007ffd, 0x004000,
            0x002000, 0x001000, 0x000800, 0x000400, 0x000200, 0x000100, 0x000080, 0x000040, 0x000020, 0x000010, 0x000008, 0x000004, 0x000002, 0x000001, 0x000000, 0x000000};
            for (i=0; i<24; i++)
                cordic_table[i] = temp_table[i];
        }
        else if (mode == -1)
        {
            const int temp_table[24] = {0x1193ea, 0x082c57, 0x040562, 0x0200ab, 0x0200ab, 0x010015, 0x008003, 0x004000, 0x002000, 0x001000, 0x000800, 0x000400, 0x000200, 0x000100, 0x000100, 0x000080, 0x000040, 0x000020, 0x000010, 0x000008, 0x000004, 0x000002, 0x000001, 0x000000};
            for (i=0; i<24; i++)
                cordic_table[i] = temp_table[i];
        }
    }
    else if(POINT_POS == 23)
    {
        if (debug)
        {
            printf("q3.23, Use liguo's table\n");
        }
        if (mode == 1)
        {
            int temp_table[24] = {0x06487ed, 0x03b58ce, 0x01f5b76, 0x00feadd, 0x007fd57, 0x003ffab, 0x001fff5, 0x000ffff, 0x0007fff, 0x0003fff, 0x0001fff, 0x0001000, 0x0000800, 0x0000400, 0x0000200, 0x0000100, 0x0000080, 0x0000040, 0x0000020, 0x0000010, 0x0000008, 0x0000004, 0x0000002, 0x0000001};
            for (i=0; i<24; i++)
                cordic_table[i] = temp_table[i]>>n;
        }
        else if (mode == -1)
        {
            int temp_table[24] = {0x0464faa, 0x020b15e, 0x0101589, 0x00802ac, 0x00802ac, 0x0040055, 0x002000b, 0x0010001, 0x0008000, 0x0004000, 0x0002000, 0x0001000, 0x0000800, 0x0000400, 0x0000400, 0x0000200, 0x0000100, 0x0000080, 0x0000040, 0x0000020, 0x0000010, 0x0000008, 0x0000004, 0x0000002};
            for (i=0; i<24; i++)
                cordic_table[i] = temp_table[i]>>n;
        }
    }
    else
    {
        if (debug)
        {
            printf("Generate table\n");
        }
        for(i=0, k=(mode==-1?1:0);i<ITERATION;i++, k++)
        {
            if (mode == 1)
                cordic_table[i] = (atan(pow(2, -k)) * MUL);
            else if (mode == -1)
            {
                if (k == repeated_it+1)
                {
                    k--;
                    repeated_it = 3*k+1;
                }
                cordic_table[i] = atanh(pow(2, -k)) * MUL;
            }
        }
        repeated_it = 4;
    }
    
    if (debug)
    {
        printf("*arg1:%x, *arg2:%x, mode:%d, rot:%d, n:%d, atan_mode:%d\n", *arg1, *arg2, mode, rot, n, atan_mode);
        printf("cordic_1K=%.20f\n", cordic_1K/MUL);
        printf("cordic_1K=%x\n", cordic_1K);
    }

    /*****Input processing*****/
    
        /*****circular mode input*****/
    if (debug)
        printf("*****Input processing*****\n");
    if (mode == 1)
    {
        if (debug)
            printf("circular mode\n");
        // rotation mode
        if (rot == 1){
            if (debug)
            {
                printf("rotation mode\n");
            }
            x = qxy_arg2;//f_multiply(qxy_arg2, cordic_1K);
            y = 0;
            if (!((*arg1 >> (31))%2) && ((*arg1 >> (31-1))%2) && (*arg1 % (0x40000000)))      // ~arg1[23] &  arg1[22];
            {
                if (debug)
                {
                    printf("quadrant 2\n");
                    printf("qxy_arg1-one=%x, %f\n", qxy_arg1-one, (qxy_arg1-one)/MUL);
                    printf("PI=%x\n", PI);
                }
                z = qxy_arg1-one;
                z = f_multiply(z, PI);
                quadrant = 2;
                
            }
            else if (((*arg1 >> (31))%2) && !((*arg1 >> (31-1))%2) && (*arg1 % 0x40000000)!=0x3fffffff)  // arg1[23] & ~arg1[22];
            {
                if (debug)
                {
                    printf("quadrant 3\n");
                    printf("qxy_arg1+one=%x\n", qxy_arg1+one);
                }
                z = qxy_arg1+one;
                if (debug)
                    printf("f_multiply=%llx\n", (long long)z*PI);
                z = f_multiply(z, PI);
                quadrant = 3;
            }
            else // quadrant 1,4
            {
                if (debug)
                    printf("quadrant 1,4\n");
                z = f_multiply(qxy_arg1, PI);
                quadrant = 1;
            }
        }
        // vectoring mode, arctan
        else
        {
            if (atan_mode == 0)
            {
                if (debug)
                    printf("vectoring mode, atan function\n");
                x = one>>n;
                y = qxy_arg1;
                z = 0.0;
            }
            else
            {
                z = 0.0;
                if (debug)
                {
                    printf("vectoring mode\n");
                    printf("qxy_arg1 = %x, %.20f\n", qxy_arg1, qxy_arg1/MUL);
                    printf("qxy_arg2 = %x, %.20f\n", qxy_arg2, qxy_arg2/MUL);
                    printf("*arg1 = %x, %.20f\n", *arg1, *arg1/MUL131);
                    printf("*arg2 = %x, %.20f\n", *arg2, *arg2/MUL131);
                }
                x = qxy_arg1;
                y = qxy_arg2;
                //x = exp_en_4b ? (*arg1>>5) : x; //arg1_abs
                //y = exp_en_4b ? (*arg2>>5) : y; //arg2_abs
                x = exp_en_5b ? (*arg1>>4) : x; //arg1_abs
                y = exp_en_5b ? (*arg2>>4) : y; //arg2_abs
                //x = exp_en_6b ? (*arg1>>3) : x; //arg1_abs
                //y = exp_en_6b ? (*arg2>>3) : y; //arg2_abs
                //x = exp_en_8b ? (*arg1>>1) : x; //arg1_abs
                //y = exp_en_8b ? (*arg2>>1) : y; //arg2_abs
                x = exp_en_9b ? (*arg1) : x;
                y = exp_en_9b ? (*arg2) : y;
                //x = exp_en_10b ? (*arg1<<1) : x; //arg1_abs
                //y = exp_en_10b ? (*arg2<<1) : y; //arg2_abs
                //x = exp_en_11b ? (*arg1<<2) : x; //arg1_abs
                //y = exp_en_11b ? (*arg2<<2) : y; //arg2_abs
                //x = exp_en_12b ? (*arg1<<3) : x; //arg1_abs
                //y = exp_en_12b ? (*arg2<<3) : y; //arg2_abs
                //x = exp_en_13b ? (*arg1<<4) : x; //arg1_abs
                //y = exp_en_13b ? (*arg2<<4) : y; //arg2_abs
                //x = exp_en_15b ? (*arg1<<6) : x; //arg1_abs
                //y = exp_en_15b ? (*arg2<<6) : y; //arg2_abs
                x = exp_en_16b ? (*arg1<<7) : x; //arg1_abs
                y = exp_en_16b ? (*arg2<<7) : y; //arg2_abs
                //x = exp_en_17b ? (*arg1<<8) : x; //arg1_abs
                //y = exp_en_17b ? (*arg2<<8) : y; //arg2_abs
                //x = exp_en_18b ? (*arg1<<9) : x; //arg1_abs
                //y = exp_en_18b ? (*arg2<<9) : y; //arg2_abs
                //x = exp_en_19b ? (*arg1<<10) : x; //arg1_abs
                //y = exp_en_19b ? (*arg2<<10) : y; //arg2_abs
                //x = exp_en_20b ? (*arg1<<11) : x; //arg1_abs
                //y = exp_en_20b ? (*arg2<<11) : y; //arg2_abs
                //x = exp_en_21b ? (*arg1<<12) : x; //arg1_abs
                //y = exp_en_21b ? (*arg2<<12) : y; //arg2_abs
                //x = exp_en_22b ? (*arg1<<13) : x; //arg1_abs
                //y = exp_en_22b ? (*arg2<<13) : y; //arg2_abs
                x = exp_en_23b ? (*arg1<<14) : x; //arg1_abs
                y = exp_en_23b ? (*arg2<<14) : y; //arg2_abs
                //x = exp_en_24b ? (*arg1<<15) : x; //arg1_abs
                //y = exp_en_24b ? (*arg2<<15) : y; //arg2_abs
                //x = exp_en_26b ? (*arg1<<17) : x; //arg1_abs
                //y = exp_en_26b ? (*arg2<<17) : y; //arg2_abs
                if (*arg1>>31)
                {
                    x = (~x+1);
                }
                if (*arg2>>31)
                {
                    y = (~y+1);
                }
            }
        }
    }
        /*****hyperbolic mode input*****/
    else
    {
        if (debug)
            printf("hyperbolic mode\n");
        if (rot == 1)
        {
            if (debug)
                printf("rotating mode\n");
            x = cordic_1K>>1; // xi = K' / 2
            y = 0.0;
            z = qxy_arg1; 
        }
        else
        {
            if (debug)
                printf("vectoring mode\n");

            if (atan_mode == 2)
            {
                if (debug)
                {
                    printf("natural log mode\n");
                    printf("n=%d\n", n);
                }
                x = qxy_arg1 + (one>>n); 
                y = qxy_arg1 - (one>>n);
                z = 0.0;
            }
            else if (atan_mode == 1)
            {
                if (debug)
                    printf("sqrt mode, *arg1=%.20f\n", *arg1);
                x = qxy_arg1 + (one>>(n+2));
                y = qxy_arg1 - (one>>(n+2));
                z = 0.0;
            }
            else
            {
                if (debug)
                    printf("arctanh mode\n");
                x = one>>1;  // xi=1/2
                y = qxy_arg1;  // yi=arg1=x/2
                z = 0.0;
            }
        }
    }
    if (debug)
    {
        printf("qxy_arg1 = %x, %.20f\n", qxy_arg1, qxy_arg1/MUL);
        printf("qxy_arg2 = %x, %.20f\n", qxy_arg2, qxy_arg2/MUL);

        printf("xi = %x, %.20f\n", x, x/MUL);
        printf("yi = %x, %.20f\n", y, y/MUL);
        printf("zi = %x, %.20f\n", z, z/MUL);
    }

    /********The algorithm loop*********/
    if (debug)
        printf("*****The algorithm loop*****\n");
    for (k=(mode==-1?1:0),i=0; i<((mode==-1 && atan_mode==1) ? ITERATION/2 : ITERATION); ++k,++i)
    {
        if (debug)
            printf("x[%d] = %08X/%.20f, y[%d] = %08X/%.20f, z[%d] = %08X/%.20f, cordic_table[%d]=%08X\n", k, x, x/MUL, k, y, y/MUL, k, z, z/MUL, i, cordic_table[i]);
        if (rot == 1)
        {
            d = (z & (1<<(BIT-1))) ? 0 : 1; // d=0: z<0  d=1: z>=0
        }
        else
        {
            d = (y & (1<<(BIT-1))) ? 1 : 0; // d=0: y>=0  d=1: y<0
        }
        // d=0: epsilon=-1, d=1: epsilon=1
        if (debug)
            printf("d^(mode==1)=%d", d^(mode==1));

        if (mode == 1)
        {
            ee = cordic_table[i];
            if (debug)
                printf(" ee=%.10f : %x, tan(ee)=%.10f\n", ee/MUL, ee, tan(ee/MUL));
        }
        else if (mode == -1)
        {
            ee = cordic_table[i];
            if (k==repeated_it+1)
            {
                k--;
                repeated_it=3*k+1;
            }
            if (debug)
                printf(" ee=%.10f : %x, tanh(ee)=%.10f, k=%d\n", ee/MUL, ee, tanh(ee/MUL), k);

        }
        if (debug)
        {
            printf("(y/(1<<k))=%x, (x/(1<<k))=%x\n", (y/(1<<k)), (x/(1<<k)));
            printf("(y>>k=%x), (x>>k=%x)\n", y>>k, x>>k);
        }
        tx = (d^(mode==1)) ? add_with_overf_detect(x , (y>>k)) : add_with_overf_detect(x , (~(y>>k)+1)); // (d^(mode==1)) ? (x + (y/(1<<k))) : (x - (y/(1<<k)));
        if (debug)
        {
            int tx2 = (d^(mode==1)) ? (x + (y>>k)) : (x - (y>>k));
            printf("tx2=%x\n", tx2);
            printf("tx=%x\n", tx);
        }
        
        ty = d ? add_with_overf_detect(y, (x>>k)) : add_with_overf_detect(y , (~(x>>k)+1));//d ? (y + (x/(1<<k))) : (y - (x/(1<<k)));
        tz = d ? add_with_overf_detect(z , (~ee+1)) : add_with_overf_detect(z , ee);
        x = tx; y = ty; z = tz;
    }
    /********The algorithm ends*********/
    
    /********Output processing*********/
    if (debug)
        printf("*****Output processing*****\n");
        /*****circular mode*****/
    if (mode == 1)
    {
        // rotation mode, sin cos
        if (rot == 1)
        {
            
            const uint32_t xnte0_xkc = f_multiply(x, cordic_1K);
            const uint32_t ynte0_xkc = f_multiply(y, cordic_1K);
            const uint32_t xnte0_xkcp = ((xnte0_xkc >>POINT_POS) % 2) ? ~xnte0_xkc : xnte0_xkc;
            const uint32_t xnte0_xkcn = ((xnte0_xkc >>POINT_POS) % 2) ? xnte0_xkc : ~xnte0_xkc;
            const uint32_t ynte0_xkcp = ((ynte0_xkc >>POINT_POS) % 2) ? ~ynte0_xkc : ynte0_xkc;
            const uint32_t ynte0_xkcn = ((ynte0_xkc >>POINT_POS) % 2) ? ynte0_xkc : ~ynte0_xkc;
            
            const int cos_res_p = (!arg1_sign)&&(quadrant != 2) || (arg1_sign)&&(quadrant != 3);
            const int sin_res_p = !arg1_sign;
            const uint32_t xn_cos = cos_res_p ? xnte0_xkcp : xnte0_xkcn;
            const uint32_t yn_sin = sin_res_p ? ynte0_xkcp : ynte0_xkcn;
            if (debug)
            {
                printf("arg1_sign=%d\n", arg1_sign);
                printf("cos_res_p=%d\n", cos_res_p);
                printf("sin_res_p=%d\n", sin_res_p);
            }
            *arg1 = qxy_to_q131(xn_cos);
            *arg2 = qxy_to_q131(yn_sin);
            /*// quadrant 1, 4
            if (quadrant == 1)
            {
                if (debug)
                {
                    printf("quadrant 1, 4\n");
                }
                // *arg1 = x, *arg2 = y
                *arg1 = qxy_to_q131(x);
                *arg2 = qxy_to_q131(y);
            }
            // quadrant 3
            else if (quadrant == 3)
            {
                if (debug)
                {
                    printf("quadrant 2, 3\n");
                    printf("~x+1=%x, %.20f\n", ~x+1, (~x+1)/MUL);
                    printf("q131(~x+1)=%x, %.20f\n", qxy_to_q131(~x+1), qxy_to_q131(~x+1)/MUL131);
                }                
                // *arg1 = -x, *arg2 = y
                *arg1 = qxy_to_q131(~x+1);
                *arg2 = qxy_to_q131(~y+1);
            }
            else (quadrant == 2)
            {}*/
        }
        // vectoring mode, arctan
        else
        {   
            if (atan_mode == 0)
            {
                if(debug)
                    printf("atan mode\n");
                //*arg1 = qxy_to_q131(f_multiply(z, PI_REP));
                if (debug)
                    printf("*arg1=%f\n", *arg1/MUL131);
                const uint32_t zn_atan = f_multiply(z, PI_REP);
                const uint32_t zn_atan_p = ((zn_atan >>POINT_POS) % 2) ? ~zn_atan : zn_atan;
                const uint32_t zn_atan_n = ((zn_atan >>POINT_POS) % 2) ? zn_atan : ~zn_atan;
                const int atan_res_p = !arg1_sign;
                const uint32_t zn_atan_res = atan_res_p ? zn_atan_p : zn_atan_n;
                *arg1 = qxy_to_q131(zn_atan_res);
            }
            else
            {
                if(debug)
                    printf("atan2 mode\n");
                if (*arg1 == 0 && *arg2 == 0)
                    return;
                uint32_t zn_ph;
                if (*arg1>>31)
                {
                    if (*arg2>>31)
                    {
                        if (debug)
                            printf("quadrant 3\n");
                        zn_ph=f_multiply(z-PI, PI_REP);
                    }
                    else
                    {
                        if (debug)
                        {
                            printf("quadrant 2\n");
                        }
                        zn_ph=f_multiply(PI-z, PI_REP);
                    }
                }
                else
                {
                    if (*arg2>>31)
                    {
                        if (debug)
                            printf("quadrant 4\n");
                        zn_ph=f_multiply(~z+1, PI_REP);
                    }
                    else
                    {
                        if (debug)
                            printf("quadrant 1\n");
                        zn_ph=f_multiply(z, PI_REP);
                    }
                }
                const uint32_t zn_ph_p = ((zn_ph >>POINT_POS) % 2) ? ~zn_ph : zn_ph;
                const uint32_t zn_ph_n = ((zn_ph >>POINT_POS) % 2) ? zn_ph : ~zn_ph;
                const int phase_res_p = !arg2_sign;
                const uint32_t zn_phase = phase_res_p ? zn_ph_p : zn_ph_n;
                *arg1 = qxy_to_q131(zn_phase);
                if (debug)
                {
                    printf("zn_ph = %x, %f\n", zn_ph, zn_ph/MUL);  
                    printf("zn_ph_p = %x, %f\n", zn_ph_p, zn_ph_p/MUL);
                    printf("zn_ph_n = %x, %f\n", zn_ph_n, zn_ph_n/MUL);
                    printf("phase_res_p = %x, %f\n", phase_res_p, phase_res_p/MUL);
                    printf("zn_phase = %x, %f\n", zn_phase, zn_phase/MUL);
                    printf("*arg1  = %x, %f\n", *arg1, *arg1/MUL);
                }
                const uint32_t xnte0_xkc = f_multiply(x, cordic_1K);
                const uint32_t xnte0_xkcp = ((xnte0_xkc >>(BIT-1)) % 2) ? ~xnte0_xkc : xnte0_xkc;
                
                uint32_t xnte0_xkcp_exp = xnte0_xkcp;
                
                
                
                
                
                
                
                
                
                //xnte0_xkcp_exp = exp_en_4b ? ((unsigned)(xnte0_xkcp) >> 3) : xnte0_xkcp_exp;
                xnte0_xkcp_exp = exp_en_5b ? ((unsigned)(xnte0_xkcp) >> 4) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_6b ? ((unsigned)(xnte0_xkcp) >> 5) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_8b ? ((unsigned)(xnte0_xkcp) >> 7) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_9b ? ((unsigned)(xnte0_xkcp) >> 8) : xnte0_xkcp_exp;
                xnte0_xkcp_exp = exp_en_10b ? ((unsigned)(xnte0_xkcp) >> 9) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_11b ? ((unsigned)(xnte0_xkcp) >> 10) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_12b ? ((unsigned)(xnte0_xkcp) >> 11) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_13b ? ((unsigned)(xnte0_xkcp) >> 12) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_15b ? ((unsigned)(xnte0_xkcp) >> 14) : xnte0_xkcp_exp;
                xnte0_xkcp_exp = exp_en_16b ? ((unsigned)(xnte0_xkcp) >> 15) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_17b ? ((unsigned)(xnte0_xkcp) >> 16) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_18b ? ((unsigned)(xnte0_xkcp) >> 17) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_19b ? ((unsigned)(xnte0_xkcp) >> 18) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_20b ? ((unsigned)(xnte0_xkcp) >> 19) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_21b ? ((unsigned)(xnte0_xkcp) >> 20) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_22b ? ((unsigned)(xnte0_xkcp) >> 21) : xnte0_xkcp_exp;
                xnte0_xkcp_exp = exp_en_23b ? ((unsigned)(xnte0_xkcp) >> 22) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_24b ? ((unsigned)(xnte0_xkcp) >> 23) : xnte0_xkcp_exp;
                //xnte0_xkcp_exp = exp_en_26b ? ((unsigned)(xnte0_xkcp) >> 25) : xnte0_xkcp_exp;
                const uint32_t xn_modul = (xnte0_xkcp_exp >= one) ? one : xnte0_xkcp_exp;
                *arg2 = qxy_to_q131(xn_modul);
                if (debug)
                {
                    printf("xnte0_xkc = %x, %f\n", xnte0_xkc, xnte0_xkc/MUL);
                    printf("xnte0_xkcp = %x, %f\n", xnte0_xkcp, xnte0_xkcp/MUL);
                    printf("xnte0_xkcp_exp = %x, %f\n", xnte0_xkcp_exp, xnte0_xkcp_exp/MUL);
                    printf("xn_modul = %x, %f\n", xn_modul, xn_modul/MUL);
                    printf("*arg2  = %x, %f\n", *arg2, *arg2/MUL);
                }
            }
        }
    }
        /*****hyperbolic mode*****/
    else
    {
        if (debug)
            printf("Hyperbolic mode\n");
        const uint32_t xnte0_p = ((x >>POINT_POS) % 2) ? ~x : x;
        const uint32_t xnte0_n = ((x >>POINT_POS) % 2) ? x : ~x;
        const uint32_t ynte0_p = ((y >>POINT_POS) % 2) ? ~y : y;
        const uint32_t ynte0_n = ((y >>POINT_POS) % 2) ? y : ~y;
        const uint32_t znte0_p = ((z >>POINT_POS) % 2) ? ~z : z;
        const uint32_t znte0_n = ((z >>POINT_POS) % 2) ? z : ~z;
        if (rot == 1)
        {
            if (debug)
                printf("rotation mode\n");
            const uint32_t xn_cosh = xnte0_p;
            const uint32_t yn_sinh = arg1_sign ? ynte0_n : ynte0_p;
            *arg1 = qxy_to_q131(xn_cosh);
            *arg2 = qxy_to_q131(yn_sinh);
        }
        else
        {
            if (debug)
                printf("vectoring mode\n");
            if (atan_mode == 2)
            {
                if (debug)
                    printf("log mode\n");  
                const uint32_t zn_lnx = (n == 1 ? znte0_n : znte0_p);
                *arg1 = qxy_to_q131(zn_lnx);
            }
            else if (atan_mode == 1)
            {
                if (debug)
                {
                    printf("sqrt mode\n");
                    printf("x=%x\n", x);
                    printf("f_multiply(x, cordic_1K)=%x\n", f_multiply(x, cordic_1K)); 
                }
                const uint32_t xn_sqrt = f_multiply(x, cordic_1K);
                *arg1 = qxy_to_q131(xn_sqrt);
            }
            else
            {
                if (debug)
                    printf("arctanh mode\n");
                const uint32_t zn_atanh = arg1_sign ? znte0_n : znte0_p;
                *arg1 = qxy_to_q131(zn_atanh);
            }
        }
    }
}
