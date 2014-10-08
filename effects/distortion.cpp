#include "effects/distortion.h"
#include "engine/parameters.h"
#include "fx_math.h"


Distortion::Distortion()
    : EffectBase()
{
}


double Distortion::translate_level(const double level) const
{
    return level*2 / ((double)1.02 - level);
}

double Distortion::process_sample(
        const double sample,
        const unsigned char channel)
{
    double tmp = sample;

    tmp *= (get_channel_level(channel) + 1.0);
    tmp /= ((sample<0 ? -sample : sample) * get_channel_level(channel) + 1.0);

    return limit_value_of_sample(tmp);
}

