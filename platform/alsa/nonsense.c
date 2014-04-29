#include "system.h"
#include "stdbool.h"
#include "lradc.h"
#include <stdio.h>

void udelay(unsigned t)
{
}

int lradc_read_channel(int channel)
{
    return -1;
}

void lradc_setup_channels_for_polling()
{
}

void serial_puts(const char* str)
{
    printf("%s", str);
}

void serial_puthex(unsigned int h)
{
    printf("%x", h);
}

void gpio_setup()
{
}

void gpio_set_metronome_output(bool main, bool value)
{
}

