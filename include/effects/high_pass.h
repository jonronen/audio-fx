#ifndef __HIGH_PASS_H__
#define __HIGH_PASS_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define HIGH_PASS_MAX_LEVEL 0x100


class high_pass_t : public effect_base_t {
public:
    high_pass_t();
    virtual unsigned short translate_level(unsigned short level);
    virtual int process_sample(int sample, unsigned char channel);


private:
    int m_prev_result[NUM_CHANNELS];
    int m_prev_clean[NUM_CHANNELS];
};


#endif /* __HIGH_PASS_H__ */

