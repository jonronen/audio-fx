#include "effects/low_pass2.h"
#include "effects/reso2.h"
#include "engine/parameters.h"
#include "fx_math.h"


LowPass2::LowPass2(const Reso2* reso)
    : EffectBase(), m_p_resonance(reso)
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_result[i] = 0.0;
        m_prev_prev_result[i] = 0.0;
        m_prev_clean[i] = 0.0;
        m_prev_prev_clean[i] = 0.0;
    }
}


/*
 * translate the level from a linear level
 * to an effect-specific level.
 *
 * input: between zero and one
 * output: between zero and one, nonlinear
 */
double LowPass2::translate_level(const double level) const
{
    double tmp = 1.0 - level;
    return tmp*tmp;
}


double LowPass2::process_sample(
        const double sample,
        const unsigned char channel)
{
    //
    // B0 = m*m/2
    // B1 = m*m
    // A1 = -2+2*m*m
    // A2 = 1 - (2*m - m*m) * reso_factor
    // A0 = 2.0 - A2
    //
    // result =
    //     (B0*sample + B1*prev_clean + B0*prev_prev_clean
    //      - A1*prev_result - A2*prev_prev_result)
    //      / A0
    //

    double level = get_channel_level(channel);
    double level_sq = level * level;

    double reso_factor = (2.0 * level - level_sq) *
            m_p_resonance->get_channel_level(channel);

    // first, take care of the sample, prev_clean, and prev_prev_clean
    double res = (sample + m_prev_prev_clean[channel])/2.0;
    res += m_prev_clean[channel];
    res = res * level_sq;

    // now, add the prev_result
    res += (m_prev_result[channel] * 2.0 * (1.0 - level_sq));
    // and prev_prev_result
    res += (m_prev_prev_result[channel] * (reso_factor - 1.0));

    // and finally, take care of the division factor
    res = res / (1.0 + reso_factor);

    res = limit_value_of_sample(res);

    //
    // now it's time to shift the previous's
    //
    m_prev_prev_result[channel] = m_prev_result[channel];
    m_prev_result[channel] = res;
    m_prev_prev_clean[channel] = m_prev_clean[channel];
    m_prev_clean[channel] = sample;

    return res;
}

