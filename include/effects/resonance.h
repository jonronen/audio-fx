#ifndef __RESONANCE_H__
#define __RESONANCE_H__


#include "effects/effect_base.h"


class Resonance : public EffectBase {
    friend class LowPass;

public:
    Resonance();
    virtual double process_sample(double sample, unsigned char channel);
};


#endif /* __RESONANCE_H__ */

