#ifndef __GPIO_H__
#define __GPIO_H__


#include "stdint.h"
#include "stdbool.h"


void gpio_setup(void);
bool gpio_get_input(unsigned char pin);
void gpio_set_metronome_output(bool main, bool value);


#endif /* __GPIO_H__ */

