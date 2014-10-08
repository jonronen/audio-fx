#ifndef __TREMOLO_H__
#define __TREMOLO_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


class Tremolo : public EffectBase {
public:
    Tremolo();
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


private:
    virtual double translate_lfo(double lfo_level) const;
};


#endif /* __TREMOLO_H__ */

