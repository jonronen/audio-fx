#include "effects/tremolo.h"
#include "engine/parameters.h"
#include "fx_math.h"


tremolo_t::tremolo_t()
    : effect_base_t()
{
    set_level(TREMOLO_MAX_LEVEL);
}


int tremolo_t::process_sample(int sample, unsigned char channel)
{
    return sample * get_channel_level(channel) / TREMOLO_MAX_LEVEL;
}

unsigned short tremolo_t::translate_lfo(unsigned short lfo) const
{
    return lfo + 128;
}

