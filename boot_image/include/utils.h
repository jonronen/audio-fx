#ifndef __UTILS_H__
#define __UTILS_H__

// Hardware-specific register definitions.
#define HW_RTC_MILLISECONDS 0x8005C020 
#define HW_RTC_MILLISECONDS_SET 0x8005C024 
#define HW_RTC_MILLISECONDS_CLR 0x8005C028 
#define HW_RTC_MILLISECONDS_TOG 0x8005C02C
#define HW_RTC_MILLISECONDS_RD() (*(int *)HW_RTC_MILLISECONDS)



/* minimal implementation of string functions */
/*
static char *strstr(const char *s1, const char *s2)
{
	int i;

	if (*s1 == '\0')
		return *s2 ? 0 : (char *)s1;

	while (*s1) {
		for (i = 0; ; i++) {
			if (s2[i] == '\0')
				return (char *)s1;
			if (s2[i] != s1[i])
				break;
		}
		s1++;
	}
	return 0;
}
*/

int strlen(const char *s);
char *strcpy(char *s1, const char *s2);
int strncmp(const char *str1, const char *str2, int n);
int strcmp(const char *str1, const char *str2);
void *memcpy(void *s1, const void *s2, int n);
void *memset(void *s, int c, int count);
void sys_reboot();

#define TOLOWER(x) ((x) | 0x20)
/*
#define _U  0x01    // upper
#define _L  0x02    // lower
#define _D  0x04    // digit
#define _C  0x08    // cntrl
#define _P  0x10    // punct
#define _S  0x20    // white space (space/lf/tab)
#define _X  0x40    // hex digit
#define _SP 0x80    // hard space (0x20)
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])
#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)  ((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)  ((__ismask(c)&(_C)) != 0)
#define isdigit(c)  ((__ismask(c)&(_D)) != 0)
#define isgraph(c)  ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isprint(c)  ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)  ((__ismask(c)&(_P)) != 0)
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)
*/

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)
#define isdigit(x) (x>='0' && x<='9')
#define isxdigit(x) ((x>='a' && x<='f') || (x>='A' && x<='F') || (isdigit(x)))
#define isprint(x) (x>=0x20 && x<=0x7e)



    // Wait 2us after burning to let it settle, as per Freescale doc.
#define HW_DIGCTL_MICROSECONDS_RD() ((*((int *)0x8001C0C0)))
#define usleep(x) udelay(x)
#define msleep(x) mdelay(x)
#define sleep(x)  delay(x)

/*
#define udelay(x) \
    do { \
        int start_us = HW_DIGCTL_MICROSECONDS_RD(); \
        while(HW_DIGCTL_MICROSECONDS_RD() < start_us+x); \
    } while(0)

#define mdelay(x) \
    do { \
        int start_ms = HW_RTC_MILLISECONDS_RD(); \
        while(HW_RTC_MILLISECONDS_RD() < start_ms+x); \
    } while(0)
*/

#define delay(x) \
    do { \
        int start_ms = HW_RTC_MILLISECONDS_RD(); \
        while(HW_RTC_MILLISECONDS_RD() < start_ms+(x*1000)); \
    } while(0)


extern unsigned char _ctype[];
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);

#endif //__UTILS_H__
