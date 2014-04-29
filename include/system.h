#ifndef __system_h__
#define __system_h__


#ifdef __cplusplus
extern "C" {
#endif


int fx_main();

void system_init();

void udelay(unsigned int us);
static inline void mdelay(unsigned msecs)
{
    udelay(1000 * msecs);
}


#ifdef __cplusplus
}
#endif


#endif /* __system_h__ */

