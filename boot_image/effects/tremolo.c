#include "effects/tremolo.h"
#include "engine/parameters.h"
#include "math.h"


tremolo_t::tremolo_t()
    : effect_base_t()
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = TREMOLO_MAX_LEVEL;

        m_phase[i] = 0;
    }
    set_levels(levels);
}


int tremolo_t::process_sample(int sample, unsigned char channel)
{
    return sample * m_levels[channel] / TREMOLO_MAX_LEVEL;
}

