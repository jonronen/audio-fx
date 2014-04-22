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


ARDUINO_FILES ?= /home/jon/arduino-1.5.6-r2

ARDUINO_PORT ?= ttyACM0

GCC_INCLUDES = -I$(SAM_FILES)/system/libsam -I$(SAM_FILES)/system/CMSIS/CMSIS/Include/ -I$(SAM_FILES)/system/CMSIS/Device/ATMEL/ -I$(SAM_FILES)/cores/arduino -I$(SAM_FILES)/variants/arduino_due_x

GCC_FLAGS = -c -g -O2 -w -ffunction-sections -fdata-sections -nostdlib
GCC_FLAGS += --param max-inline-insns-single=500 -Dprintf=iprintf -mcpu=cortex-m3 -DF_CPU=84000000L
GCC_FLAGS += -DARDUINO=156 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb
GCC_FLAGS += -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Unknown"
GCC_FLAGS += -DUSB_PRODUCT="\"Arduino Due\"" $(GCC_INCLUDES)

GPP_FLAGS = $(GCC_FLAGS) -fno-rtti -fno-exceptions

CROSS_COMPILE ?= $(ARDUINO_FILES)/hardware/tools/g++_arm_none_eabi/bin/arm-none-eabi-

GPP ?= $(CROSS_COMPILE)g++
GCC ?= $(CROSS_COMPILE)gcc
OBJCOPY ?= $(CROSS_COMPILE)objcopy
AR ?= $(CROSS_COMPILE)ar

SAM_FILES = $(ARDUINO_FILES)/hardware/arduino/sam

LINKER_FLAGS = -O2 -Wl,--gc-sections -mcpu=cortex-m3 -T$(SAM_FILES)/variants/arduino_due_x/linker_scripts/gcc/flash.ld -lm -lgcc -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols -Wl,--start-group

due_uda1345_shield.bin: due_uda1345_shield.elf
	$(OBJCOPY) -O binary due_uda1345_shield.elf due_uda1345_shield.bin 

due_uda1345_shield.elf: core.a due_uda1345_shield.cpp.o
	$(GPP) $(LINKER_FLAGS) -Wl,-Map,due_uda1345_shield.map -o due_uda1345_shield.elf due_uda1345_shield.cpp.o $(SAM_FILES)/variants/arduino_due_x/libsam_sam3x8e_gcc_rel.a core.a -Wl,--end-group

core.a: syscalls_sam3.c.o wiring_analog.c.o cortex_handlers.c.o wiring_digital.c.o WInterrupts.c.o itoa.c.o wiring_shift.c.o hooks.c.o wiring.c.o dtostrf.c.o iar_calls_sam3.c.o RingBuffer.cpp.o HID.cpp.o USBCore.cpp.o CDC.cpp.o wiring_pulse.cpp.o WString.cpp.o Reset.cpp.o WMath.cpp.o Stream.cpp.o main.cpp.o UARTClass.cpp.o USARTClass.cpp.o cxxabi-compat.cpp.o IPAddress.cpp.o Print.cpp.o variant.cpp.o
	$(AR) rcs core.a syscalls_sam3.c.o 
	$(AR) rcs core.a wiring_analog.c.o 
	$(AR) rcs core.a cortex_handlers.c.o 
	$(AR) rcs core.a wiring_digital.c.o 
	$(AR) rcs core.a WInterrupts.c.o 
	$(AR) rcs core.a itoa.c.o 
	$(AR) rcs core.a wiring_shift.c.o 
	$(AR) rcs core.a hooks.c.o 
	$(AR) rcs core.a wiring.c.o 
	$(AR) rcs core.a dtostrf.c.o 
	$(AR) rcs core.a iar_calls_sam3.c.o 
	$(AR) rcs core.a RingBuffer.cpp.o 
	$(AR) rcs core.a HID.cpp.o 
	$(AR) rcs core.a USBCore.cpp.o 
	$(AR) rcs core.a CDC.cpp.o 
	$(AR) rcs core.a wiring_pulse.cpp.o 
	$(AR) rcs core.a WString.cpp.o 
	$(AR) rcs core.a Reset.cpp.o 
	$(AR) rcs core.a WMath.cpp.o 
	$(AR) rcs core.a Stream.cpp.o 
	$(AR) rcs core.a main.cpp.o 
	$(AR) rcs core.a UARTClass.cpp.o 
	$(AR) rcs core.a USARTClass.cpp.o 
	$(AR) rcs core.a cxxabi-compat.cpp.o 
	$(AR) rcs core.a IPAddress.cpp.o 
	$(AR) rcs core.a Print.cpp.o 
	$(AR) rcs core.a variant.cpp.o 

