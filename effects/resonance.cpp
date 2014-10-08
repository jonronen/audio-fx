#include "effects/resonance.h"
#include "engine/parameters.h"


Resonance::Resonance()
    : EffectBase()
{
    set_level(1.0);
}


double Resonance::process_sample(
        const double sample,
        const unsigned char channel)
{
    return sample;
}

