#ifndef __EFFECT_BASE_H__
#define __EFFECT_BASE_H__


#include "effects/parameters.h"
#include "effects/metronome.h"


class effect_base_t {
public:
    void set_ctrl(param_ctrl_t ctrl);
    void set_pot_index(unsigned char index);
    void set_fixed_level(unsigned short level);
    void set_metronome_ops(
        metronome_op_t ops[],
        unsigned short levels[],
        unsigned short cnt
    );

    void metronome_phase(unsigned char phase_index, unsigned short op_index);

    int process_sample(int sample, unsigned char channel);

    effect_base_t();
    virtual ~effect_base_t();

private:
    param_ctrl_t m_param_ctrl;
    unsigned char m_pot_index;
    unsigned short m_level[NUM_CHANNELS];

    metronome_op_t m_metronome_ops[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    unsigned short m_metronome_levels[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];

    bool m_updating_params;

    effect_base_t(const effect_base_t& other);
};



#endif /* __EFFECT_BASE_H__ */

