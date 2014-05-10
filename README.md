Audio-FX
========

An audio effects framework, written in C++, for real-time audio processing with filters and sound effects.

This projects is meant to be cross-platform, and currently supports the following platforms:

1. iMX233-based systems like the Chumby Hacker Board (http://wiki.chumby.com/index.php?title=Chumby_hacker_board) and the oLinuXino (https://www.olimex.com/Products/OLinuXino/iMX233/)
2. Arduino DUE with some codec shields (e.g. http://oshpark.com/shared_projects/O79Q55Fz)
3. A PC or a laptop with Linux and ALSA utils installed.

These sources contain two variants of the same idea:
1. A Linux kernel module, meant to be built and loaded from the Chumby's OS.
2. A boot image, meant to be built and copied to a MicroSD card, as a replacement for the whole OS.

The final goal of this project is to create a DIY sound effects machine that uses the Chumby's line-in as audio input and uses the Chumby's headphone as audio output. The different parameters for the planned effects can be controlled either by connecting knobs and switches directly or by using MIDI connections and/or USB HID devices (to be done...)

This project is still work in progress, it is by no means an official product.
It comes as-is with no warranty.


How to build a Chumby image
---------------------------
1. make image (this automatically compiles and builds an image for the chumby)
2. insert a microSD card to your reader
3. sudo dd if=output_chumby/startup.img of=/dev/<sd_card_dev>
4. sudo eject /dev/<sd_card_dev>
5. plug your sd card and start playing

