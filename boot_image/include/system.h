#ifndef __system_h__
#define __system_h__


void system_init();

void udelay(unsigned int us);
static inline void mdelay(unsigned msecs)
{
    udelay(1000 * msecs);
}


#endif /* __system_h__ */

