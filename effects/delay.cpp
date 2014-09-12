#include "effects/delay.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "utils/str.h"
#include "fx_math.h"
#include "serial.h"


delay_t::delay_t(bool with_feedback, unsigned int mn, unsigned int mx)
    : effect_base_t(), m_f_feedback(with_feedback)
{
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_lfo_freq[i] = 1;
        m_min_offset[i] = DELAY_MIN_MIN_OFFSET;
        if (mn >= DELAY_MIN_MIN_OFFSET) m_min_offset[i] = mn;
        m_max_offset[i] = DELAY_HISTORY_SIZE-1;
        if (mx < DELAY_HISTORY_SIZE) m_max_offset[i] = mx;

        m_lfo_phase[i] = 0;
        m_lfo_cnt[i] = 0;
    }
    set_level(0);

    memset(m_history, 0x00, sizeof(m_history));
    m_history_offset = 0;
}


void delay_t::set_pot_indices(unsigned char mix_index, unsigned char lfo_index)
{
    m_updating_params = true;
    m_pot_index = mix_index;
    m_lfo_index = lfo_index;
    m_updating_params = false;
}


void delay_t::set_lfo(unsigned short lfo)
{
    int i;
    unsigned short tmp = translate_lfo(lfo);

    m_updating_params = true;
    for (i=0; i<NUM_CHANNELS; i++) {
        m_lfo_freq[i] = tmp;
    }
    m_updating_params = false;
}


unsigned short delay_t::translate_level(unsigned short level) const
{
    return level/0x10;
}


void delay_t::params_update()
{
    int tmp;

    if ((m_param_ctrl == PARAM_CTRL_FIXED) ||
        (m_param_ctrl == PARAM_CTRL_EXTERNAL) ||
        (m_param_ctrl == PARAM_CTRL_METRONOME))
        return;

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

    if (m_lfo_index < MAX_LRADC_CHANNEL) {
        //serial_puts("pot #");
        //serial_puthex(m_lfo_index);

        tmp = lradc_read_channel(m_lfo_index);

        if (tmp != -1) {
            //serial_puts(": ");
            //serial_puthex(tmp);
            //serial_puts("\n");

            set_lfo(tmp);
        }
    }
}


int delay_t::process_sample(int sample, unsigned char channel)
{
    /* get the offset (in the history buffer) of the shifted feedback */
    int delay_offset = scaled_shifted_sine(
        m_min_offset[channel],
        m_max_offset[channel],
        m_lfo_phase[channel]
    );
    delay_offset = ((int)m_history_offset - delay_offset);
    if (delay_offset < 0) delay_offset += DELAY_HISTORY_SIZE;

    /* if we don't have feedback, this is the time to save the sample */
    if (!m_f_feedback) {
        /*
         * store the result in the history buffer
         * (this will be the shifted feedback in the future)
         */
        m_history[channel][m_history_offset] = sample;
    }

    /* linearly combine the current sample with the shifted feedback */
    sample = sample * (DELAY_MAX_LEVEL - get_channel_level(channel)) /
        DELAY_MAX_LEVEL;
    sample += m_history[channel][delay_offset] *
        get_channel_level(channel) / DELAY_MAX_LEVEL;

    /* if we do have feedback, save the sample after modifying it */
    if (m_f_feedback) {
        /*
         * store the result in the history buffer
         * (this will be the shifted feedback in the future)
         */
        m_history[channel][m_history_offset] = sample;
    }

    /* and increment the feedback stuff anyway */
    m_history_offset = ((m_history_offset + 1) % DELAY_HISTORY_SIZE);

    m_lfo_cnt[channel] += m_lfo_freq[channel];
    if (m_lfo_cnt[channel] >= TICK_FREQUENCY) m_lfo_phase[channel]++;

    return sample;
}

