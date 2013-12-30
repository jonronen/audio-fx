#ifndef __DELAY_H__
#define __DELAY_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define DELAY_MIN_MIN_OFFSET 40
#define DELAY_HISTORY_SIZE 50000
#define DELAY_MAX_LEVEL 0x100
#define DELAY_MIN_LFO_FREQ 16
#define DELAY_MAX_LFO_FREQ 3*256


class delay_t : public effect_base_t {
public:
    delay_t(bool with_feedback, unsigned int mn, unsigned int mx);
    virtual int process_sample(int sample, unsigned char channel);
    void set_pot_indices(unsigned char mix_index, unsigned char lfo_index);
    void set_lfo(unsigned short lfo);
    virtual void params_update();


protected:
    virtual unsigned short translate_level(unsigned short level) const;
    unsigned int m_lfo_freq[NUM_CHANNELS];
    unsigned int m_max_offset[NUM_CHANNELS];
    unsigned int m_min_offset[NUM_CHANNELS];
    unsigned char m_lfo_index;

private:
    unsigned char m_lfo_phase[NUM_CHANNELS];
    unsigned int m_lfo_cnt[NUM_CHANNELS];
    int m_history[NUM_CHANNELS][DELAY_HISTORY_SIZE];
    unsigned short m_history_offset;
    bool m_f_feedback;
};


#endif /* __DELAY_H__ */

