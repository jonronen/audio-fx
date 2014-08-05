#include "lradc.h"
#include <Arduino.h>

int lradc_read_channel(int channel)
{
    if (channel < 0) return 0;

    // scale to 12-bit from 10-bit
    return (uint32_t)analogRead((uint32_t)channel) * 4;
}

