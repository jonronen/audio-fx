#ifndef __BAND_PASS_H__
#define __BAND_PASS_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define BAND_PASS_MAX_LEVEL 0x100


class band_pass_t : public effect_base_t {
public:
    band_pass_t();
    virtual unsigned short translate_level(unsigned short level);
    virtual int process_sample(int sample, unsigned char channel);


private:
    int m_prev_clean[NUM_CHANNELS];
    int m_low_pass[NUM_CHANNELS];
    int m_high_pass[NUM_CHANNELS];
};


#endif /* __BAND_PASS_H__ */

