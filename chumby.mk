CURRENTDIR	= .
PLATFORMDIR	= $(CURRENTDIR)/platform/imx233
OUTPUTDIR	= $(CURRENTDIR)/output_chumby

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0x40808000
BOOT_LAYOUT_IN	= $(PLATFORMDIR)/boot.lds
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/boot.out.lds


# Output ELF image
CHUMBY_BOOT	= $(OUTPUTDIR)/chumby_fx

# Output binary image
CHUMBY_BOOT_ROM	= $(OUTPUTDIR)/chumby_fx.rom
CHUMBY_IMAGE_FILE = $(OUTPUTDIR)/startup.img

CROSS_COMPILE ?= arm-linux-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
GPP	= $(CROSS_COMPILE)g++
LD	= $(CROSS_COMPILE)g++
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

CFLAGS	= -Wall -I$(INCLUDEDIR) -I$(INCLUDEDIR)/bare
CFLAGS	+= -fno-common -fno-exceptions -fno-non-call-exceptions -fno-builtin
CFLAGS	+= -fno-stack-protector
CFLAGS	+= -O2 -DMEMORYSIZE=64 -fPIC
CPPFLAGS = $(CFLAGS) -fno-weak -fno-rtti
LDFLAGS = -static -nostdlib -O2 -T $(BOOT_LAYOUT_OUT)


PLATFORM_OBJS = entry load_from_serial serial lradc dma \
                mmu-arm icoll system audio_dma gpio

# IMPORTANT! entry.o should appear first - this is where execution starts
CHUMBY_BOOT_OBJS = $(addsuffix .$(PLATFORM).o, \
                    $(addprefix $(PLATFORMDIR)/, $(PLATFORM_OBJS))) \
                   $(GENERIC_OBJS) \
                   $(BAREMETAL_OBJS)


# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
%.$(PLATFORM).o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.$(PLATFORM).o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.$(PLATFORM).o: %.cpp
	$(GPP) -c $(CPPFLAGS) -o $@ $<

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
	cp $(PLATFORMDIR)/startup.img.orig $(CHUMBY_IMAGE_FILE)
	dd if=$(CHUMBY_BOOT_ROM) seek=132 of=$(CHUMBY_IMAGE_FILE)

