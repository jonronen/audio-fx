#
# audio-fx
# Copyright 2012-2014 Jon Ronen-Drori
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


CURRENTDIR      = .
PLATFORMDIR     = $(CURRENTDIR)/platform/atsam3x
OUTPUTDIR       = $(CURRENTDIR)/output_atsam3x

FLAVOUR	?= uda1345
TARGET	=  due_$(FLAVOUR)_shield
BIN 	=  $(OUTPUTDIR)/$(TARGET)

ARDUINO_FILES ?= /home/jon/arduino-1.5.6-r2
SAM_FILES = $(ARDUINO_FILES)/hardware/arduino/sam

ARDUINO_PORT ?= ttyACM0

C_INCLUDES = -I$(SAM_FILES)/system/libsam
C_INCLUDES += -I$(SAM_FILES)/system/CMSIS/CMSIS/Include/
C_INCLUDES += -I$(SAM_FILES)/system/CMSIS/Device/ATMEL/
C_INCLUDES += -I$(SAM_FILES)/cores/arduino
C_INCLUDES += -I$(SAM_FILES)/variants/arduino_due_x
C_INCLUDES += -I$(CURRENTDIR)/include

CFLAGS = -c -g -O2 -w -ffunction-sections -fdata-sections -nostdlib
CFLAGS += --param max-inline-insns-single=500 -Dprintf=iprintf -mcpu=cortex-m3 -DF_CPU=84000000L
CFLAGS += -DARDUINO=156 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb
CFLAGS += -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Unknown"
CFLAGS += -DUSB_PRODUCT="\"Arduino Due\"" $(C_INCLUDES)

CPPFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions

CROSS_COMPILE ?= $(ARDUINO_FILES)/hardware/tools/g++_arm_none_eabi/bin/arm-none-eabi-

CPP	= $(CROSS_COMPILE)g++
CC	= $(CROSS_COMPILE)gcc
OBJCOPY	= $(CROSS_COMPILE)objcopy
AR	= $(CROSS_COMPILE)ar

LDFLAGS = -O2 -Wl,--gc-sections -mcpu=cortex-m3 -T$(SAM_FILES)/variants/arduino_due_x/linker_scripts/gcc/flash.ld -lm -lgcc -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols -Wl,--start-group


SAM_OBJS = syscalls_sam3 wiring_analog cortex_handlers wiring_digital
SAM_OBJS += WInterrupts itoa wiring_shift hooks wiring avr/dtostrf
SAM_OBJS += iar_calls_sam3 RingBuffer USB/HID USB/USBCore USB/CDC wiring_pulse
SAM_OBJS += WString Reset WMath Stream main UARTClass USARTClass
SAM_OBJS += cxxabi-compat IPAddress Print ../../variants/arduino_due_x/variant
PLATFORM_OBJS = $(TARGET)

ATSAM3X_OBJS = $(addsuffix .$(PLATFORM).o, \
                 $(addprefix $(PLATFORMDIR)/, $(PLATFORM_OBJS)) \
                 $(addprefix $(SAM_FILES)/cores/arduino/, $(SAM_OBJS)) \
               ) \
               $(GENERIC_OBJS)


# Default goal
.PHONY: all
all: build


%.$(PLATFORM).o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.$(PLATFORM).o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(BIN).bin

build_prep:
	mkdir -p $(OUTPUTDIR)

$(BIN).elf: $(ATSAM3X_OBJS)
	$(GPP) $(LDFLAGS) -Wl,-Map,$(BIN).map -o $(BIN).elf $(ATSAM3X_OBJS) $(SAM_FILES)/variants/arduino_due_x/libsam_sam3x8e_gcc_rel.a -Wl,--end-group


clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(ATSAM3X_OBJS)
	@echo Build output:
	rm -rf $(OUTPUTDIR)


$(BIN).bin: $(BIN).elf
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin 


upload: $(BIN).bin
	$(ARDUINO_FILES)/hardware/tools/bossac -i --port=$(ARDUINO_PORT) -U false -e -w -v -b $(BIN).bin -R

