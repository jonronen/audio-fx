#ifndef __REGS_LCDIF_H__
#define __REGS_LCDIF_H__

#define REGS_LCDIF_BASE_PHYS (0x80030000)

#define HW_LCDIF_CTRL_WR(x)             (*((int *)(REGS_LCDIF_BASE_PHYS+0x000))=x)
#define HW_LCDIF_CTRL1_WR(x)            (*((int *)(REGS_LCDIF_BASE_PHYS+0x010))=x)
#define HW_LCDIF_NEXT_BUF_WR(x)         (*((int *)(REGS_LCDIF_BASE_PHYS+0x040))=x)
#define HW_LCDIF_CUR_BUF_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x030))=x)
#define HW_LCDIF_TRANSFER_COUNT_WR(x)   (*((int *)(REGS_LCDIF_BASE_PHYS+0x020))=x)
#define HW_LCDIF_TIMING_WR(x)           (*((int *)(REGS_LCDIF_BASE_PHYS+0x060))=x)
#define HW_LCDIF_VDCTRL0_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x070))=x)
#define HW_LCDIF_VDCTRL1_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x080))=x)
#define HW_LCDIF_VDCTRL2_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x090))=x)
#define HW_LCDIF_VDCTRL3_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x0a0))=x)
#define HW_LCDIF_VDCTRL4_WR(x)          (*((int *)(REGS_LCDIF_BASE_PHYS+0x0b0))=x)


#define HW_LCDIF_CUR_BUF_RD()           (*((int *)(REGS_LCDIF_BASE_PHYS+0x030)))




#endif //__REGS_LCDIF_H__
