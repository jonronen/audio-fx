#include "effects/band_pass.h"
#include "engine/parameters.h"
#include "fx_math.h"
#include "stdint.h"


BandPass::BandPass()
    : EffectBase()
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_clean[i] = 0.0;
        m_low_pass[i] = 0.0;
        m_high_pass[i] = 0.0;
    }
    set_level(1.0);
}


double BandPass::process_sample(
        const double sample, const unsigned char channel)
{
    double alpha_level = get_channel_level(channel);
    double beta_level = alpha_level / 2;

    double tmp_val =
        (1.0-beta_level) *
        (1.0-beta_level);
    tmp_val *= (sample -
                m_prev_clean[channel] +
                m_high_pass[channel]);

    m_high_pass[channel] = limit_value_of_sample(tmp_val);

    m_prev_clean[channel] = sample;

    tmp_val = 1.0;
    tmp_val -= (alpha_level * alpha_level);
    tmp_val *= m_low_pass[channel];
    tmp_val += (alpha_level*alpha_level * m_high_pass[channel]);

    m_low_pass[channel] = limit_value_of_sample(tmp_val);
    return m_low_pass[channel];
}

