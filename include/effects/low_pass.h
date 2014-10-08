#ifndef __LOW_PASS_H__
#define __LOW_PASS_H__


#include "effects/effect_base.h"
#include "effects/resonance.h"
#include "engine/parameters.h"


class LowPass : public EffectBase {
public:
    LowPass(const Resonance* reso);
    virtual double translate_level(const double level) const;
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


private:
    const Resonance* m_p_resonance;
    double m_prev_result[NUM_CHANNELS];
    double m_prev_delta[NUM_CHANNELS];
};


#endif /* __LOW_PASS_H__ */

