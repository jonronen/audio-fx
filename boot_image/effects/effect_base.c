#include "effects/effect_base.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "serial.h"


void effect_base_t::set_ctrl(param_ctrl_t ctrl)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_param_ctrl = ctrl;
    /* TODO: based on the ctrl, set the level (MAX, etc.) */
    m_updating_params = false;
}

void effect_base_t::set_pot_index(unsigned char index)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_pot_index = index;
    m_updating_params = false;
}

void effect_base_t::set_level(unsigned short level)
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(level);
    }
    m_updating_params = false;
}

void effect_base_t::set_levels(unsigned short levels[NUM_CHANNELS])
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(levels[i]);
    }
    m_updating_params = false;
}

void effect_base_t::set_fixed_level(unsigned short level)
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_level(level);
    }
}

void effect_base_t::set_fixed_levels(unsigned short levels[NUM_CHANNELS])
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_levels(levels);
    }
}

unsigned short effect_base_t::get_channel_level(unsigned char channel) const
{
    return m_levels[channel];
}

unsigned short effect_base_t::translate_level(unsigned short level) const
{
    return level/0x10;
}

unsigned short effect_base_t::translate_lfo(unsigned short lfo) const
{
    return lfo;
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
    int tmp;

    if ((m_param_ctrl == PARAM_CTRL_FIXED) ||
        (m_param_ctrl == PARAM_CTRL_EXTERNAL) ||
        (m_param_ctrl == PARAM_CTRL_METRONOME))
        return;

    else if ((m_param_ctrl == PARAM_CTRL_MANUAL) ||
             (m_param_ctrl == PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL)) {
        if (m_pot_index != MAX_LRADC_CHANNEL) {
            //serial_puts("pot #");
            //serial_puthex(m_pot_index);
            tmp = lradc_read_channel(m_pot_index);
            if (tmp != -1) {
                //serial_puts(": ");
                //serial_puthex(tmp);
                //serial_puts("\n");
                set_level(tmp);
            }
            else {
                //serial_puts(" returned -1\n");
            }
        }
    }

    else if (m_param_ctrl == PARAM_CTRL_LFO) {
        if (m_pot_index == MAX_LRADC_CHANNEL) return;
        tmp = lradc_read_channel(m_pot_index);
        if (tmp == -1) return;
        m_lfo_freq = translate_lfo(tmp);
    }
}

void effect_base_t::params_tick()
{
    int j;

    if (m_param_ctrl == PARAM_CTRL_LFO) {
        for (j=0; j<NUM_CHANNELS; j++) {
            m_lfo_cnt[j] += m_lfo_freq;
            if (m_lfo_cnt[j] >= TICK_FREQUENCY) {
                m_lfo_cnt[j] -= TICK_FREQUENCY;
                m_lfo_phase[j]++;
                m_levels[j] = translate_level(phase_perform_op(m_lfo_op[j], m_lfo_phase[j]));
            }
        }
    }
}

void effect_base_t::metronome_phase(
    unsigned char phase_index,
    unsigned short op_index
)
{
    unsigned short level;

    if ((m_param_ctrl != PARAM_CTRL_METRONOME) &&
        (m_param_ctrl != PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL))
        return;

    /* get the raw level as an output from the metronome */
    unsigned short metronome_result =
        phase_perform_op(m_metronome_ops[op_index], phase_index);

    /*
     * the final level is a convex combination
     * of the metronome/manual level and the default level
     */
    if (m_param_ctrl == PARAM_CTRL_METRONOME) {
        level = m_metronome_levels[op_index];
    }
    else {
        level = m_levels[0]; // assuming all channels are at the same level...
    }
    metronome_result = (unsigned short)(
        (unsigned int)metronome_result * (unsigned int)level /
        EFFECT_MAX_LEVEL
    );
    set_level(metronome_result);
}


effect_base_t::effect_base_t()
{
    int i;

    m_param_ctrl = PARAM_CTRL_FIXED;
    m_pot_index = MAX_LRADC_CHANNEL;
    m_lfo_freq = 1;

    for (i=0; i<NUM_CHANNELS; i++) {
        m_lfo_op[i] = METRONOME_OP_CONST_FULL;
        m_lfo_cnt[i] = 0;
        m_lfo_phase[i] = 0;
    }

    for (i=0; i<MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS; i++) {
        m_metronome_ops[i] = METRONOME_OP_CONST_FULL;
        m_metronome_levels[i] = 0;
    }

    set_level(0);

    m_updating_params = false;
}

int effect_base_t::process_sample(int sample, unsigned char channel)
{
    return sample;
}

