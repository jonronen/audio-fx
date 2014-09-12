#include "effects/low_pass.h"
#include "effects/resonance.h"
#include "engine/parameters.h"
#include "fx_math.h"


LowPass::LowPass(const Resonance* reso)
    : EffectBase(), m_p_resonance(reso)
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_result[i] = 0;
        m_prev_delta[i] = 0;
    }
}


/*
 * translate the level from a linear level
 * to an effect-specific level.
 *
 * input: between zero and one
 * output: between zero and one, nonlinear
 */
double LowPass::translate_level(double level)
{
    return level*level*level; /* TODO: work out on that one */
}


double LowPass::process_sample(double sample, unsigned char channel)
{
    m_prev_delta[channel] *= m_p_resonance->get_channel_level(channel);
    m_prev_delta[channel] +=
        (sample - m_prev_result[channel]) *
        get_channel_level(channel);
    m_prev_delta[channel] = limit_value_of_delta(m_prev_delta[channel]);

    sample = m_prev_result[channel] + m_prev_delta[channel];
    sample = limit_value_of_sample(sample);

    m_prev_result[channel] = sample;

    return sample;
}

