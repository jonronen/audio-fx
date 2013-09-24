#ifndef CLKCTRL_IMX233_H
#define CLKCTRL_IMX233_H

#include "system.h"

#define HW_CLKCTRL_BASE     0x80040000

#define HW_CLKCTRL_PLLCTRL0 (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x0))
#define HW_CLKCTRL_PLLCTRL0__POWER          (1 << 16)
#define HW_CLKCTRL_PLLCTRL0__EN_USB_CLKS    (1 << 18)
#define HW_CLKCTRL_PLLCTRL0__DIV_SEL_BP     20
#define HW_CLKCTRL_PLLCTRL0__DIV_SEL_BM     (3 << 20)

#define HW_CLKCTRL_PLLCTRL1 (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x10))
#define HW_CLKCTRL_PLLCTRL1__LOCK       (1 << 31)

#define HW_CLKCTRL_CPU      (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x20))
#define HW_CLKCTRL_CPU__DIV_CPU_BP  0
#define HW_CLKCTRL_CPU__DIV_CPU_BM  0x3f
#define HW_CLKCTRL_CPU__INTERRUPT_WAIT  (1 << 12)
#define HW_CLKCTRL_CPU__DIV_XTAL_BP 16
#define HW_CLKCTRL_CPU__DIV_XTAL_BM (0x3ff << 16)
#define HW_CLKCTRL_CPU__DIV_XTAL_FRAC_EN    (1 << 26)
#define HW_CLKCTRL_CPU__BUSY_REF_CPU    (1 << 28)

#define HW_CLKCTRL_HBUS     (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x30))
#define HW_CLKCTRL_HBUS__DIV_BP         0
#define HW_CLKCTRL_HBUS__DIV_BM         0x1f
#define HW_CLKCTRL_HBUS__DIV_FRAC_EN    (1 << 5)
#define HW_CLKCTRL_HBUS__SLOW_DIV_BP    16
#define HW_CLKCTRL_HBUS__SLOW_DIV_BM    (0x7 << 16)
#define HW_CLKCTRL_HBUS__AUTO_SLOW_MODE (1 << 20)

/* warning: this register doesn't have a CLR/SET variant ! */
#define HW_CLKCTRL_XBUS     (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x40))
#define HW_CLKCTRL_XBUS__DIV_BP     0
#define HW_CLKCTRL_XBUS__DIV_BM     0x3ff
#define HW_CLKCTRL_XBUS__BUSY       (1 << 31)

#define HW_CLKCTRL_XTAL     (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x50))
#define HW_CLKCTRL_XTAL__TIMROT_CLK32K_GATE (1 << 26)
#define HW_CLKCTRL_XTAL__DRI_CLK24M_GATE    (1 << 28)
#define HW_CLKCTRL_XTAL__FILT_CLK24M_GATE   (1 << 30)

/* warning: this register doesn't have a CLR/SET variant ! */
#define HW_CLKCTRL_PIX      (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x60))
#define HW_CLKCTRL_PIX__DIV_BP  0
#define HW_CLKCTRL_PIX__DIV_BM  0xfff

/* warning: this register doesn't have a CLR/SET variant ! */
#define HW_CLKCTRL_SSP      (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x70))
#define HW_CLKCTRL_SSP__DIV_BP  0
#define HW_CLKCTRL_SSP__DIV_BM  0x1ff

/* warning: this register doesn't have a CLR/SET variant ! */
#define HW_CLKCTRL_EMI      (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0xa0))
#define HW_CLKCTRL_EMI__DIV_EMI_BP  0
#define HW_CLKCTRL_EMI__DIV_EMI_BM  0x3f
#define HW_CLKCTRL_EMI__DIV_XTAL_BP 8
#define HW_CLKCTRL_EMI__DIV_XTAL_BM (0xf << 8)
#define HW_CLKCTRL_EMI__BUSY_REF_EMI    (1 << 28)
#define HW_CLKCTRL_EMI__BUSY_REF_XTAL   (1 << 29)
#define HW_CLKCTRL_EMI__SYNC_MODE_EN    (1 << 30)
#define HW_CLKCTRL_EMI__CLKGATE     (1 << 31)

#define HW_CLKCTRL_CLKSEQ   (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x110))
#define HW_CLKCTRL_CLKSEQ__BYPASS_PIX   (1 << 1)
#define HW_CLKCTRL_CLKSEQ__BYPASS_SSP   (1 << 5)
#define HW_CLKCTRL_CLKSEQ__BYPASS_EMI   (1 << 6)
#define HW_CLKCTRL_CLKSEQ__BYPASS_CPU   (1 << 7)

#define HW_CLKCTRL_FRAC     (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0xf0))
#define HW_CLKCTRL_FRAC_CPU (*(volatile uint8_t *)(HW_CLKCTRL_BASE + 0xf0))
#define HW_CLKCTRL_FRAC_EMI (*(volatile uint8_t *)(HW_CLKCTRL_BASE + 0xf1))
#define HW_CLKCTRL_FRAC_PIX (*(volatile uint8_t *)(HW_CLKCTRL_BASE + 0xf2))
#define HW_CLKCTRL_FRAC_IO  (*(volatile uint8_t *)(HW_CLKCTRL_BASE + 0xf3))
#define HW_CLKCTRL_FRAC_XX__XXDIV_BM    0x3f
#define HW_CLKCTRL_FRAC_XX__XX_STABLE   (1 << 6)
#define HW_CLKCTRL_FRAC_XX__CLKGATEXX   (1 << 7)

/* warning: this register doesn't have a CLR/SET variant ! */
#define HW_CLKCTRL_RESET    (*(volatile uint32_t *)(HW_CLKCTRL_BASE + 0x120))
#define HW_CLKCTRL_RESET_CHIP   0x2
#define HW_CLKCTRL_RESET_DIG    0x1


#endif /* CLKCTRL_IMX233_H */
