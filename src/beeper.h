#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <zephyr.h>
int beep_off();
int beep_on(int32_t pitch);
int beep(s32_t duration, s32_t pitch);
int beepn(s32_t duration, s32_t count, s32_t pitch);

#endif
