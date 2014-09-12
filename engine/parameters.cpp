#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "stdint.h"
#include "stddef.h"
#include "serial.h"
#include "utils/str.h"

/* effects */
#include "effects/effect_base.h"
#include "effects/low_pass.h"
//#include "effects/high_pass.h"
//#include "effects/band_pass.h"
//#include "effects/tremolo.h"
//#include "effects/overdrive.h"
//#include "effects/distortion.h"
//#include "effects/delay.h"
#include "effects/resonance.h"
#include "effects/passthru.h"


EffectBase* g_effects[MAX_PRESET_COUNT][MAX_EFFECT_COUNT];
unsigned int g_preset_count;

static PassThru g_passthru;
static Resonance g_reso0;
static LowPass  g_low_pass0(&g_reso0);
//static tremolo_t   g_trem;
//static delay_t     g_reverb(true, 30000, 30000);
//static distortion_t g_dist;


/* TODO: work out on the phase transitions */
#ifdef NOT_DEFINED
static const unsigned short g_sine_phase[] = {
     0x0000, 0x0030, 0x0060, 0x008f,  0x00be, 0x00ec, 0x011a, 0x0147,
     0x0175, 0x01a1, 0x01ce, 0x01fa,  0x0226, 0x0251, 0x027c, 0x02a7,
     0x02d1, 0x02fc, 0x0325, 0x034e,  0x0377, 0x03a0, 0x03c8, 0x03f0,
     0x0418, 0x043f, 0x0466, 0x048d,  0x04b3, 0x04d9, 0x04fe, 0x0524,
     0x0548, 0x056d, 0x0591, 0x05b5,  0x05d9, 0x05fc, 0x061f, 0x0642,
     0x0664, 0x0686, 0x06a8, 0x06c9,  0x06ea, 0x070b, 0x072c, 0x074c,
     0x076b, 0x078b, 0x07aa, 0x07c9,  0x07e8, 0x0806, 0x0824, 0x0842,
     0x085f, 0x087d, 0x0899, 0x08b6,  0x08d2, 0x08ee, 0x090a, 0x0925,
     0x0940, 0x095b, 0x0976, 0x0990,  0x09aa, 0x09c4, 0x09dd, 0x09f7,
     0x0a10, 0x0a28, 0x0a41, 0x0a59,  0x0a71, 0x0a88, 0x0aa0, 0x0ab7,
     0x0acd, 0x0ae4, 0x0afa, 0x0b10,  0x0b26, 0x0b3c, 0x0b51, 0x0b66,
     0x0b7b, 0x0b8f, 0x0ba4, 0x0bb8,  0x0bcc, 0x0bdf, 0x0bf3, 0x0c06,
     0x0c18, 0x0c2b, 0x0c3e, 0x0c50,  0x0c62, 0x0c73, 0x0c85, 0x0c96,
     0x0ca7, 0x0cb8, 0x0cc9, 0x0cd9,  0x0ce9, 0x0cf9, 0x0d09, 0x0d18,
     0x0d27, 0x0d37, 0x0d45, 0x0d54,  0x0d63, 0x0d71, 0x0d7f, 0x0d8d,
     0x0d9a, 0x0da8, 0x0db5, 0x0dc2,  0x0dcf, 0x0ddc, 0x0de8, 0x0df4,
     0x0e00, 0x0e0c, 0x0e18, 0x0e24,  0x0e2f, 0x0e3a, 0x0e45, 0x0e50,
     0x0e5b, 0x0e65, 0x0e6f, 0x0e79,  0x0e83, 0x0e8d, 0x0e97, 0x0ea0,
     0x0ea9, 0x0eb3, 0x0ebc, 0x0ec4,  0x0ecd, 0x0ed5, 0x0ede, 0x0ee6,
     0x0eee, 0x0ef6, 0x0efd, 0x0f05,  0x0f0c, 0x0f14, 0x0f1b, 0x0f22,
     0x0f28, 0x0f2f, 0x0f36, 0x0f3c,  0x0f42, 0x0f49, 0x0f4f, 0x0f54,
     0x0f5a, 0x0f60, 0x0f65, 0x0f6b,  0x0f70, 0x0f75, 0x0f7a, 0x0f7f,
     0x0f83, 0x0f88, 0x0f8d, 0x0f91,  0x0f95, 0x0f9a, 0x0f9e, 0x0fa2,
     0x0fa5, 0x0fa9, 0x0fad, 0x0fb0,  0x0fb4, 0x0fb7, 0x0fba, 0x0fbd,
     0x0fc0, 0x0fc3, 0x0fc6, 0x0fc9,  0x0fcc, 0x0fce, 0x0fd1, 0x0fd3,
     0x0fd6, 0x0fd8, 0x0fda, 0x0fdc,  0x0fde, 0x0fe0, 0x0fe2, 0x0fe4,
     0x0fe5, 0x0fe7, 0x0fe9, 0x0fea,  0x0fec, 0x0fed, 0x0fee, 0x0ff0,
     0x0ff1, 0x0ff2, 0x0ff3, 0x0ff4,  0x0ff5, 0x0ff6, 0x0ff7, 0x0ff8,
     0x0ff8, 0x0ff9, 0x0ffa, 0x0ffb,  0x0ffb, 0x0ffc, 0x0ffc, 0x0ffd,
     0x0ffd, 0x0ffe, 0x0ffe, 0x0ffe,  0x0fff, 0x0fff, 0x0fff, 0x0fff,
     0x0fff, 0x1000, 0x1000, 0x1000,  0x1000, 0x1000, 0x1000, 0x1000,
     0x1000, 0x1000, 0x1000, 0x1000,  0x1000, 0x1000, 0x1000, 0x1000
};


