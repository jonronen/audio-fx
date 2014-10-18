#ifndef __RESONANCE2_H__
#define __RESONANCE2_H__


#include "effects/effect_base.h"


class Reso2 : public EffectBase {
    friend class LowPass2;

public:
    Reso2();
    virtual double translate_level(const double level) const;
    virtual double process_sample(
        const double sample,
        const unsigned char channel);
};


#endif /* __RESONANCE2_H__ */

