CURRENTDIR	= .
PLATFORMDIR	= $(CURRENTDIR)/platform/imx233
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0x40808000
BOOT_LAYOUT_IN	= $(PLATFORMDIR)/boot.lds
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/boot.out.lds


# Output ELF image
CHUMBY_BOOT	= $(OUTPUTDIR)/chumby_fx

# Output binary image
CHUMBY_BOOT_ROM	= $(OUTPUTDIR)/chumby_fx.rom

CROSS_COMPILE ?= arm-linux-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)g++
LD	= $(CROSS_COMPILE)g++
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
CFLAGS 	= -Wall -I$(INCLUDEDIR) -fno-common -fno-exceptions -fno-non-call-exceptions -fno-weak -fno-rtti -fno-builtin -O2 -DMEMORYSIZE=64
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT_OUT) -L$(LIBGCCDIR)

CFLAGS += -fPIC


# Generic code
UTILS_OBJS = str.o math.o
PLATFORM_OBJS = entry.o serial.o lradc.o dma.o mmu-arm.o icoll.o system.o audio_dma.o
ENGINE_OBJS = fx_main.o parameters.o metronome.o
EFFECTS_OBJS = effect_base.o


# IMPORTANT! entry.o should appear first - this is where execution starts
CHUMBY_BOOT_OBJS = $(addprefix $(PLATFORMDIR)/, $(PLATFORM_OBJS)) \
		  $(addprefix utils/, $(UTILS_OBJS)) \
		  $(addprefix engine/, $(ENGINE_OBJS)) \
		  $(addprefix effects/, $(EFFECTS_OBJS))


# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(CHUMBY_BOOT_ROM)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(CHUMBY_BOOT_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)
	@echo Build system:
	rm -rf $(INCLUDEDIR)/arch

##
## Rules to build linux_prep image
## 
#$(CMDLINES_STRIP): $(CMDLINES)
#	$(call strip_cmdlines_file)

$(CHUMBY_BOOT_ROM): $(CHUMBY_BOOT)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$(CHUMBY_BOOT): $(CHUMBY_BOOT_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $(CHUMBY_BOOT_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DMEMORY_SIZE=64 -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

image: build_prep $(CHUMBY_BOOT_ROM)
	cp startup.img.orig startup.img
	dd if=$(CHUMBY_BOOT_ROM) seek=132 of=./startup.img
