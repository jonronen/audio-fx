#ifndef __DISTORTION_H__
#define __DISTORTION_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


class Distortion : public EffectBase {
public:
    Distortion();
    virtual double process_sample(
        const double sample,
        const unsigned char channel);
    virtual double translate_level(const double level) const;
};


#endif /* __DISTORTION_H__ */

