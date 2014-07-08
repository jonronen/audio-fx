#include "effects/distortion.h"
#include "engine/parameters.h"
#include "fx_math.h"


distortion_t::distortion_t()
    : effect_base_t()
{
}


unsigned short distortion_t::translate_level(unsigned short level)
{
    // translate from 12-bit to 17-bit
    m_dist_level = (double)level / (double)(2080 - level/2);

    return 0;
}

int distortion_t::process_sample(int sample, unsigned char channel)
{
    double tmp = (double)sample;

    tmp *= (m_dist_level + 1.0);
    tmp /= ((double)sample * m_dist_level / (double)(int)(1<<22) + 1.0);

    return limit_value_of_sample((int)tmp);
}

