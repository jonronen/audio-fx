#include "fx_math.h"


double scaled_sine(const double scaled_degrees)
{
    unsigned char is_negative = (scaled_degrees < 0) ? 1 : 0;
    double tmp = is_negative ?
                 -scaled_degrees :
                 scaled_degrees;
    tmp -= (double)(int)tmp;

    tmp *= (1-tmp) * 4;

    return is_negative ? -tmp : tmp;
}

double scaled_shifted_sine(
    const double min,
    const double max,
    const double phase
)
{
    double half_diff = (max-min)/2;
    double scaled_sin = scaled_sine(phase);

    // deal with overflows
    if (half_diff <= 0) return max;

    return scaled_sin*half_diff + (min+max)/2;
}

