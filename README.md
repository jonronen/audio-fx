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


ALSA Platform
-------------


Arduino DUE
-----------


Raspberry PI (TBD)
------------------


Chumby image
------------
1. make image (this automatically compiles and builds an image for the chumby)
2. insert a microSD card to your reader
3. sudo dd if=output_chumby/startup.img of=/dev/<sd_card_dev>
4. sudo eject /dev/<sd_card_dev>
5. plug your sd card and start playing

