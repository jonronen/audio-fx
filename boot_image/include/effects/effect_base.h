#ifndef __EFFECT_BASE_H__
#define __EFFECT_BASE_H__


#include "engine/parameters.h"
#include "engine/metronome.h"


class effect_base_t {
public:
    /* methods that update the parameters */
    virtual void params_update();
    virtual void metronome_phase(
        unsigned char phase_index,
        unsigned short op_index
    );
    virtual unsigned short translate_level(unsigned short level);

    /* methods that use the parameters for modifying samples */
    virtual int process_sample(int sample, unsigned char channel);

protected:
    void set_ctrl(param_ctrl_t ctrl);
    void set_pot_index(unsigned char index);
    void set_levels(unsigned short levels[NUM_CHANNELS]);
    void set_metronome_ops(
        metronome_op_t ops[],
        unsigned short levels[],
        unsigned short cnt
    );
    effect_base_t();

    unsigned short m_levels[NUM_CHANNELS];

private:
    param_ctrl_t m_param_ctrl;
    unsigned short m_pot_index;

    metronome_op_t m_metronome_ops[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    unsigned short m_metronome_levels[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];

    bool m_updating_params;

    effect_base_t(const effect_base_t& other);
};



#endif /* __EFFECT_BASE_H__ */

