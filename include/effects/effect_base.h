#ifndef __EFFECT_BASE_H__
#define __EFFECT_BASE_H__


#include "engine/parameters.h"
#include "engine/metronome.h"



/*
 * our levels are 12-bit unsigned integers
 * zero means no effect at all,
 * 0x1000 means full power
 */
#define EFFECT_MAX_LEVEL 0x1000


class effect_base_t {
public:
    /*
     * methods that update the parameters
     */

    /* update at ease :) */
    virtual void params_update();
    /* update with a timer in mind */
    virtual void params_tick();
    /* update according to the metronome phase and operation */
    virtual void metronome_phase(
        unsigned char phase_index,
        unsigned short op_index
    );

    /*
     * the following methods are setup methods.
     * try not to call them too much, only when really changing the setup
     */
    void set_ctrl(param_ctrl_t ctrl);
    void set_pot_index(unsigned char index);
    void set_metronome_ops(
        const metronome_op_t ops[],
        const unsigned short levels[],
        unsigned short cnt
    );
    void set_fixed_levels(const unsigned short levels[NUM_CHANNELS]);
    void set_fixed_level(unsigned short level);

    /* methods that use the parameters for modifying samples */
    virtual int process_sample(int sample, unsigned char channel);

protected:
    /* set levels per-channel. units are before translating: 12-bit */
    void set_levels(const unsigned short levels[NUM_CHANNELS]);
    /* set levels for all channels. units are before translating: 12-bit */
    void set_level(unsigned short level);

    /*
     * translate levels from generic levels to effect-specific levels.
     *
     * input: 12-bit unsigned integer (0-0x1000, zero means no effect, 0x1000 means full)
     * output: effect-specific
     */
    virtual unsigned short translate_level(unsigned short level) const;

    /*
     * translate lfo frequency from 12-bit to an actual frequency in Hz
     * (this is effect-specific as well, therefore virtual)
     */
    virtual unsigned short translate_lfo(unsigned short lfo_level) const;

    /* get the translated channel level (for performing effects on it) */
    unsigned short get_channel_level(unsigned char channel) const;

    param_ctrl_t m_param_ctrl;
    unsigned short m_pot_index;
    unsigned short m_lfo_freq;
    bool m_updating_params;

    effect_base_t();

private:
    metronome_op_t m_metronome_ops[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    unsigned short m_metronome_levels[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    unsigned short m_metronome_op_cnt;

    unsigned char m_lfo_phase[NUM_CHANNELS];
    unsigned short m_lfo_cnt[NUM_CHANNELS];
    metronome_op_t m_lfo_op[NUM_CHANNELS];

    unsigned short m_levels[NUM_CHANNELS];

    effect_base_t(const effect_base_t& other);
};



/* allow up to 64 effects in parallel */
#define MAX_EFFECT_COUNT 64
extern effect_base_t* g_effects[MAX_EFFECT_COUNT];



#endif /* __EFFECT_BASE_H__ */

