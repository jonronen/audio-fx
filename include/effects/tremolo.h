#ifndef __TREMOLO_H__
#define __TREMOLO_H__


#include "effects/effect_base.h"
#include "engine/parameters.h"


#define TREMOLO_MAX_LEVEL 0x100


class tremolo_t : public effect_base_t {
public:
    tremolo_t();
    virtual int process_sample(int sample, unsigned char channel);


private:
    virtual unsigned short translate_lfo(unsigned short lfo_level) const;
};


#endif /* __TREMOLO_H__ */

