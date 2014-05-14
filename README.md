Audio-FX
========


Introduction
------------

Audio-FX is an audio effects framework, written in C++, for real-time audio processing with filters and sound effects.

This projects is meant to be cross-platform, and currently supports the following platforms:

1. iMX233-based systems like the Chumby Hacker Board (http://wiki.chumby.com/index.php?title=Chumby_hacker_board) and the oLinuXino (https://www.olimex.com/Products/OLinuXino/iMX233/)
2. Arduino DUE with some codec shields (e.g. http://oshpark.com/shared_projects/O79Q55Fz)
3. A PC or a laptop with Linux and ALSA utils installed.

Audio effects can be implemented using the following frameworks:
1. Bare-metal image running on a board like the oLinuXino, Chumby, or Arduino.
2. A Linux kernel module that makes use of the low-level sound drivers.
3. User-space code that makes use of higher-level sound drivers.

Once the sound drivers are used to capture audio samples, effects and filters are used to modify them and then play them using the sound drivers again. These effects are written in C++ with portability and modularity in mind. The parameters of these effects can be controlled either by using GPIOs or by connecting external deviced like USB HIDs (not supported right now).

This project is still work in progress, it is by no means an official product.
It comes as-is with no warranty.


Directory Structure
-------------------
* effects - C++ objects (with matching .h files on include/effects) for the various sound effects and filters. Every effect should inherit from effect_base_t, defined in include/effects/effect_base.h with basic implementation in effects/effect_base.cpp
* engine - contains C/C++ sources for the core components of the system, such as the metronome, the effects engine, and the main.
* include - contains .h files for the various components.
* platform - contains platform-specific code (each platform has its own subdirectory)
* scripts - contains Python scripts that operate on .wav files, for tests.
* utils - contains cross-platform utilities such as a basic heap implementation (for dynamic memory allocations) and various implementations of mathematic functions.


ALSA Platform
-------------
ALSA is a Linux library (user-space and kernel-space) for sound. Make sure the alsa-utils package is installed if you wish to build this project on a Linux platform with ALSA.

Building this project using ALSA is just for tests and experiments - don't expect a real-time audio processing. Using the ALSA API in a user-space environment introduces a substantial amount of delay to the sound processing, which means you're not going to get a real-time effect machine.

However, for testing and debugging, this plaform is quite good. The only hardware you need is a PC (or a Laptop), a microphone, and some earphones or speakers.

Build instructions:

1. PLATFORM=alsa make
2. output_alsa/alsa_fx


Arduino DUE
-----------
Arduino DUE is based on an ARM processor clocked at 84MHz with plenty of headers for GPIOs, analog inputs, and I2S. It requires a decent codec (installed as an Arduino shield) if you want good quality audio out of it. Two such shields can be found at https://github.com/jonronen/geda-stuff/tree/master/audio_shield (schematics and design) and at http://www.oshpark.com/profiles/jronen (PCBs)

Build instructions:

1. PLATFORM=atsam3x make
2. PLATFORM=atsam3x make upload

Some notes:

* Arduino DUE doesn't have enough RAM for effects like delay and reverb
* It does have plenty of GPIOs which makes this platform very friendly for rotary encoders, potentiometers, buttons, and switches. Use that!


Raspberry PI (TBD)
------------------


Chumby image
------------
Build instructions:

1. make image (this automatically compiles and builds an image for the chumby)
2. insert a microSD card to your PC's card reader
3. sudo dd if=output_chumby/startup.img of=/dev/{sd_card_dev}
4. sudo eject /dev/{sd_card_dev}
5. plug your sd card and start playing

