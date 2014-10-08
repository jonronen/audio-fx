#include "effects/tremolo.h"
#include "engine/parameters.h"
#include "fx_math.h"


Tremolo::Tremolo()
    : EffectBase()
{
    set_level(0);
}


double Tremolo::process_sample(
        const double sample,
        const unsigned char channel)
{
    return sample * get_channel_level(channel);
}

double Tremolo::translate_lfo(double lfo) const
{
    return lfo + 128;
}

