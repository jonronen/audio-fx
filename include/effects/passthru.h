#ifndef __PASSTHRU_H__
#define __PASSTHRU_H__


#include "effects/effect_base.h"


class PassThru : public EffectBase {
public:
    PassThru();
    virtual double process_sample(
        const double sample,
        const unsigned char channel);
};


#endif /* __PASSTHRU_H__ */

