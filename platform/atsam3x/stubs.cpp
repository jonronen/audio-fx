#include <Arduino.h>
#include "serial.h"
#include "system.h"


extern "C" void serial_puts(const char *s)
{
    Serial.print(s);
}

extern "C" void serial_init(void)
{
    Serial.begin(115200);
}

extern "C" void serial_puthex(unsigned int c)
{
    Serial.print(c);
}

extern "C" void system_init()
{
}

extern "C" void udelay(unsigned int us)
{
}

