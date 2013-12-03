#include "effects/resonance.h"
#include "engine/parameters.h"


resonance_t::resonance_t()
    : effect_base_t()
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = RESONANCE_MAX_LEVEL;
    }
    set_levels(levels);
}


unsigned short resonance_t::translate_level(unsigned short level)
{
    return level;
}


int resonance_t::process_sample(int sample, unsigned char channel)
{
    return sample;
}