static unsigned short phase_sine_transition(
    unsigned short curr_level,
    unsigned short next_level,
    unsigned char phase
)
{
    unsigned int res = (
        (unsigned int)curr_level *
         (unsigned int)g_sine_phase[PHASES_PER_OP-phase] +
        (unsigned int)next_level *
         (unsigned int)g_sine_phase[phase]
    ) / (unsigned int)PHASES_PER_OP;

    return (unsigned short)res;
}



static const unsigned short g_exp_phase[] = {
     0x0000, 0x0000, 0x0000, 0x0000,  0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000,  0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000,  0x0000, 0x0000, 0x0001, 0x0001,
     0x0001, 0x0001, 0x0001, 0x0001,  0x0001, 0x0001, 0x0001, 0x0001,
     0x0001, 0x0001, 0x0002, 0x0002,  0x0002, 0x0002, 0x0002, 0x0002,
     0x0002, 0x0002, 0x0002, 0x0003,  0x0003, 0x0003, 0x0003, 0x0003,
     0x0003, 0x0003, 0x0004, 0x0004,  0x0004, 0x0004, 0x0004, 0x0004,
     0x0005, 0x0005, 0x0005, 0x0005,  0x0006, 0x0006, 0x0006, 0x0006,
     0x0007, 0x0007, 0x0007, 0x0007,  0x0008, 0x0008, 0x0008, 0x0009,
     0x0009, 0x0009, 0x000a, 0x000a,  0x000a, 0x000b, 0x000b, 0x000c,
     0x000c, 0x000c, 0x000d, 0x000d,  0x000e, 0x000e, 0x000f, 0x000f,
     0x0010, 0x0011, 0x0011, 0x0012,  0x0012, 0x0013, 0x0014, 0x0014,
     0x0015, 0x0016, 0x0017, 0x0017,  0x0018, 0x0019, 0x001a, 0x001b,
     0x001c, 0x001d, 0x001e, 0x001f,  0x0020, 0x0021, 0x0022, 0x0023,
     0x0025, 0x0026, 0x0027, 0x0028,  0x002a, 0x002b, 0x002d, 0x002e,
     0x0030, 0x0031, 0x0033, 0x0035,  0x0037, 0x0039, 0x003a, 0x003c,
     0x003f, 0x0041, 0x0043, 0x0045,  0x0047, 0x004a, 0x004c, 0x004f,
     0x0052, 0x0054, 0x0057, 0x005a,  0x005d, 0x0060, 0x0063, 0x0067,
     0x006a, 0x006e, 0x0071, 0x0075,  0x0079, 0x007d, 0x0081, 0x0086,
     0x008a, 0x008f, 0x0093, 0x0098,  0x009d, 0x00a3, 0x00a8, 0x00ae,
     0x00b4, 0x00ba, 0x00c0, 0x00c6,  0x00cd, 0x00d3, 0x00db, 0x00e2,
     0x00e9, 0x00f1, 0x00f9, 0x0101,  0x010a, 0x0113, 0x011c, 0x0125,
     0x012f, 0x0139, 0x0143, 0x014e,  0x0159, 0x0165, 0x0171, 0x017d,
     0x0189, 0x0196, 0x01a4, 0x01b2,  0x01c0, 0x01cf, 0x01de, 0x01ee,
     0x01ff, 0x0210, 0x0221, 0x0233,  0x0246, 0x0259, 0x026d, 0x0281,
     0x0297, 0x02ad, 0x02c3, 0x02db,  0x02f3, 0x030c, 0x0326, 0x0340,
     0x035c, 0x0378, 0x0396, 0x03b4,  0x03d3, 0x03f4, 0x0415, 0x0438,
     0x045b, 0x0480, 0x04a6, 0x04ce,  0x04f6, 0x0520, 0x054c, 0x0579,
     0x05a7, 0x05d7, 0x0608, 0x063b,  0x0670, 0x06a7, 0x06df, 0x0719,
     0x0755, 0x0793, 0x07d3, 0x0815,  0x085a, 0x08a0, 0x08e9, 0x0935,
     0x0983, 0x09d3, 0x0a26, 0x0a7c,  0x0ad5, 0x0b30, 0x0b8f, 0x0bf1,
     0x0c56, 0x0cbe, 0x0d2a, 0x0d99,  0x0e0c, 0x0e83, 0x0efe, 0x0f7d
};

