#include "math.h"


int scaled_sine_8_bit(const unsigned int degrees)
{
    int tmp = (int)(degrees & 0x7f);
    unsigned int is_negative = (degrees & 0x80);

    tmp *= (0x80-tmp);
    tmp /= 32;

    return is_negative ? -tmp : tmp;
}

unsigned int scaled_shifted_sine(
    unsigned int min,
    unsigned int max,
    unsigned char phase
)
{
    unsigned int diff = max-min;
    long long int scaled_sin = scaled_sine_8_bit(phase);

    if ((diff == 0) || (diff > max)) return max;

    scaled_sin = scaled_sin * (long long int) diff / 2 / 0x100;
    scaled_sin += diff/2;

    return (unsigned int)scaled_sin + min;
}

