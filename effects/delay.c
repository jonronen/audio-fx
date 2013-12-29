#include "effects/delay.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "utils/str.h"
#include "math.h"
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


int delay_t::process_sample(int sample, unsigned char channel)
{
    /* get the offset (in the history buffer) of the shifted feedback */
    int delay_offset = 1000;
        /*scaled_shifted_sine(m_min_offset[channel], m_max_offset[channel], m_lfo_phase[channel]);*/
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
    sample = sample * get_channel_level(channel) / DELAY_MAX_LEVEL;
    sample += m_history[channel][delay_offset] *
        (DELAY_MAX_LEVEL - get_channel_level(channel)) / DELAY_MAX_LEVEL;

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

