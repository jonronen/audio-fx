#ifndef __METRONOME_H__
#define __METRONOME_H__


#define BPMS_PER_SECOND 60
#define TICK_FREQUENCY 44100


#define MAX_DIVISION_FACTOR 16
#define MAX_PATTERN_UNITS 16


typedef enum _metronome_op_t {
    METRONOME_OP_CONST_NONE,
    METRONOME_OP_CONST_FULL,
    METRONOME_OP_LINEAR_RISE,
    METRONOME_OP_LINEAR_FALL,
    METRONOME_OP_SINE_RISE,
    METRONOME_OP_SINE_FALL,
    METRONOME_OP_EXP_RISE,
    METRONOME_OP_EXP_FALL,
    METRONOME_OP_MAX
} metronome_op_t;

typedef enum _metronome_op_type_t {
    OP_TYPE_VOLUME,
    OP_TYPE_LOW_PASS,
    OP_TYPE_RESONANCE,
    OP_TYPE_HIGH_PASS,
    OP_TYPE_FLANGER,
    OP_TYPE_DISTORTION,
    OP_TYPE_MAX
} metronome_op_type_t;


void metronome_set_frequency(unsigned short freq, unsigned char num_ops);
void metronome_set_ops(
    metronome_op_type_t type,
    metronome_op_t ops[],
    unsigned short levels[]
);
void metronome_tick();
void metronome_start();
void metronome_stop();


#endif /* __METRONOME_H__ */

