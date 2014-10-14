#include "lradc.h"
#include <Arduino.h>

double lradc_read_channel(const unsigned int channel)
{
    if (channel < 0) return 0;

    // scale to a double from 10-bit
    return (double)analogRead((uint32_t)channel) / 1024.0;
}

