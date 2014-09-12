#ifndef __MATH_H__
#define __MATH_H__


#ifdef __cplusplus
extern "C" {
#endif


static inline double limit_value_of_sample(const double sample_value)
{
    if (sample_value <= -1.0) return -1.0;
    else if (sample_value >= 1.0) return 1.0;
    else return sample_value;
}

static inline double limit_value_of_delta(const double delta_value)
{
    if (delta_value <= -2.0) return -2.0;
    else if (delta_value >= 2.0) return 2.0;
    else return delta_value;
}

double scaled_sine(const double degrees);
double scaled_shifted_sine(
    double min,
    double max,
    double phase
);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __MATH_H__ */

