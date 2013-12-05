#include "effects/resonance.h"
#include "engine/parameters.h"


resonance_t::resonance_t()
    : effect_base_t()
{
    set_level(RESONANCE_MAX_LEVEL);
}


unsigned short resonance_t::translate_level(unsigned short level)
{
    return level;
}


int resonance_t::process_sample(int sample, unsigned char channel)
{
    return sample;
}

