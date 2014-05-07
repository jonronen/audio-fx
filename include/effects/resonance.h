#ifndef __RESONANCE_H__
#define __RESONANCE_H__


#include "effects/effect_base.h"
#include "effects/resonance.h"
#include "engine/parameters.h"


#define RESONANCE_MAX_LEVEL 0x100


class resonance_t : public effect_base_t {
    friend class low_pass_t;

public:
    resonance_t();
    virtual unsigned short translate_level(unsigned short level);
    virtual int process_sample(int sample, unsigned char channel);
};


#endif /* __RESONANCE_H__ */

