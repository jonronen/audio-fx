#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "stdint.h"
#include "serial.h"
#include "utils/str.h"

/* effects */
#include "effects/effect_base.h"
#include "effects/low_pass.h"
#include "effects/high_pass.h"
#include "effects/tremolo.h"
#include "effects/overdrive.h"
//#include "effects/distortion.h"
#include "effects/delay.h"
#include "effects/resonance.h"


effect_base_t* g_effects[MAX_EFFECT_COUNT];

static resonance_t g_reso0;
static resonance_t g_reso1;
static low_pass_t  g_low_pass0(&g_reso0);
static low_pass_t  g_low_pass1(&g_reso1);
static tremolo_t   g_trem;
static delay_t     g_reverb(true, 8000, 8000);


/*
 * numerical tables for parameter conversions (e.g. logarithms)
 */
static const unsigned short g_two_exp_fraction[] = {
    256, 256, 256, 257, 257, 257, 258, 258, 
    258, 259, 259, 259, 260, 260, 260, 261, 
    261, 261, 262, 262, 263, 263, 263, 264, 
    264, 264, 265, 265, 265, 266, 266, 266, 
    267, 267, 268, 268, 268, 269, 269, 269, 
    270, 270, 270, 271, 271, 272, 272, 272, 
    273, 273, 273, 274, 274, 275, 275, 275, 
    276, 276, 276, 277, 277, 278, 278, 278, 
    279, 279, 279, 280, 280, 281, 281, 281, 
    282, 282, 282, 283, 283, 284, 284, 284, 
    285, 285, 286, 286, 286, 287, 287, 287, 
    288, 288, 289, 289, 289, 290, 290, 291, 
    291, 291, 292, 292, 293, 293, 293, 294, 
    294, 295, 295, 295, 296, 296, 297, 297, 
    297, 298, 298, 299, 299, 299, 300, 300, 
    301, 301, 301, 302, 302, 303, 303, 304, 
    304, 304, 305, 305, 306, 306, 306, 307, 
    307, 308, 308, 309, 309, 309, 310, 310, 
    311, 311, 311, 312, 312, 313, 313, 314, 
    314, 314, 315, 315, 316, 316, 317, 317, 
    317, 318, 318, 319, 319, 320, 320, 320, 
    321, 321, 322, 322, 323, 323, 323, 324, 
    324, 325, 325, 326, 326, 327, 327, 327, 
    328, 328, 329, 329, 330, 330, 331, 331, 
    331, 332, 332, 333, 333, 334, 334, 335, 
    335, 336, 336, 336, 337, 337, 338, 338, 
    339, 339, 340, 340, 341, 341, 342, 342, 
    342, 343, 343, 344, 344, 345, 345, 346, 
    346, 347, 347, 348, 348, 349, 349, 349, 
    350, 350, 351, 351, 352, 352, 353, 353, 
    354, 354, 355, 355, 356, 356, 357, 357, 
    358, 358, 359, 359, 360, 360, 361, 361, 
    362, 362, 363, 363, 364, 364, 364, 365, 
    365, 366, 366, 367, 367, 368, 368, 369, 
    369, 370, 370, 371, 371, 372, 372, 373, 
    373, 374, 375, 375, 376, 376, 377, 377, 
    378, 378, 379, 379, 380, 380, 381, 381, 
    382, 382, 383, 383, 384, 384, 385, 385, 
    386, 386, 387, 387, 388, 388, 389, 390, 
    390, 391, 391, 392, 392, 393, 393, 394, 
    394, 395, 395, 396, 396, 397, 398, 398, 
    399, 399, 400, 400, 401, 401, 402, 402, 
    403, 403, 404, 405, 405, 406, 406, 407, 
    407, 408, 408, 409, 410, 410, 411, 411, 
    412, 412, 413, 413, 414, 415, 415, 416, 
    416, 417, 417, 418, 419, 419, 420, 420, 
    421, 421, 422, 423, 423, 424, 424, 425, 
    425, 426, 427, 427, 428, 428, 429, 429, 
    430, 431, 431, 432, 432, 433, 434, 434, 
    435, 435, 436, 436, 437, 438, 438, 439, 
    439, 440, 441, 441, 442, 442, 443, 444, 
    444, 445, 445, 446, 447, 447, 448, 448, 
    449, 450, 450, 451, 452, 452, 453, 453, 
    454, 455, 455, 456, 456, 457, 458, 458, 
    459, 460, 460, 461, 461, 462, 463, 463, 
    464, 465, 465, 466, 466, 467, 468, 468, 
    469, 470, 470, 471, 472, 472, 473, 473, 
    474, 475, 475, 476, 477, 477, 478, 479, 
    479, 480, 481, 481, 482, 483, 483, 484, 
    485, 485, 486, 486, 487, 488, 488, 489, 
    490, 490, 491, 492, 492, 493, 494, 494, 
    495, 496, 496, 497, 498, 498, 499, 500, 
    501, 501, 502, 503, 503, 504, 505, 505, 
    506, 507, 507, 508, 509, 509, 510, 511,
};

