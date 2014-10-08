#ifndef __OVERDRIVE_H__
#define __OVERDRIVE_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


class Overdrive : public EffectBase {
public:
    Overdrive();
    virtual double translate_level(const double level) const;
    virtual double process_sample(
        const double sample,
        const unsigned char channel);
};


#endif /* __OVERDRIVE_H__ */

