#include "effects/effect_base.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "serial.h"


int EffectBase::set_ctrl(const param_ctrl_t ctrl)
{
    if (ctrl > PARAM_CTRL_MAX) {
        serial_puts("set_ctrl: wrong parameter 0x");
        serial_puthex(ctrl);
        serial_puts("\n");
        return -1;
    }

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_param_ctrl = ctrl;
    /* TODO: based on the ctrl, set the level (MAX, etc.) */
    m_updating_params = false;

    return 0;
}

int EffectBase::set_pot_index(const unsigned char index)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_pot_index = index;
    m_updating_params = false;

    return 0;
}

int EffectBase::set_level(const double level)
{
    int i;

    if ((level < 0.0) || (level > 1.0)) {
        serial_puts("set_level: wrong level\n");
        return -1;
    }

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(level);
    }
    m_updating_params = false;

    return 0;
}

int EffectBase::set_levels(const double levels[NUM_CHANNELS])
{
    int i;

    for (i=0; i<NUM_CHANNELS; i++) {
        if ((levels[i] < 0.0) || (levels[i] > 1.0)) {
            serial_puts("set_levels: wrong level for channel 0x");
            serial_puthex(i);
            serial_puts("\n");
            return -1;
        }
    }

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(levels[i]);
    }
    m_updating_params = false;

    return 0;
}

int EffectBase::set_fixed_level(const double level)
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_level(level);
        return 0;
    }

    serial_puts("set_fixed_level: ctrl is not FIXED\n");
    return -1;
}

int EffectBase::set_fixed_levels(const double levels[NUM_CHANNELS])
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_levels(levels);
        return 0;
    }

    serial_puts("set_fixed_levels: ctrl is not FIXED\n");
    return -1;
}

double EffectBase::get_channel_level(unsigned char channel) const
{
    return m_levels[channel];
}

int EffectBase::set_metronome_ops(
    const metronome_op_t ops[],
    const double levels[],
    const unsigned short cnt
)
{
    int i;

    // sanity check
    if (cnt > MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS) {
        serial_puts("set_metronome_ops: too many ops\n");
        return -1;
    }

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<cnt; i++) {
        m_metronome_ops[i] = ops[i];
        m_metronome_levels[i] = levels[i];
    }
    m_metronome_op_cnt = cnt;
    m_updating_params = false;

    return 0;
}


/* methods that update the parameters */
int EffectBase::params_update()
{
    double tmp;

    if ((m_param_ctrl == PARAM_CTRL_FIXED) ||
        (m_param_ctrl == PARAM_CTRL_EXTERNAL) ||
        (m_param_ctrl == PARAM_CTRL_METRONOME))
        return 0;

    else if (m_param_ctrl == PARAM_CTRL_MANUAL) {
        if (m_pot_index < MAX_LRADC_CHANNEL) {
            tmp = lradc_read_channel(m_pot_index);
            if (tmp != LRADC_INVALID_VALUE) {
                return set_level(tmp);
            }
            else {
                serial_puts("params_update: cannot read lradc channel 0x");
                serial_puthex(m_pot_index);
                serial_puts("\n");
                return -1;
            }
        }
    }

    else if (m_param_ctrl == PARAM_CTRL_LFO) {
        if (m_pot_index >= MAX_LRADC_CHANNEL) {
            serial_puts("params_update: wrong pot index 0x");
            serial_puthex(m_pot_index);
            serial_puts("\n");
            return -1;
        }

        tmp = lradc_read_channel(m_pot_index);

        if (tmp == LRADC_INVALID_VALUE) {
            serial_puts("params_update: cannot read lradc channel 0x");
            serial_puthex(m_pot_index);
            serial_puts("\n");
            return -1;
        }

        m_lfo_increment = translate_lfo(tmp);
    }

    return 0;
}

int EffectBase::params_tick()
{
    int j;

    if (m_param_ctrl == PARAM_CTRL_LFO) {
        for (j=0; j<NUM_CHANNELS; j++) {
            m_lfo_phase[j] += m_lfo_increment;
            if (m_lfo_phase[j] >= 2.0) {
                m_lfo_phase[j] -= 2.0;
            }
            m_levels[j] = translate_level(
                lfo_perform_op(
                    m_lfo_op[j],
                    m_lfo_phase[j]
                )
            );
        }
    }
    return 0;
}

int EffectBase::metronome_phase(
    const double phase,
    const unsigned short op_index
)
{
    double curr_level, next_level;

    if (m_param_ctrl != PARAM_CTRL_METRONOME) {
        // this is not an error, we just don't need to do anything
        return 0;
    }

    /*
     * the final level is a convex combination
     * of the metronome level and the next/default
     */
    curr_level = m_metronome_levels[op_index];
    next_level = m_metronome_levels[
        (op_index+1 >= m_metronome_op_cnt) ? 0 : (op_index+1)
    ];

    /* get the raw level as an output from the metronome */
    double metronome_result =
        phase_perform_op(
            m_metronome_ops[op_index],
            phase,
            curr_level,
            next_level
        );

    set_level(metronome_result);

    return 0;
}


EffectBase::EffectBase()
{
    int i;

    m_param_ctrl = PARAM_CTRL_FIXED;
    m_pot_index = MAX_LRADC_CHANNEL;
    m_lfo_increment = 0;

    for (i=0; i<NUM_CHANNELS; i++) {
        /* TODO: change this to something less rude */
        m_lfo_op[i] = LFO_OP_SINE;
        m_lfo_phase[i] = 0;
    }

    for (i=0; i<MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS; i++) {
        m_metronome_ops[i] = METRONOME_OP_CONST_FULL;
        m_metronome_levels[i] = 0;
    }

    set_level(0);

    m_updating_params = false;
}

/* default implementations, just to be ok with pass-thru's and stuff */
double EffectBase::translate_level(const double level) const
{
    return level;
}

double EffectBase::translate_lfo(const double lfo) const
{
    static const double MIN_LFO_FREQ = 0.3;
    static const double MAX_LFO_FREQ = 16.0;

    // LFO should be between m_min_lfo_freq and m_max_lfo_freq
    double tmp_lfo = MIN_LFO_FREQ;
    tmp_lfo += (lfo * (MAX_LFO_FREQ - MIN_LFO_FREQ));

    return tmp_lfo/TICK_FREQUENCY;
}

