#ifndef __UTILS_H__
#define __UTILS_H__


#ifdef __cplusplus
extern "C" {
#endif


int strlen(const char *s);
char *strcpy(char *s1, const char *s2);
int strncmp(const char *str1, const char *str2, unsigned int n);
int strcmp(const char *str1, const char *str2);
void *memcpy(void *s1, const void *s2, unsigned int n);
void *memset(void *s, int c, unsigned int count);


#ifdef __cplusplus
}
#endif


extern unsigned char _ctype[];

#endif /*__UTILS_H__*/
