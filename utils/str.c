#include "utils/str.h"

void *memset(void *s, int c, unsigned int count) {
    char *xs = (char*)s;

    while (count--)
        *xs++ = c;
    return s;
}   


int strlen(const char *s)
{
    const char *start = s;

    while (*s)
        s++;

    return s - start;
}

char *strcpy(char *s1, const char *s2)
{
    char *s = s1;

    while ((*s1++ = *s2++) != '\0')
        ;

    return s;
}

int strncmp(const char *str1, const char *str2, unsigned int n) {
    unsigned int c=0;
    while(*str1 && *str2 && c<n) {
        c++;
        if(*str1 != *str2)
            return 1;
        str1++;
        str2++;
    }

    if(!*str1 && !*str2)
        return 0;
    if(c >= n)
        return 0;
    return 1;
}

int strcmp(const char *str1, const char *str2) {
    while(*str1 && *str2) {
        if(*str1 != *str2)
            return 1;
        str1++;
        str2++;
    }

    if(!*str1 && !*str2)
        return 0;
    return 1;
}


void *memcpy(void *s1, const void *s2, unsigned int n)
{
    char *dst = (char*)s1;
    const char *src = (const char*)s2;

    while (n-- > 0)
        *dst++ = *src++;

    return s1;
}

