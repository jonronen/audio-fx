#include "effects/overdrive.h"
#include "engine/parameters.h"
#include "fx_math.h"


Overdrive::Overdrive()
    : EffectBase()
{
    set_level(0);
}


double Overdrive::translate_level(const double level) const
{
    return 0.25 + (level-0.25)*300;
}


double Overdrive::process_sample(
        const double sample,
        const unsigned char channel)
{
    return limit_value_of_sample(
        sample * get_channel_level(channel)
    );
}

