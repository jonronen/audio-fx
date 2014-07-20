#ifndef __RESONANCE_H__
#define __RESONANCE_H__


#include "effects/effect_base.h"


#define RESONANCE_MAX_LEVEL 0x100


class resonance_t : public effect_base_t {
    friend class low_pass_t;

public:
    resonance_t();
    virtual int process_sample(int sample, unsigned char channel);
};


#endif /* __RESONANCE_H__ */

