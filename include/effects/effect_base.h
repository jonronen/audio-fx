#ifndef __EFFECT_BASE_H__
#define __EFFECT_BASE_H__


#include "engine/parameters.h"
#include "engine/metronome.h"



/*
 * our levels are doubles between 0 and 1.
 * zero means no effect at all,
 * 1 means full power.
 */


class EffectBase {
public:
    /*
     * methods that update the parameters
     */

    /* update at ease :) */
    virtual int params_update();
    /* update with a timer in mind */
    virtual int params_tick();
    /* update according to the metronome phase and operation */
    virtual int metronome_phase(
        const unsigned char phase_index,
        const unsigned short op_index
    );

    /*
     * the following methods are setup methods.
     * try not to call them too much, only when really changing the setup
     */
    virtual int set_ctrl(param_ctrl_t ctrl);
    virtual int set_pot_index(unsigned char index);
    virtual int set_metronome_ops(
        const metronome_op_t ops[],
        const double levels[],
        const unsigned short cnt
    );
    virtual int set_fixed_levels(const double levels[NUM_CHANNELS]);
    virtual int set_fixed_level(const double level);

    /* methods that use the parameters for modifying samples */
    virtual double process_sample(
        const double sample,
        const unsigned char channel);


protected:
    /* set levels per-channel. units are before translating: 12-bit */
    virtual int set_levels(const double levels[NUM_CHANNELS]);
    /* set levels for all channels. units are before translating: 12-bit */
    virtual int set_level(const double level);

    /*
     * translate levels from generic levels to effect-specific levels.
     *
     * input: a double between zero and one
     * output: effect-specific
     */
    virtual double translate_level(const double level) const;

    /*
     * translate lfo frequency to an actual frequency in Hz
     * (this is effect-specific as well, therefore virtual)
     */
    virtual double translate_lfo(const double lfo_level) const;

    /* get the translated channel level (for performing effects on it) */
    virtual double get_channel_level(const unsigned char channel) const;

    param_ctrl_t m_param_ctrl;
    unsigned short m_pot_index;
    double m_lfo_freq;
    bool m_updating_params;

    EffectBase();

private:
    metronome_op_t m_metronome_ops[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    double m_metronome_levels[MAX_DIVISION_FACTOR*MAX_PATTERN_UNITS];
    unsigned short m_metronome_op_cnt;

    double m_lfo_phase[NUM_CHANNELS];
    double m_lfo_cnt[NUM_CHANNELS];
    lfo_op_t m_lfo_op[NUM_CHANNELS];

    double m_levels[NUM_CHANNELS];

    EffectBase(const EffectBase& other);
};



/* allow up to 64 effects in parallel */
#define MAX_EFFECT_COUNT 16
#define MAX_PRESET_COUNT 8
extern EffectBase* g_effects[MAX_PRESET_COUNT][MAX_EFFECT_COUNT];
extern unsigned int g_preset_count;



#endif /* __EFFECT_BASE_H__ */

