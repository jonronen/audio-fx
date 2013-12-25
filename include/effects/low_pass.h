#ifndef __LOW_PASS_H__
#define __LOW_PASS_H__


#include "effects/effect_base.h"
#include "effects/resonance.h"
#include "engine/parameters.h"


#define LOW_PASS_MAX_LEVEL 0x100


class low_pass_t : public effect_base_t {
public:
    low_pass_t(const resonance_t* reso);
    unsigned short translate_level(unsigned short level);
    int process_sample(int sample, unsigned char channel);


private:
    const resonance_t* m_p_resonance;
    int m_prev_result[NUM_CHANNELS];
    int m_prev_delta[NUM_CHANNELS];
};


#endif /* __LOW_PASS_H__ */

