#include "effects/high_pass.h"
#include "engine/parameters.h"
#include "fx_math.h"


high_pass_t::high_pass_t()
    : effect_base_t()
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_clean[i] = 0;
        m_prev_result[i] = 0;
    }
    set_level(HIGH_PASS_MAX_LEVEL);
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
        (int)get_channel_level(channel) / HIGH_PASS_MAX_LEVEL
    );
    m_prev_result[channel] = sample;
    m_prev_clean[channel] = saved_sample;

    return sample;
}

