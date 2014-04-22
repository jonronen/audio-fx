PLATFORM ?= chumby


# Generic code
UTILS_OBJS = str math new heap
ENGINE_OBJS = fx_main parameters metronome
EFFECTS_OBJS = effect_base delay overdrive tremolo high_pass low_pass resonance
GENERIC_OBJS = $(addsuffix .$(PLATFORM).o, \
		  $(addprefix utils/, $(UTILS_OBJS)) \
		  $(addprefix engine/, $(ENGINE_OBJS)) \
		  $(addprefix effects/, $(EFFECTS_OBJS)))

include $(PLATFORM).mk

