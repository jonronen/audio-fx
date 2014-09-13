PLATFORM ?= chumby


# Generic code
UTILS_OBJS = str math
ENGINE_OBJS = fx_main parameters metronome
#EFFECTS_OBJS = effect_base delay overdrive tremolo high_pass low_pass resonance
#EFFECTS_OBJS += band_pass distortion passthru
EFFECTS_OBJS = effect_base low_pass resonance passthru delay
GENERIC_OBJS = $(addsuffix .$(PLATFORM).o, \
                $(addprefix utils/, $(UTILS_OBJS)) \
                $(addprefix engine/, $(ENGINE_OBJS)) \
                $(addprefix effects/, $(EFFECTS_OBJS)) \
               )

BAREMETAL_OBJS = $(addsuffix .$(PLATFORM).o, \
                  $(addprefix utils/, new heap) \
                 )

include $(PLATFORM).mk