unsigned short two_exp_12bit_to_8bit(unsigned short raw_value)
{
    unsigned int tmp = 1 << (raw_value/512);
    tmp *= g_two_exp_fraction[raw_value % 512];
    tmp /= 256;

    return (unsigned short)(tmp & 0xffff);
}



/*
 * square wave (either min or max)
 */
static const unsigned short g_square_phase[] = {
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
};

unsigned short phase_to_square_wave(unsigned char phase)
{
    return g_square_phase[phase];
}


/*
 * triangular wave (linear fade-out, then linear fade-in)
 */
/* TODO: change this to 12-bit instead of 8-bit */
static const unsigned short g_triangular_phase[] = {
    0x100, 0xfe,  0xfc,  0xfa,  0xf8,  0xf6,  0xf4,  0xf2,
    0xf0,  0xee,  0xec,  0xea,  0xe8,  0xe6,  0xe4,  0xe2,
    0xe0,  0xde,  0xdc,  0xda,  0xd8,  0xd6,  0xd4,  0xd2,
    0xd0,  0xce,  0xcc,  0xca,  0xc8,  0xc6,  0xc4,  0xc2,
    0xc0,  0xbe,  0xbc,  0xba,  0xb8,  0xb6,  0xb4,  0xb2,
    0xb0,  0xae,  0xac,  0xaa,  0xa8,  0xa6,  0xa4,  0xa2,
    0xa0,  0x9e,  0x9c,  0x9a,  0x98,  0x96,  0x94,  0x92,
    0x90,  0x8e,  0x8c,  0x8a,  0x88,  0x86,  0x84,  0x82,
    0x80,  0x7e,  0x7c,  0x7a,  0x78,  0x76,  0x74,  0x72,
    0x70,  0x6e,  0x6c,  0x6a,  0x68,  0x66,  0x64,  0x62,
    0x60,  0x5e,  0x5c,  0x5a,  0x58,  0x56,  0x54,  0x52,
    0x50,  0x4e,  0x4c,  0x4a,  0x48,  0x46,  0x44,  0x42,
    0x40,  0x3e,  0x3c,  0x3a,  0x38,  0x36,  0x34,  0x32,
    0x30,  0x2e,  0x2c,  0x2a,  0x28,  0x26,  0x24,  0x22,
    0x20,  0x1e,  0x1c,  0x1a,  0x18,  0x16,  0x14,  0x12,
    0x10,  0xe,   0xc,   0xa,   0x8,   0x6,   0x4,   0x2,
    0x0,   0x2,   0x4,   0x6,   0x8,   0xa,   0xc,   0xe,
    0x10,  0x12,  0x14,  0x16,  0x18,  0x1a,  0x1c,  0x1e,
    0x20,  0x22,  0x24,  0x26,  0x28,  0x2a,  0x2c,  0x2e,
    0x30,  0x32,  0x34,  0x36,  0x38,  0x3a,  0x3c,  0x3e,
    0x40,  0x42,  0x44,  0x46,  0x48,  0x4a,  0x4c,  0x4e,
    0x50,  0x52,  0x54,  0x56,  0x58,  0x5a,  0x5c,  0x5e,
    0x60,  0x62,  0x64,  0x66,  0x68,  0x6a,  0x6c,  0x6e,
    0x70,  0x72,  0x74,  0x76,  0x78,  0x7a,  0x7c,  0x7e,
    0x80,  0x82,  0x84,  0x86,  0x88,  0x8a,  0x8c,  0x8e,
    0x90,  0x92,  0x94,  0x96,  0x98,  0x9a,  0x9c,  0x9e,
    0xa0,  0xa2,  0xa4,  0xa6,  0xa8,  0xaa,  0xac,  0xae,
    0xb0,  0xb2,  0xb4,  0xb6,  0xb8,  0xba,  0xbc,  0xbe,
    0xc0,  0xc2,  0xc4,  0xc6,  0xc8,  0xca,  0xcc,  0xce,
    0xd0,  0xd2,  0xd4,  0xd6,  0xd8,  0xda,  0xdc,  0xde,
    0xe0,  0xe2,  0xe4,  0xe6,  0xe8,  0xea,  0xec,  0xee,
    0xf0,  0xf2,  0xf4,  0xf6,  0xf8,  0xfa,  0xfc,  0xfe,
};

unsigned short phase_to_triangular_wave(unsigned char phase)
{
    return g_triangular_phase[phase];
}


/*
 * saw tooth (linear fade-out with a discontinuity upwards)
 */
unsigned short phase_to_sawtooth_wave(unsigned char phase)
{
    return (phase+1)*16;
}


/*
 * reverse saw tooth (linear fade-out with a discontinuity upwards)
 */
unsigned short phase_to_reverse_sawtooth_wave(unsigned char phase)
{
    return 0x1000 - (phase*16);
}


