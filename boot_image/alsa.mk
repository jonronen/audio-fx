CURRENTDIR	= .
PLATFORMDIR	= $(CURRENTDIR)/platform/alsa
OUTPUTDIR	= $(CURRENTDIR)/output_alsa

INCLUDEDIR	= $(CURRENTDIR)/include

# Output binary
BIN = $(OUTPUTDIR)/alsa_fx

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)g++
LD	= $(CROSS_COMPILE)g++
CPP	= $(CROSS_COMPILE)cpp

CFLAGS = -Wall -I$(INCLUDEDIR) -O2
LDFLAGS = -lasound


PLATFORM_OBJS = system

ALSA_OBJS = $(addsuffix .$(PLATFORM).o, \
	      $(addprefix $(PLATFORMDIR)/, $(PLATFORM_OBJS))) \
			$(GENERIC_OBJS)


# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
%.$(PLATFORM).o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(BIN)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(ALSA_OBJS)
	@echo Build output:
	rm -rf $(OUTPUTDIR)

$(BIN): $(ALSA_OBJS)
	$(LD) -o $@ $(ALSA_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

