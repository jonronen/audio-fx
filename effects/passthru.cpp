#include "effects/passthru.h"


pass_thru_t::pass_thru_t()
    : effect_base_t()
{
}


int pass_thru_t::process_sample(int sample, unsigned char channel)
{
    return sample;
}

