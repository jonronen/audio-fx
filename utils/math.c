#include "fx_math.h"


static int sine_8_bit(const unsigned char degrees)
{
    int tmp = (int)(degrees & 0x7f);
    unsigned char is_negative = (degrees & 0x80);

    tmp *= (0x80-tmp);

    return is_negative ? -tmp : tmp;
}

int scaled_sine_8_bit(const unsigned char degrees)
{
    return sine_8_bit(degrees) / 32;
}

unsigned short scaled_shifted_sine(
    unsigned short min,
    unsigned short max,
    unsigned char phase
)
{
    unsigned int diff = (int)max-(int)min;
    int scaled_sin = sine_8_bit(phase);

    // deal with overflows
    if ((diff == 0) || (diff > max)) return max;

    scaled_sin = scaled_sin * (int) diff / 64 / 0x100;
    scaled_sin += diff/2;

    return (unsigned short)scaled_sin + min;
}

