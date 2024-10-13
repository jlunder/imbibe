#ifndef __IMBIBE_H_INCLUDED
#define __IMBIBE_H_INCLUDED

#include "base.h"

#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "debug.h"
#include "memory.h"
#include "termviz.h"

namespace sim {

extern void step_poll();
extern void step_idle();
extern void step_animate(uint32_t anim_ms);
extern void step_frame();

} // namespace sim

#endif // __IMBIBE_H_INCLUDED
