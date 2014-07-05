#include "effects/band_pass.h"
#include "engine/parameters.h"
#include "fx_math.h"
#include "stdint.h"


band_pass_t::band_pass_t()
    : effect_base_t()
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_clean[i] = 0;
        m_low_pass[i] = 0;
        m_high_pass[i] = 0;
    }
    set_level(BAND_PASS_MAX_LEVEL);
}


unsigned short band_pass_t::translate_level(unsigned short level)
{
    return level/16;
}


int band_pass_t::process_sample(int sample, unsigned char channel)
{
    int alpha_level = (int)get_channel_level(channel);
    int beta_level = alpha_level / 2;

    int64_t tmp_val =
        (int64_t)(BAND_PASS_MAX_LEVEL-beta_level) *
        (BAND_PASS_MAX_LEVEL-beta_level);
    tmp_val *= ((int64_t)sample -
                m_prev_clean[channel] +
                m_high_pass[channel]);
    tmp_val /= (BAND_PASS_MAX_LEVEL * BAND_PASS_MAX_LEVEL);

    m_high_pass[channel] = limit_value_of_sample((int)tmp_val);

    m_prev_clean[channel] = sample;

    tmp_val = (int64_t)BAND_PASS_MAX_LEVEL*BAND_PASS_MAX_LEVEL;
    tmp_val -= ((int64_t)alpha_level * alpha_level);
    tmp_val *= m_low_pass[channel];
    tmp_val += ((int64_t)alpha_level*alpha_level * m_high_pass[channel]);
    tmp_val /= (BAND_PASS_MAX_LEVEL * BAND_PASS_MAX_LEVEL);

    m_low_pass[channel] = limit_value_of_sample((int)tmp_val);
    return m_low_pass[channel];
}

