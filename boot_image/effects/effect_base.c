#include "effects/effect_base.h"
#include "effects/parameters.h"
#include "effects/metronome.h"
#include "lradc.h"


void effect_base_t::set_ctrl(param_ctrl_t ctrl)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_param_ctrl = ctrl;
    m_updating_params = false;
}

void effect_base_t::set_pot_index(unsigned char index)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_pot_index = index;
    m_updating_params = false;
}

void effect_base_t::set_levels(unsigned short levels[NUM_CHANNELS])
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = levels[i];
    }
    m_updating_params = false;
}

void effect_base_t::set_metronome_ops(
    metronome_op_t ops[],
    unsigned short levels[],
    unsigned short cnt
)
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<cnt; i++) {
        m_metronome_ops[i] = ops[i];
        m_metronome_levels[i] = levels[i];
    }
    m_updating_params = false;
}


/* methods that update the parameters */
void effect_base_t::params_update()
{
    unsigned short tmps[NUM_CHANNELS];
    int tmp;
    int j;

    if ((m_param_ctrl == PARAM_CTRL_FIXED) ||
        (m_param_ctrl == PARAM_CTRL_EXTERNAL) ||
        (m_param_ctrl == PARAM_CTRL_METRONOME))
        return;

    else if ((m_param_ctrl == PARAM_CTRL_MANUAL) ||
             (m_param_ctrl == PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL)) {
        if (m_pot_index != MAX_LRADC_CHANNEL) {
            tmp = lradc_read_channel(m_pot_index);
            if (tmp != -1) {
                for (j=0; j<NUM_CHANNELS; j++) tmps[j] = (unsigned short)tmp;
                set_levels(tmps);
            }
        }
    }

    else if (m_param_ctrl == PARAM_CTRL_LFO) {
        /* TODO: LFO */
    }
}

void effect_base_t::metronome_phase(
    unsigned char phase_index,
    unsigned short op_index
)
{
    /* TODO: metronome */
}


effect_base_t::effect_base_t()
{
    int i;

    m_param_ctrl = PARAM_CTRL_FIXED;
    m_pot_index = MAX_LRADC_CHANNEL;

    for (i=0; i<MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS; i++) {
        m_metronome_ops[i] = METRONOME_OP_CONST_FULL;
    }

    m_updating_params = false;
}

int effect_base_t::process_sample(int sample, unsigned char channel)
{
    return sample;
}

