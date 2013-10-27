/*
 * (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "platform/imx233/serial.h"
#include "platform/imx233/pinctrl.h"
#include "serial.h"

#define isprint(x) (x>=0x20 && x<=0x7e)

/*
 * Set baud rate. The settings are always 8n1:
 * 8 data bits, no parity, 1 stop bit 
 */
static void serial_setbrg (void)
{
    unsigned int cr, lcr_h;
    unsigned int quot;

    // Disable everything
    cr = REG_RD(DBGUART_BASE + UARTDBGCR);
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    // Calculate and set baudrate
    quot = CONFIG_DBGUART_CLK * 4 / CONFIG_BAUDRATE;
    REG_WR(DBGUART_BASE + UARTDBGFBRD, quot & 0x3f);
    REG_WR(DBGUART_BASE + UARTDBGIBRD, quot >> 6);

    // Set 8n1 mode, enable FIFOs
    lcr_h = WLEN8 | FEN;
    REG_WR(DBGUART_BASE + UARTDBGLCR_H, lcr_h);

    // Enable Debug UART
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);
}

void serial_init (void)
{
    unsigned int cr;

    // Set the uart_tx and uart_rx pins to be for the uart, and not e.g.
    // for GPIO.
    // Disable UART
//    REG_WR(DBGUART_BASE + UARTDBGCR, 0);
    __REG_CLR(HW_PINCTRL_MUXSEL(3)) = ((1<<20)|(1<<22));
    __REG_SET(HW_PINCTRL_MUXSEL(3)) = ((2<<20)|(2<<22));


    // Disable UART
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    // Mask interrupts
    REG_WR(DBGUART_BASE + UARTDBGIMSC, 0);

    // Set default baudrate
    serial_setbrg();

    // Disable FIFOs.
    cr = REG_RD(DBGUART_BASE + UARTDBGLCR_H);
    cr &= ~0x00000010;
    REG_WR(DBGUART_BASE + UARTDBGLCR_H, cr);

    // Enable UART
//    cr = DTR | TXE | RXE | UARTEN;
//    cr = REG_RD(DBGUART_BASE + UARTDBGCR);
    cr = TXE | RXE | UARTEN;
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);

    // Disable FIFOs.
    cr = REG_RD(DBGUART_BASE + UARTDBGLCR_H);
    cr &= ~0x00000010;
    REG_WR(DBGUART_BASE + UARTDBGLCR_H, cr);


    return;
}

void serial_flush() {
        while (!(REG_RD(DBGUART_BASE + UARTDBGFR) & TXFE));
}


/* Send a character */
void serial_putc (const char c) {
    // Wait for room in TX FIFO.
    while (REG_RD(DBGUART_BASE + UARTDBGFR) & TXFF)
        ;

    // Write the data byte.
    REG_WR(DBGUART_BASE + UARTDBGDR, c);

    if (c == '\n')
        serial_putc('\r');
}

void serial_puts (const char *s) {
    if(!s || !*s)
        return;
    while (*s)
        serial_putc(*s++);
}


#define HEXDIGITS "0123456789abcdef"
int serial_hexdump(unsigned char *c, int size) {
    int offset = 0;
    int abs_offset = 0;
    while(offset < size) {

        // On the first pass around, print out the absolute address.
        if(!offset)
            serial_puthex(abs_offset), serial_puts("  ");

        // Always print out the hex values.
        serial_putc(HEXDIGITS[(c[offset]>>4)&0x0f]),
        serial_putc(HEXDIGITS[(c[offset]>>0)&0x0f]),
        serial_putc(' ');

        // On byte 7 and 15 (e.g. the last bytes), add an extra space.
        if(offset == 7 || offset == 15)
            serial_putc(' ');

        // On the very last byte, print out an ascii reprepsentation.
        if(offset == 15) {
            int offset_offset;
            serial_putc('|');
            for(offset_offset = 0; offset_offset <= offset; offset_offset++) {
                unsigned char ch;
                ch = c[offset_offset];
                if(isprint(ch) && ch < 0x80)
                    serial_putc(ch);
                else
                    serial_putc('.');
            }
            serial_puts("|\n");

            size       -= 16;
            offset      = -1;
            abs_offset += 16;
            c          += 16;
        }
        offset++;
    }
    serial_putc('\n');
    return 0;
}

// Test whether a character is in RX buffer
int serial_tstc (void) {
    int ready = (!(REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE));

    // Check if RX FIFO is not empty
    if(ready) {
        int data = REG_RD(DBGUART_BASE + UARTDBGDR);
        if( data & 0xFFFFFF00 ) {
            // Clear error.
            serial_clear_error();
            return serial_tstc();
        }
    }
    return ready;
}

void serial_clear_error() {
    REG_WR(DBGUART_BASE + UARTDBGRSR_ECR, 0x000000f0);
    serial_init();
}

/* Receive character */
int serial_getc (void)
{
    int data = 0;
    while(!data) {

        // Wait while TX FIFO is empty
        while (REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE);

        data = REG_RD(DBGUART_BASE + UARTDBGDR);
        if(REG_RD(DBGUART_BASE + UARTDBGRSR_ECR)) {
            // Clear error.
            serial_clear_error();
            data = 0;
        }
    }

    /* Read data byte */
    return data;
}

static char hex[] = "0123456789abcdef";

void serial_puthex(unsigned int c) {
    int i;
    serial_puts("0x");
    for(i=7; i>=0; i--)
        serial_putc(hex[(c>>(4*i))&0x0f]);
}



