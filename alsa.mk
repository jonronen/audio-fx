CURRENTDIR	= .
PLATFORMDIR	= $(CURRENTDIR)/platform/alsa
OUTPUTDIR	= $(CURRENTDIR)/output_alsa

INCLUDEDIR	= $(CURRENTDIR)/include

# Output binary
BIN = $(OUTPUTDIR)/alsa_fx

CC	= $(CROSS_COMPILE)gcc
CPP	= $(CROSS_COMPILE)g++
LD	= $(CROSS_COMPILE)g++

CFLAGS = -Wall -Wextra -I$(INCLUDEDIR) -O2
LDFLAGS = -lasound -pthread


PLATFORM_OBJS = system nonsense

ALSA_OBJS = $(addsuffix .$(PLATFORM).o, \
	      $(addprefix $(PLATFORMDIR)/, $(PLATFORM_OBJS))) \
			$(GENERIC_OBJS)


# Default goal
.PHONY: all
all: build



%.$(PLATFORM).o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.$(PLATFORM).o: %.cpp
	$(CPP) -c $(CFLAGS) -o $@ $<

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

