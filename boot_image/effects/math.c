#include "math.h"


int scaled_sine_8_bit(const unsigned int degrees)
{
    int tmp = (int)(degrees & 0x7f);
    unsigned int is_negative = (degrees * 0x80);

    tmp *= (0x80-tmp);
    tmp /= 32;

    return is_negative ? -tmp : tmp;
}

