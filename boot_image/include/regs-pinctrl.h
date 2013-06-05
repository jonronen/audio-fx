#ifndef __REGS_PINCTRL_H__
#define __REGS_PINCTRL_H__


#define REGS_PINCTRL_BASE_PHYS (0x80018000)

#define HW_PINCTRL_MUXSEL2_WR(x)        (*((int *)(REGS_PINCTRL_BASE_PHYS+0x120))=x)
#define HW_PINCTRL_MUXSEL2_RD()         (*((int *)(REGS_PINCTRL_BASE_PHYS+0x120)))
#define HW_PINCTRL_MUXSEL2_SET(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x124))=x)
#define HW_PINCTRL_MUXSEL2_CLR(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x128))=x)
#define HW_PINCTRL_MUXSEL2_TOG(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x12a))=x)

#define HW_PINCTRL_MUXSEL3_WR(x)        (*((int *)(REGS_PINCTRL_BASE_PHYS+0x130))=x)
#define HW_PINCTRL_MUXSEL3_RD()         (*((int *)(REGS_PINCTRL_BASE_PHYS+0x130)))
#define HW_PINCTRL_MUXSEL3_SET(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x134))=x)
#define HW_PINCTRL_MUXSEL3_CLR(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x138))=x)
#define HW_PINCTRL_MUXSEL3_TOG(x)       (*((int *)(REGS_PINCTRL_BASE_PHYS+0x13a))=x)


#define HW_PINCTRL_DOUT1_WR(x)          (*((int *)(REGS_PINCTRL_BASE_PHYS+0x510))=x)
#define HW_PINCTRL_DOUT1_RD()           (*((int *)(REGS_PINCTRL_BASE_PHYS+0x510)))
#define HW_PINCTRL_DOUT1_SET(x)         (*((int *)(REGS_PINCTRL_BASE_PHYS+0x514))=x)
#define HW_PINCTRL_DOUT1_CLR(x)         (*((int *)(REGS_PINCTRL_BASE_PHYS+0x518))=x)
#define HW_PINCTRL_DOUT1_TOG(x)         (*((int *)(REGS_PINCTRL_BASE_PHYS+0x51c))=x)

#define HW_PINCTRL_DOE1_WR(x)           (*((int *)(REGS_PINCTRL_BASE_PHYS+0x710))=x)
#define HW_PINCTRL_DOE1_RD()            (*((int *)(REGS_PINCTRL_BASE_PHYS+0x710)))
#define HW_PINCTRL_DOE1_SET(x)          (*((int *)(REGS_PINCTRL_BASE_PHYS+0x714))=x)
#define HW_PINCTRL_DOE1_CLR(x)          (*((int *)(REGS_PINCTRL_BASE_PHYS+0x718))=x)
#define HW_PINCTRL_DOE1_TOG(x)          (*((int *)(REGS_PINCTRL_BASE_PHYS+0x71C))=x)



#endif //__REGS_PINCTRL_H__