static unsigned short phase_exp_transition(
    unsigned short curr_level,
    unsigned short next_level,
    unsigned char phase
)
{
    unsigned int res = (
        (unsigned int)curr_level *
         (unsigned int)g_exp_phase[PHASES_PER_OP-phase] +
        (unsigned int)next_level *
         (unsigned int)g_exp_phase[phase]
    ) / (unsigned int)PHASES_PER_OP;

    return (unsigned short)res;
}
#endif


static double phase_linear_transition(
    double curr_level,
    double next_level,
    double phase
)
{
    return (curr_level * (1-phase)) + (next_level * phase);
}

double phase_perform_op(
    metronome_op_t op,
    double phase,
    double curr_level,
    double next_level
)
{
    double res = 0.0;

    switch(op) {
      case METRONOME_OP_CONST_NONE:
        res = 0.0;
        break;
      case METRONOME_OP_CONST_FULL:
        res = curr_level;
        break;
      case METRONOME_OP_LINEAR_RISE:
        res = phase_linear_transition(0, curr_level, phase);
        break;
      case METRONOME_OP_LINEAR_FALL:
        res = phase_linear_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_LINEAR_TRANSITION:
        res = phase_linear_transition(curr_level, next_level, phase);
        break;
      case METRONOME_OP_SINE_RISE:
        res = 0.0; /* TODO: phase_sine_transition(0, curr_level, phase); */
        break;
      case METRONOME_OP_SINE_FALL:
        res = 0.0; // TODO: phase_sine_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_SINE_TRANSITION:
        res = 0.0; // TODO: phase_sine_transition(curr_level, next_level, phase);
        break;
      case METRONOME_OP_EXP_RISE:
        res = 0.0; // TODO: phase_exp_transition(0, curr_level, phase);
        break;
      case METRONOME_OP_EXP_FALL:
        res = 0.0; // TODO: phase_exp_transition(curr_level, 0, phase);
        break;
      case METRONOME_OP_EXP_TRANSITION:
        res = 0.0; // TODO: phase_exp_transition(curr_level, next_level, phase);
        break;
      default:
        res = 0;
    }

    return res;
}


