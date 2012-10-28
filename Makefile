obj-m += sound-kmod.o

all:
	CROSS_COMPILE=arm-linux- ARCH=arm make -C /home/jronen/chumby/kernel/linux-2.6.28.mx233/ M=$(PWD) modules
clean:
	CROSS_COMPILE=arm-linux- ARCH=arm make -C /home/jronen/chumby/kernel/linux-2.6.28.mx233/ M=$(PWD) clean
