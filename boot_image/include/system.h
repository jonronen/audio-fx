#ifndef __system_h__
#define __system_h__


#ifdef __cplusplus
extern "C" {
#endif
int fx_main();
#ifdef __cplusplus
}
#endif

void system_init();

void udelay(unsigned int us);
static inline void mdelay(unsigned msecs)
{
    udelay(1000 * msecs);
}


#endif /* __system_h__ */

