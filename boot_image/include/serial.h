/*
 * Debug UART register definitions
 */

#ifndef STMP3XXX_DBGUART_H
#define STMP3XXX_DBGUART_H

#define CONFIG_BAUDRATE 115200
#define CONFIG_DBGUART_CLK  24000000 // 115200 baud

#define REG_WR(x,v) ((*(volatile unsigned int*)(x)) = (v))
#define REG_RD(x)   (*(volatile unsigned int*)(x))
#define STMP378X_REGS_BASE  0x80000000
#define DBGUART_BASE    (STMP378X_REGS_BASE + 0x00070000)
#define PINCTRL_BASE (STMP378X_REGS_BASE + 0x18000)

#include "types.h"
#include "regs-pinctrl.h"

#define HW_PINCTRL_MUXSEL3_ADDR (PINCTRL_BASE + 0x00000130)
#define HW_PINCTRL_MUXSEL3 REG_RD(HW_PINCTRL_MUXSEL3_ADDR)
//#define HW_PINCTRL_MUXSEL3_SET(y) REG_WR(HW_PINCTRL_MUXSEL3_ADDR+4, y)
//#define HW_PINCTRL_MUXSEL3_CLR(y) REG_WR(HW_PINCTRL_MUXSEL3_ADDR+8, y)
#define BM_PINCTRL_MUXSEL3_BANK1_PIN26 0x00300000
#define BM_PINCTRL_MUXSEL3_BANK1_PIN27 0x00C00000




#define UARTDBGDR	0x00
#define UARTDBGRSR_ECR	0x04
#define UARTDBGFR	0x18
#define UARTDBGILPR	0x20
#define UARTDBGIBRD	0x24
#define UARTDBGFBRD	0x28
#define UARTDBGLCR_H	0x2c
#define UARTDBGCR	0x30
#define UARTDBGIFLS	0x34
#define UARTDBGIMSC	0x38
#define UARTDBGRIS	0x3c
#define UARTDBGMIS	0x40
#define UARTDBGICR	0x44
#define UARTDBGDMACR	0x48

/* UARTDBGFR - Flag Register bits */
#define CTS	0x0001
#define DSR	0x0002
#define DCD	0x0004
#define BUSY	0x0008
#define RXFE	0x0010
#define TXFF	0x0020
#define RXFF	0x0040
#define TXFE	0x0080
#define RI	0x0100

/* UARTDBGLCR_H - Line Control Register bits */
#define BRK	0x0001
#define PEN	0x0002
#define EPS	0x0004
#define STP2	0x0008
#define FEN	0x0010
#define WLEN5	0x0000
#define WLEN6	0x0020
#define WLEN7	0x0040
#define WLEN8	0x0060
#define SPS	0x0080

/* UARTDBGCR - Control Register bits */
#define UARTEN	0x0001
#define LBE	0x0080
#define TXE	0x0100
#define RXE	0x0200
#define DTR	0x0400
#define RTS	0x0800
#define OUT1	0x1000
#define OUT2	0x2000
#define RTSEN	0x4000
#define CTSEN	0x8000

void serial_puts(const char *s);
void serial_init(void);
int  serial_getc(void);
void serial_putc(const char c);
void serial_puthex(u32 c);
int  serial_tstc (void);
void serial_clear_error(void);
void serial_flush(void);


#endif /* STMP3XXX_DBGUART_H */
