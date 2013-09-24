#include "utils/str.h"

void *memset(void *s, int c, int count) {
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

int strncmp(const char *str1, const char *str2, int n) {
    int c=0;
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


void *memcpy(void *s1, const void *s2, int n)
{
	char *dst = (char*)s1;
	const char *src = (const char*)s2;

	while (n-- > 0)
		*dst++ = *src++;

	return s1;
}


unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base) {
    unsigned long result = 0,value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
            cp += 2;
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;

    return result;
}

