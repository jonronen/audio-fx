#ifndef __REGS_AESKEY_H__
#define __REGS_AESKEY_H__


#define HW_RTC_PERSISTENT0_RD()         (*((unsigned int *)(0x8005C060)))
#define HW_RTC_PERSISTENT0_SET(x)         (*((unsigned int *)(0x8005C064)) = x)
#define HW_RTC_PERSISTENT0_CLR(x)         (*((unsigned int *)(0x8005C068)) = x)
#define HW_RTC_PERSISTENT1_RD()         (*((unsigned int *)(0x8005C070)))
#define HW_RTC_PERSISTENT1_SET(x)         (*((unsigned int *)(0x8005C074)) = x)
#define HW_RTC_PERSISTENT1_CLR(x)         (*((unsigned int *)(0x8005C078)) = x)
#define HW_RTC_PERSISTENT2_RD()         (*((unsigned int *)(0x8005C080)))
#define HW_RTC_PERSISTENT3_RD()         (*((unsigned int *)(0x8005C090)))
#define HW_RTC_PERSISTENT4_RD()         (*((unsigned int *)(0x8005C0A0)))
#define HW_RTC_PERSISTENT5_RD()         (*((unsigned int *)(0x8005C0B0)))

#define HW_RTC_PERSISTENT2_WR(x)         (*((unsigned int *)(0x8005C080)) = x)
#define HW_RTC_PERSISTENT3_WR(x)         (*((unsigned int *)(0x8005C090)) = x)
#define HW_RTC_PERSISTENT4_WR(x)         (*((unsigned int *)(0x8005C0A0)) = x)
#define HW_RTC_PERSISTENT5_WR(x)         (*((unsigned int *)(0x8005C0B0)) = x)

#define HW_OCOTP_CTRL_RD()           (*((unsigned int *)(0x8002c000)))
#define HW_OCOTP_CTRL_SET(x)         (*((unsigned int *)(0x8002c004)) = x)
#define HW_OCOTP_LOCK_RD()            (*((unsigned int *)(0x8002c120)))

#define HW_OCOTP_CRYPTO3_RD()         (*((unsigned int *)(0x8002c090)))
#define HW_OCOTP_CRYPTO2_RD()         (*((unsigned int *)(0x8002c080)))
#define HW_OCOTP_CRYPTO1_RD()         (*((unsigned int *)(0x8002c070)))
#define HW_OCOTP_CRYPTO0_RD()         (*((unsigned int *)(0x8002c060)))


#endif //__REGS_AESKEY_H__
