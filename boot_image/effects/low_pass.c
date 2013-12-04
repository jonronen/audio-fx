#include "effects/low_pass.h"
#include "effects/resonance.h"
#include "engine/parameters.h"
#include "math.h"


low_pass_t::low_pass_t(const resonance_t* reso)
    : effect_base_t(), m_p_resonance(reso)
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = LOW_PASS_MAX_LEVEL;

        m_prev_result[i] = 0;
        m_prev_delta[i] = 0;
    }
    set_levels(levels);
}


unsigned short low_pass_t::translate_level(unsigned short level)
{
    return two_exp_12bit_to_8bit(level);
}


int low_pass_t::process_sample(int sample, unsigned char channel)
{
    m_prev_delta[channel] *= m_p_resonance->m_levels[channel];
    m_prev_delta[channel] /= RESONANCE_MAX_LEVEL;
    m_prev_delta[channel] +=
        (((sample - m_prev_result[channel]) *
          (int)m_levels[channel]) / LOW_PASS_MAX_LEVEL);
    m_prev_delta[channel] = limit_value_of_delta(m_prev_delta[channel]);
    sample = limit_value_of_sample(
        m_prev_result[channel] + m_prev_delta[channel]
    );
    m_prev_result[channel] = sample;

    return sample;
}

