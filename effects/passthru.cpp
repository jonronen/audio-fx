#include "effects/passthru.h"


PassThru::PassThru()
    : EffectBase()
{
}


double PassThru::process_sample(double sample, unsigned char channel)
{
    return sample;
}

