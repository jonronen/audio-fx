#include "gpio.h"
#include "platform/imx233/pinctrl.h"


/*
 * we're using LCD_D06 - LDC_D15 as our GPIO inputs
 * and LCD_D16 - LCD_D17 as GPIO outputs (metronome)
 *
 * NOTE: this is different on Olinuxino (values here are for Chumby)
 */

#define GPIO_BANK 1
#define GPIO_METRONOME_MAIN_PIN 16
#define GPIO_METRONOME_SECONDARY_PIN 17

#define GPIO_INPUT_PIN_COUNT 10


void gpio_setup(void)
{
    int i;

    imx233_pinctrl_init();

    imx233_enable_gpio_output(
        GPIO_BANK,
        GPIO_METRONOME_MAIN_PIN,
        true
    );
    imx233_set_gpio_output(
        GPIO_BANK,
        GPIO_METRONOME_MAIN_PIN,
        false
    );

    imx233_enable_gpio_output(
        GPIO_BANK,
        GPIO_METRONOME_SECONDARY_PIN,
        true
    );
    imx233_set_gpio_output(
        GPIO_BANK,
        GPIO_METRONOME_SECONDARY_PIN,
        false
    );

    for (i=0; i<GPIO_INPUT_PIN_COUNT; i++) {
        imx233_set_pin_function(GPIO_BANK, i+6, PINCTRL_FUNCTION_GPIO);
        imx233_set_pin_drive_strength(GPIO_BANK, i+6, PINCTRL_DRIVE_12mA);
        imx233_enable_gpio_output(GPIO_BANK, i+6, false);
    }
}

bool gpio_get_input(unsigned char pin)
{
    if (pin >= GPIO_INPUT_PIN_COUNT) return false;

    if (imx233_get_gpio_input_mask(GPIO_BANK, 1<<(pin+6))) return true;
    return false;
}

void gpio_set_metronome_output(bool main, bool value)
{
    unsigned char pin_no = GPIO_METRONOME_MAIN_PIN;
    if (!main) pin_no = GPIO_METRONOME_SECONDARY_PIN;

    imx233_set_gpio_output(GPIO_BANK, pin_no, value);
}

