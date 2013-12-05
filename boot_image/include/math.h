#ifndef __MATH_H__
#define __MATH_H__


static inline int limit_value_of_sample(const int sample_value)
{
    if (sample_value <= -0x3fffff) return -0x3fffff;
    else if (sample_value >= 0x3fffff) return 0x3fffff;
    else return sample_value;
}

static inline int limit_value_of_delta(const int delta_value)
{
    if (delta_value <= -0x7fffff) return -0x7fffff;
    else if (delta_value >= 0x7fffff) return 0x7fffff;
    else return delta_value;
}

int scaled_sine_8_bit(const unsigned int degrees);
unsigned int scaled_shifted_sine(
    unsigned int min,
    unsigned int max,
    unsigned char phase
);


#endif /* __MATH_H__ */

