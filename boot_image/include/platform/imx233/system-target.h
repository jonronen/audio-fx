#ifndef SYSTEM_TARGET_H
#define SYSTEM_TARGET_H

#include "platform/imx233/system-arm.h"
#include "platform/arm/mmu-arm.h"
#include "platform/imx233/clkctrl.h"
#include "platform/imx233/icoll.h"

/* Digital control */
#define HW_DIGCTL_BASE          0x8001C000
#define HW_DIGCTL_CTRL          (*(volatile uint32_t *)(HW_DIGCTL_BASE + 0))
#define HW_DIGCTL_CTRL__USB_CLKGATE (1 << 2)

#define HW_DIGCTL_HCLKCOUNT     (*(volatile uint32_t *)(HW_DIGCTL_BASE + 0x20))

#define HW_DIGCTL_MICROSECONDS  (*(volatile uint32_t *)(HW_DIGCTL_BASE + 0xC0))

#define HW_DIGCTL_ARMCACHE      (*(volatile uint32_t *)(HW_DIGCTL_BASE + 0x2b0))
#define HW_DIGCTL_ARMCACHE__ITAG_SS_BP  0
#define HW_DIGCTL_ARMCACHE__ITAG_SS_BM  (3 << 0)
#define HW_DIGCTL_ARMCACHE__DTAG_SS_BP  4
#define HW_DIGCTL_ARMCACHE__DTAG_SS_BM  (3 << 4)
#define HW_DIGCTL_ARMCACHE__CACHE_SS_BP  8
#define HW_DIGCTL_ARMCACHE__CACHE_SS_BM  (3 << 8)
#define HW_DIGCTL_ARMCACHE__DRTY_SS_BP  12
#define HW_DIGCTL_ARMCACHE__DRTY_SS_BM  (3 << 12)
#define HW_DIGCTL_ARMCACHE__VALID_SS_BP  16
#define HW_DIGCTL_ARMCACHE__VALID_SS_BM  (3 << 16)

/* USB Phy */
#define HW_USBPHY_BASE          0x8007C000 
#define HW_USBPHY_PWD           (*(volatile uint32_t *)(HW_USBPHY_BASE + 0))
#define HW_USBPHY_PWD__ALL      (7 << 10 | 0xf << 17)

#define HW_USBPHY_CTRL          (*(volatile uint32_t *)(HW_USBPHY_BASE + 0x30))

/**
 * Absolute maximum CPU speed: 454.74 MHz
 * Intermediate CPU speeds: 392.73 MHz, 360MHz, 261.82 MHz, 64 MHz
 * Absolute minimum CPU speed: 24 MHz */
#define IMX233_CPUFREQ_454_MHz  454740
#define IMX233_CPUFREQ_392_MHz  392730
#define IMX233_CPUFREQ_360_MHz  360000
#define IMX233_CPUFREQ_261_MHz  261820
#define IMX233_CPUFREQ_64_MHz    64000
#define IMX233_CPUFREQ_24_MHz    24000

#define CPUFREQ_DEFAULT     IMX233_CPUFREQ_64_MHz
#define CPUFREQ_NORMAL      IMX233_CPUFREQ_64_MHz
#define CPUFREQ_MAX         IMX233_CPUFREQ_454_MHz
#define CPUFREQ_SLEEP       IMX233_CPUFREQ_64_MHz

bool imx233_us_elapsed(uint32_t ref, unsigned us_delay);
void imx233_reset_block(volatile uint32_t *block_reg);
void power_off(void);

bool dbg_hw_target_info(void);

#endif /* SYSTEM_TARGET_H */
