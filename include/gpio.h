#ifndef __GPIO_H__
#define __GPIO_H__


#include "stdint.h"
#include "stdbool.h"


#ifdef __cplusplus
extern "C" {
#endif


void gpio_setup(void);
bool gpio_get_input(unsigned char pin);
void gpio_set_metronome_output(bool main, bool value);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __GPIO_H__ */

