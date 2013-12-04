#include "effects/high_pass.h"
#include "engine/parameters.h"
#include "math.h"


high_pass_t::high_pass_t()
    : effect_base_t()
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = HIGH_PASS_MAX_LEVEL;

        m_prev_clean[i] = 0;
        m_prev_result[i] = 0;
    }
    set_levels(levels);
}


unsigned short high_pass_t::translate_level(unsigned short level)
{
    return two_exp_12bit_to_8bit(level);
}


int high_pass_t::process_sample(int sample, unsigned char channel)
{
    int saved_sample = sample;

    sample = limit_value_of_sample(
        (m_prev_result[channel] + sample - m_prev_clean[channel]) *
        (int)m_levels[channel] / HIGH_PASS_MAX_LEVEL
    );
    m_prev_result[channel] = sample;
    m_prev_clean[channel] = saved_sample;

    return sample;
}

