#include "effects/overdrive.h"
#include "engine/parameters.h"
#include "fx_math.h"


overdrive_t::overdrive_t()
    : effect_base_t()
{
    set_level(OVERDRIVE_NORMAL_LEVEL);
}


unsigned short overdrive_t::translate_level(unsigned short level)
{
    return level < 257 ? 0x40 : 0x41 + (level-257)/20;
}


int overdrive_t::process_sample(int sample, unsigned char channel)
{
    return limit_value_of_sample(
        sample * (int)get_channel_level(channel) / OVERDRIVE_MAX_LEVEL
    );
}

