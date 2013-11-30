#ifndef __lradc_h__
#define __lradc_h__


#define MAX_LRADC_CHANNEL 256

/* TODO: units */
int lradc_read_channel(int channel);
void lradc_setup_channels_for_polling();


#endif /* __lradc_h__ */

