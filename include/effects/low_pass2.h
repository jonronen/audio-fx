#ifndef __LOW_PASS2_H__
#define __LOW_PASS2_H__


#include "effects/effect_base.h"
#include "effects/reso2.h"
#include "engine/parameters.h"


class LowPass2 : public EffectBase {
public:
    LowPass2(const Reso2* reso);
    virtual double translate_level(const double level) const;
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


private:
    const Reso2* m_p_resonance;
    double m_prev_result[NUM_CHANNELS];
    double m_prev_prev_result[NUM_CHANNELS];
    double m_prev_clean[NUM_CHANNELS];
    double m_prev_prev_clean[NUM_CHANNELS];
};


#endif /* __LOW_PASS2_H__ */

