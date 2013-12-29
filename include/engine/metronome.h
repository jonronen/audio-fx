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
    METRONOME_OP_LINEAR_SMOOTH,
    METRONOME_OP_SINE_RISE,
    METRONOME_OP_SINE_FALL,
    METRONOME_OP_EXP_RISE,
    METRONOME_OP_EXP_FALL,
    METRONOME_OP_MAX
} metronome_op_t;


void metronome_setup(
    unsigned short freq,
    unsigned char num_ops,
    unsigned char pattern_units
);
void metronome_tick();
void metronome_start();
void metronome_stop();


#endif /* __METRONOME_H__ */