void parameters_setup()
{
    int i=0;

    const metronome_op_t metr_ops[8] = {
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE
    };
    unsigned short metr_levels[8] = {
        0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000
    };
    /*
    metronome_op_t metr_ops[4] = {
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL,
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL
    };
    */
    const double metr_levels_lpf[8] = {
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0};
    const double metr_levels_reso[4] = {0xC00, 0xC00, 0xC00, 0xC00};

    const double metr_levels_wah[8] = {
        0xA00, 0xB00, 0xC00, 0xD00,
        0xE00, 0xD00, 0xC00, 0xB00};
    const metronome_op_t metr_ops_wah[8] = {
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION
    };

    memset(g_effects, 0x00, sizeof(g_effects));

    // test - initialise the metronome with some fixed parameters
    metronome_setup(180, 4, 2);

    /* test - initialise the effects with a basic setup */
    //g_reso0.set_ctrl(PARAM_CTRL_FIXED);
    //g_reso0.set_fixed_level(3200);
    //g_reso0.set_fixed_level(0);
    g_reso0.set_ctrl(PARAM_CTRL_FIXED);
    g_reso0.set_fixed_level(0.5);

    g_low_pass0.set_ctrl(PARAM_CTRL_METRONOME);
    g_low_pass0.set_metronome_ops(metr_ops, metr_levels_lpf, 8);
    //g_low_pass0.set_ctrl(PARAM_CTRL_MANUAL);
    //g_low_pass0.set_pot_index(4);
    //g_low_pass0.set_ctrl(PARAM_CTRL_FIXED);
    //g_low_pass0.set_fixed_level(0x800);

    //g_trem.set_ctrl(PARAM_CTRL_LFO);
    //g_trem.set_pot_index(4);
    //g_trem.set_metronome_ops(metr_ops, metr_levels, 8);

    //g_reverb.set_ctrl(PARAM_CTRL_MANUAL);
    //g_reverb.set_pot_index(4);
    //g_reverb.set_ctrl(PARAM_CTRL_FIXED);
    //g_reverb.set_fixed_level(0x800);

    //g_dist.set_ctrl(PARAM_CTRL_FIXED);
    //g_dist.set_fixed_level(0x800);

    g_effects[0][0] = &g_passthru;
    g_effects[0][1] = (EffectBase*)NULL;

    i = 0;
    g_effects[1][i++] = &g_reso0;
    g_effects[1][i++] = &g_low_pass0;
    //g_effects[1][i++] = &g_trem;
    //g_effects[1][i++] = &g_reverb;
    //g_effects[1][i++] = &g_dist;

    // TODO: work out on that one...
    //g_effects[1][i] = new delay_t(true, 150, 300);
    //g_effects[1][i]->set_ctrl(PARAM_CTRL_MANUAL);
    //g_effects[1][i]->set_fixed_level(0xF00);
    //((delay_t*)g_effects[1][i])->set_lfo(100);
    //i++;

    //g_effects[1][i] = new band_pass_t();
    //g_effects[1][i]->set_ctrl(PARAM_CTRL_METRONOME);
    //g_effects[1][i]->set_metronome_ops(metr_ops_wah, metr_levels_wah, 8);
    //i++;

    g_effects[1][i] = (EffectBase*)NULL;

    g_preset_count = 1;

    lradc_setup_channels_for_polling();
}


void parameters_set()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[g_preset_count][i] == NULL) break;
        g_effects[g_preset_count][i]->params_update();
    }
}


void parameters_counter_increment()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[g_preset_count][i] == NULL) break;
        g_effects[g_preset_count][i]->params_tick();
    }
    metronome_tick();
}

