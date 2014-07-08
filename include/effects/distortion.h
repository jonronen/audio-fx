#ifndef __DISTORTION_H__
#define __DISTORTION_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define DISTORTION_MAX_LEVEL 0x100


class distortion_t : public effect_base_t {
public:
    distortion_t();
    virtual int process_sample(int sample, unsigned char channel);
    virtual unsigned short translate_level(unsigned short level);
protected:
    double m_dist_level;
};


#endif /* __DISTORTION_H__ */

