#ifndef __DELAY_H__
#define __DELAY_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define DELAY_MIN_MIN_OFFSET 40
#define DELAY_HISTORY_SIZE 50000
#define DELAY_MIN_LFO_FREQ 16
#define DELAY_MAX_LFO_FREQ 3*256


class Delay : public EffectBase {
public:
    Delay(bool with_feedback, unsigned int mn, unsigned int mx);
    virtual double process_sample(
        const double sample,
        const unsigned char channel);
    int set_pot_indices(
            const unsigned char mix_index,
            const unsigned char lfo_index);
    int set_lfo(const double lfo);
    virtual void params_update();


protected:
    double m_lfo_increment[NUM_CHANNELS];
    unsigned short m_max_offset[NUM_CHANNELS];
    unsigned short m_min_offset[NUM_CHANNELS];
    unsigned short m_lfo_index;

private:
    double m_lfo_phase[NUM_CHANNELS];
    double m_history[NUM_CHANNELS][DELAY_HISTORY_SIZE];
    unsigned short m_history_offset;
    bool m_f_feedback;
};


#endif /* __DELAY_H__ */

