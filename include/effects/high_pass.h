#ifndef __HIGH_PASS_H__
#define __HIGH_PASS_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define HIGH_PASS_MAX_LEVEL 0x100


class HighPass : public EffectBase {
public:
    HighPass();
    virtual double translate_level(const double level) const;
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


private:
    double m_prev_result[NUM_CHANNELS];
    double m_prev_clean[NUM_CHANNELS];
};


#endif /* __HIGH_PASS_H__ */

