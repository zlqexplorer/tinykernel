#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H

#include "thread.h"

void update_tss_esp(struct task_struct* pcb);
void tss_init(void);

#endif