due_uda1345_shield.cpp.o: due_uda1345_shield.cpp
	$(GPP) $(GPP_FLAGS) due_uda1345_shield.cpp -o due_uda1345_shield.cpp.o 

syscalls_sam3.c.o: $(SAM_FILES)/cores/arduino/syscalls_sam3.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/syscalls_sam3.c -o syscalls_sam3.c.o
wiring_analog.c.o: $(SAM_FILES)/cores/arduino/wiring_analog.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/wiring_analog.c -o wiring_analog.c.o
cortex_handlers.c.o: $(SAM_FILES)/cores/arduino/cortex_handlers.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/cortex_handlers.c -o cortex_handlers.c.o
wiring_digital.c.o: $(SAM_FILES)/cores/arduino/wiring_digital.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/wiring_digital.c -o wiring_digital.c.o
WInterrupts.c.o: $(SAM_FILES)/cores/arduino/WInterrupts.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/WInterrupts.c -o WInterrupts.c.o
itoa.c.o: $(SAM_FILES)/cores/arduino/itoa.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/itoa.c -o itoa.c.o
wiring_shift.c.o: $(SAM_FILES)/cores/arduino/wiring_shift.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/wiring_shift.c -o wiring_shift.c.o
hooks.c.o: $(SAM_FILES)/cores/arduino/hooks.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/hooks.c -o hooks.c.o
wiring.c.o: $(SAM_FILES)/cores/arduino/wiring.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/wiring.c -o wiring.c.o
dtostrf.c.o: $(SAM_FILES)/cores/arduino/avr/dtostrf.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/avr/dtostrf.c -o dtostrf.c.o
iar_calls_sam3.c.o: $(SAM_FILES)/cores/arduino/iar_calls_sam3.c
	$(GCC) $(GCC_FLAGS) $(SAM_FILES)/cores/arduino/iar_calls_sam3.c -o iar_calls_sam3.c.o
RingBuffer.cpp.o: $(SAM_FILES)/cores/arduino/RingBuffer.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/RingBuffer.cpp -o RingBuffer.cpp.o
HID.cpp.o: $(SAM_FILES)/cores/arduino/USB/HID.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/USB/HID.cpp -o HID.cpp.o
USBCore.cpp.o: $(SAM_FILES)/cores/arduino/USB/USBCore.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/USB/USBCore.cpp -o USBCore.cpp.o
CDC.cpp.o: $(SAM_FILES)/cores/arduino/USB/CDC.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/USB/CDC.cpp -o CDC.cpp.o
wiring_pulse.cpp.o: $(SAM_FILES)/cores/arduino/wiring_pulse.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/wiring_pulse.cpp -o wiring_pulse.cpp.o
WString.cpp.o: $(SAM_FILES)/cores/arduino/WString.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/WString.cpp -o WString.cpp.o
Reset.cpp.o: $(SAM_FILES)/cores/arduino/Reset.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/Reset.cpp -o Reset.cpp.o
WMath.cpp.o: $(SAM_FILES)/cores/arduino/WMath.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/WMath.cpp -o WMath.cpp.o
Stream.cpp.o: $(SAM_FILES)/cores/arduino/Stream.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/Stream.cpp -o Stream.cpp.o
main.cpp.o: $(SAM_FILES)/cores/arduino/main.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/main.cpp -o main.cpp.o
UARTClass.cpp.o: $(SAM_FILES)/cores/arduino/UARTClass.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/UARTClass.cpp -o UARTClass.cpp.o
USARTClass.cpp.o: $(SAM_FILES)/cores/arduino/USARTClass.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/USARTClass.cpp -o USARTClass.cpp.o
cxxabi-compat.cpp.o: $(SAM_FILES)/cores/arduino/cxxabi-compat.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/cxxabi-compat.cpp -o cxxabi-compat.cpp.o
IPAddress.cpp.o: $(SAM_FILES)/cores/arduino/IPAddress.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/IPAddress.cpp -o IPAddress.cpp.o
Print.cpp.o: $(SAM_FILES)/cores/arduino/Print.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/cores/arduino/Print.cpp -o Print.cpp.o
variant.cpp.o: $(SAM_FILES)/variants/arduino_due_x/variant.cpp
	$(GPP) $(GPP_FLAGS) $(SAM_FILES)/variants/arduino_due_x/variant.cpp -o variant.cpp.o


upload: due_uda1345_shield.bin
	$(ARDUINO_FILES)/hardware/tools/bossac -i --port=$(ARDUINO_PORT) -U false -e -w -v -b due_uda1345_shield.bin -R 



clean:
	rm -rf *.o *.bin *.elf *.a *.map


