#include "effects/effect_base.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "serial.h"


void EffectBase::set_ctrl(param_ctrl_t ctrl)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_param_ctrl = ctrl;
    /* TODO: based on the ctrl, set the level (MAX, etc.) */
    m_updating_params = false;
}

void EffectBase::set_pot_index(unsigned char index)
{
    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    m_pot_index = index;
    m_updating_params = false;
}

void EffectBase::set_level(double level)
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(level);
    }
    m_updating_params = false;
}

void EffectBase::set_levels(const double levels[NUM_CHANNELS])
{
    int i;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_levels[i] = translate_level(levels[i]);
    }
    m_updating_params = false;
}

void EffectBase::set_fixed_level(const double level)
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_level(level);
    }

    /* TODO: else return an error */
}

void EffectBase::set_fixed_levels(const double levels[NUM_CHANNELS])
{
    if (m_param_ctrl == PARAM_CTRL_FIXED) {
        set_levels(levels);
    }
}

double EffectBase::get_channel_level(unsigned char channel) const
{
    return m_levels[channel];
}

void EffectBase::set_metronome_ops(
    const metronome_op_t ops[],
    const double levels[],
    unsigned short cnt
)
{
    int i;

    // sanity check
    if (cnt > MAX_DIVISION_FACTOR * MAX_PATTERN_UNITS) return;

    /* TODO: lock a mutex? disable interrupts? */
    m_updating_params = true;
    for (i=0; i<cnt; i++) {
        m_metronome_ops[i] = ops[i];
        m_metronome_levels[i] = levels[i];
    }
    m_metronome_op_cnt = cnt;
    m_updating_params = false;
}


/* methods that update the parameters */
void EffectBase::params_update()
{
    double tmp;

    if ((m_param_ctrl == PARAM_CTRL_FIXED) ||
        (m_param_ctrl == PARAM_CTRL_EXTERNAL) ||
        (m_param_ctrl == PARAM_CTRL_METRONOME))
        return;

    else if ((m_param_ctrl == PARAM_CTRL_MANUAL) ||
             (m_param_ctrl == PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL)) {
        if (m_pot_index < MAX_LRADC_CHANNEL) {
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
        if (m_pot_index >= MAX_LRADC_CHANNEL) return;

        //serial_puts("pot #");
        //serial_puthex(m_pot_index);

        tmp = lradc_read_channel(m_pot_index);

        if (tmp == -1) return;

        //serial_puts(": ");
        //serial_puthex(tmp);
        //serial_puts("\n");

        m_lfo_freq = translate_lfo(tmp);
    }
}

void EffectBase::params_tick()
{
    int j;

    if (m_param_ctrl == PARAM_CTRL_LFO) {
        for (j=0; j<NUM_CHANNELS; j++) {
            m_lfo_cnt[j] += m_lfo_freq;
            if (m_lfo_cnt[j] >= TICK_FREQUENCY) {
                m_lfo_cnt[j] -= TICK_FREQUENCY;
                m_lfo_phase[j]++;
                m_levels[j] = translate_level(
                    phase_perform_op(
                        m_lfo_op[j],
                        m_lfo_phase[j],
                        1,
                        1
                    )
                );
            }
        }
    }
}

void EffectBase::metronome_phase(
    unsigned char phase_index,
    unsigned short op_index
)
{
    double curr_level, next_level;

    if ((m_param_ctrl != PARAM_CTRL_METRONOME) &&
        (m_param_ctrl != PARAM_CTRL_METRONOME_WITH_MANUAL_LEVEL))
        return;

    /*
     * the final level is a convex combination
     * of the metronome/manual level and the default level
     */
    if (m_param_ctrl == PARAM_CTRL_METRONOME) {
        curr_level = m_metronome_levels[op_index];
        next_level = m_metronome_levels[
            op_index+1 >= m_metronome_op_cnt ? 0 : op_index+1
        ];
    }
    else {
        // assuming all channels are at the same level...
        curr_level = m_levels[0];
        next_level = m_levels[0];
    }

    /* get the raw level as an output from the metronome */
    double metronome_result =
        phase_perform_op(
            m_metronome_ops[op_index],
            phase_index,
            curr_level,
            next_level
        );

    set_level(metronome_result);
}


EffectBase::EffectBase()
{
    int i;

    m_param_ctrl = PARAM_CTRL_FIXED;
    m_pot_index = MAX_LRADC_CHANNEL;
    m_lfo_freq = 1;

    for (i=0; i<NUM_CHANNELS; i++) {
        /* TODO: change this to something less rude */
        m_lfo_op[i] = METRONOME_OP_LINEAR_FALL;
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

/* default implementations, just to be ok with pass-thru's and stuff */
double EffectBase::translate_level(const double level) const
{
    return level;
}

double EffectBase::translate_lfo(const double lfo) const
{
    return lfo;
}

double EffectBase::process_sample(
    const double sample,
    const unsigned char channel)
{
    return sample;
}

