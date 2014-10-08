#include "effects/high_pass.h"
#include "engine/parameters.h"
#include "fx_math.h"


HighPass::HighPass()
    : EffectBase()
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_prev_clean[i] = 0.0;
        m_prev_result[i] = 0.0;
    }
    set_level(1);
}


double HighPass::translate_level(const double level) const
{
    return level*level*level;
}


double HighPass::process_sample(
        const double sample,
        const unsigned char channel)
{
    double saved_sample = sample;

    double ret = limit_value_of_sample(
        (m_prev_result[channel] + sample - m_prev_clean[channel]) *
        get_channel_level(channel)
    );
    m_prev_result[channel] = ret;
    m_prev_clean[channel] = saved_sample;

    return ret;
}