/* TODO: change this to 12-bit instead of 8-bit */
static const unsigned short g_sine_phase[] = {
    0x100, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe,
    0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7,
    0xf6, 0xf5, 0xf3, 0xf2, 0xf0, 0xef, 0xed, 0xec,
    0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
    0xda, 0xd8, 0xd5, 0xd3, 0xd1, 0xce, 0xcc, 0xc9,
    0xc7, 0xc4, 0xc1, 0xbf, 0xbc, 0xb9, 0xb6, 0xb3,
    0xb0, 0xae, 0xab, 0xa8, 0xa5, 0xa2, 0x9f, 0x9c,
    0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83,
    0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a,
    0x67, 0x63, 0x60, 0x5d, 0x5a, 0x57, 0x54, 0x51,
    0x4f, 0x4c, 0x49, 0x46, 0x43, 0x40, 0x3e, 0x3b,
    0x38, 0x36, 0x33, 0x31, 0x2e, 0x2c, 0x2a, 0x27,
    0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17,
    0x15, 0x13, 0x12, 0x10, 0x0f, 0x0d, 0x0c, 0x0a,
    0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x03,
    0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13,
    0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23,
    0x25, 0x27, 0x2a, 0x2c, 0x2e, 0x31, 0x33, 0x36,
    0x38, 0x3b, 0x3e, 0x40, 0x43, 0x46, 0x49, 0x4c,
    0x4f, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x63,
    0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c,
    0x7f, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95,
    0x98, 0x9c, 0x9f, 0xa2, 0xa5, 0xa8, 0xab, 0xae,
    0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbf, 0xc1, 0xc4,
    0xc7, 0xc9, 0xcc, 0xce, 0xd1, 0xd3, 0xd5, 0xd8,
    0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8,
    0xea, 0xec, 0xed, 0xef, 0xf0, 0xf2, 0xf3, 0xf5,
    0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc,
    0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned short phase_to_sine_wave(unsigned char phase)
{
    return g_sine_phase[phase];
}

unsigned short phase_perform_op(
    metronome_op_t op,
    unsigned char phase,
    unsigned short curr_level,
    unsigned short next_level
)
{
    unsigned short res = 0;

    switch(op) {
      case METRONOME_OP_CONST_NONE:
        res = 0;
        break;
      case METRONOME_OP_CONST_FULL:
        res = 0x1000;
        break;
      case METRONOME_OP_LINEAR_RISE:
        res = phase_to_sawtooth_wave(phase); 
        break;
      case METRONOME_OP_LINEAR_FALL:
        res = phase_to_reverse_sawtooth_wave(phase);
        break;
      case METRONOME_OP_SINE_RISE:
        /* TODO */
        break;
      case METRONOME_OP_SINE_FALL:
        /* TODO */
        break;
      case METRONOME_OP_EXP_RISE:
        /* TODO */
        break;
      case METRONOME_OP_EXP_FALL:
        /* TODO */
        break;
      default:
        res = 0;
    }

    res = (unsigned short)(
        (unsigned int)res * (unsigned int)curr_level /
        EFFECT_MAX_LEVEL
    );

    return res;
}


void parameters_setup()
{
    int i=0;

    /*
    metronome_op_t metr_ops[2] = {
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE
    };
    unsigned short metr_levels[2] = {0x1000, 0x1000};
    */
    metronome_op_t metr_ops[4] = {
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL,
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL
    };
    unsigned short metr_levels_lpf[4] = {0x100, 0x400, 0x900, 0x1000};
    unsigned short metr_levels_reso[4] = {0x100, 0x400, 0x900, 0xC00};

    memset(g_effects, 0x00, sizeof(g_effects));

    // test - initialise the metronome with some fixed parameters
    metronome_setup(120, 1, 4);

    /* test - initialise the effects with a basic setup */
    //g_reso0.set_fixed_level(3200);
    //g_reso0.set_fixed_level(0);
    g_reso0.set_ctrl(PARAM_CTRL_METRONOME);
    g_reso0.set_metronome_ops(metr_ops, metr_levels_reso, 4);

    g_low_pass0.set_ctrl(PARAM_CTRL_METRONOME);
    g_low_pass0.set_metronome_ops(metr_ops, metr_levels_lpf, 4);
    //g_low_pass0.set_ctrl(PARAM_CTRL_MANUAL);
    //g_low_pass0.set_pot_index(4);
    //g_low_pass0.set_ctrl(PARAM_CTRL_FIXED);
    //g_low_pass0.set_fixed_level(0x800);

    g_trem.set_ctrl(PARAM_CTRL_LFO);
    g_trem.set_pot_index(4);
    //g_trem.set_metronome_ops(metr_ops, metr_levels, 2);

    g_reverb.set_ctrl(PARAM_CTRL_MANUAL);
    g_reverb.set_pot_index(4);

    //g_effects[i++] = &g_reso0;
    //g_effects[i++] = &g_low_pass0;
    //g_effects[i++] = &g_trem;
    g_effects[i++] = &g_reverb;
    g_effects[i] = (effect_base_t*)NULL;

    lradc_setup_channels_for_polling();
}


void parameters_set()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[i] == NULL) break;
        g_effects[i]->params_update();
    }
}


void parameters_counter_increment()
{
    int i;

    for (i=0; i<MAX_EFFECT_COUNT; i++) {
        if (g_effects[i] == NULL) break;
        g_effects[i]->params_tick();
    }
    metronome_tick();
}

