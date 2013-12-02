#include "effects/low_pass.h"
#include "effects/resonance.h"
#include "engine/parameters.h"


low_pass_t::low_pass_t(const resonance_t* reso) : m_p_resonance(reso)
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = LOW_PASS_MAX_LEVEL;
    }
    set_levels(levels);
}


unsigned short low_pass_t::translate_level(unsigned short level)
{
    return two_exp_12bit_to_8bit(level);
}


int low_pass_t::process_sample(int sample, unsigned char channel)
{
    /* TODO */
}

