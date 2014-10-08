#include "effects/passthru.h"


PassThru::PassThru()
    : EffectBase()
{
}


double PassThru::process_sample(
        const double sample,
        const unsigned char channel)
{
    return sample;
}

