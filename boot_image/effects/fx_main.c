/* 
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include "stdint.h"
#include "serial.h"
#include "lradc.h"
#include "system.h"
#include "audio_dma.h"


#define LRADC_CHANNEL 4

static uint8_t g_print_cnt;


int fx_main()
{
    unsigned int tmp;

    system_init();
    lradc_setup_channel_for_polling(LRADC_CHANNEL);
    audio_setup();
    audio_dma_init(0);

    g_print_cnt = 0;

    serial_puts("initialisations complete\n");

    audio_dma_start();

    serial_puts("\n");

    while(1) {
        tmp = lradc_read_channel(LRADC_CHANNEL);
        serial_puts("lradc data: ");
        serial_puthex(tmp);
        serial_puts("\n");

        udelay(500000);
    }
    return 0;
}

