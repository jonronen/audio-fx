#ifndef __FLANGER_H__
#define __FLANGER_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define FLANGER_MIN_MIN_OFFSET 40
#define FLANGER_HISTORY_SIZE 512
#define FLANGER_MAX_LEVEL 0x100
#define FLANGER_MIN_LFO_FREQ 16
#define FLANGER_MAX_LFO_FREQ 3*256


class flanger_t : public effect_base_t {
public:
    flanger_t();
    int process_sample(int sample, unsigned char channel);


protected:
    unsigned int m_lfo_freq[NUM_CHANNELS];
    unsigned int m_max_offset[NUM_CHANNELS];
    unsigned int m_min_offset[NUM_CHANNELS];

private:
    unsigned char m_lfo_phase[NUM_CHANNELS];
    unsigned int m_lfo_cnt[NUM_CHANNELS];
    int m_history[NUM_CHANNELS][FLANGER_HISTORY_SIZE];
    unsigned short m_history_offset;
};


#endif /* __FLANGER_H__ */

