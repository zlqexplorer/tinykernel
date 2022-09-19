#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include "stdint.h"

void mtime_sleep(uint32_t m_seconds);
void timer_init(void);

#endif
