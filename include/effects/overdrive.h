#ifndef __OVERDRIVE_H__
#define __OVERDRIVE_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define OVERDRIVE_MAX_LEVEL 0x100
#define OVERDRIVE_NORMAL_LEVEL 0x40


class overdrive_t : public effect_base_t {
public:
    overdrive_t();
    unsigned short translate_level(unsigned short level);
    int process_sample(int sample, unsigned char channel);
};


#endif /* __OVERDRIVE_H__ */

