#ifndef __PASSTHRU_H__
#define __PASSTHRU_H__


#include "effects/effect_base.h"


class pass_thru_t : public effect_base_t {
public:
    pass_thru_t();
    virtual int process_sample(int sample, unsigned char channel);
};


#endif /* __PASSTHRU_H__ */

