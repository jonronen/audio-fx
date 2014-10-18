#include "effects/reso2.h"
#include "engine/parameters.h"


Reso2::Reso2()
    : EffectBase()
{
    set_level(1.0);
}


double Reso2::process_sample(
        const double sample,
        const unsigned char channel)
{
    return sample;
}

double Reso2::translate_level(const double level) const
{
    double tmp = 1.0 - level;
    return 0.7 * tmp * tmp;
}

