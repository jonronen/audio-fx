#include "effects/flanger.h"
#include "engine/parameters.h"
#include "engine/metronome.h"
#include "utils/str.h"
#include "math.h"


flanger_t::flanger_t()
    : effect_base_t()
{
    unsigned short levels[NUM_CHANNELS];
    int i;
    for (i=0; i<NUM_CHANNELS; i++) {
        levels[i] = 0;

        m_lfo_freq[i] = 1;
        m_min_offset[i] = FLANGER_MIN_MIN_OFFSET;
        m_max_offset[i] = FLANGER_HISTORY_SIZE;

        m_lfo_phase[i] = 0;
        m_lfo_cnt[i] = 0;
    }
    set_levels(levels);

    memset(m_history, 0x00, sizeof(m_history));
    m_history_offset = 0;
}


int flanger_t::process_sample(int sample, unsigned char channel)
{
    /* get the offset (in the history buffer) of the shifted feedback */
    int feedback_offset =
        scaled_shifted_sine(m_min_offset[channel], m_max_offset[channel], m_lfo_phase[channel]);
    feedback_offset = ((int)m_history_offset - feedback_offset) % FLANGER_HISTORY_SIZE;

    /* linearly combine the current sample with the shifted feedback */
    sample = sample * m_levels[channel] / FLANGER_MAX_LEVEL;
    sample += m_history[channel][feedback_offset] *
        (FLANGER_MAX_LEVEL - m_levels[channel]) / FLANGER_MAX_LEVEL;

    /*
     * store the result in the history buffer
     * (this will be the shifted feedback in the future)
     */
    m_history[channel][m_history_offset] = sample;
    m_history_offset++;

    m_lfo_cnt[channel] += m_lfo_freq[channel];
    if (m_lfo_cnt[channel] >= TICK_FREQUENCY) m_lfo_phase[channel]++;

    return sample;
}

