#ifndef __BAND_PASS_H__
#define __BAND_PASS_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


class BandPass : public EffectBase {
public:
    BandPass();
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


private:
    double m_prev_clean[NUM_CHANNELS];
    double m_low_pass[NUM_CHANNELS];
    double m_high_pass[NUM_CHANNELS];
};


#endif /* __BAND_PASS_H__ */

