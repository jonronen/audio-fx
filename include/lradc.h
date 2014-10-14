#ifndef __lradc_h__
#define __lradc_h__


#define MAX_LRADC_CHANNEL 256

#define LRADC_INVALID_VALUE -1.0

#ifdef __cplusplus
extern "C" {
#endif


double lradc_read_channel(const unsigned int channel);
void lradc_setup_channels_for_polling();


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __lradc_h__ */

