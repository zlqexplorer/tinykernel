#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H

#include "list.h"
#include "stdint.h"
#include "thread.h"

struct semaphore{
    uint8_t value;
    struct list waiters;
};

struct lock{
    struct task_struct* holder;
    struct semaphore sem;
    uint32_t holder_repeat_nr;
};

void sema_init(struct semaphore* sem,uint8_t val);
void lock_init(struct lock* plock);
void sema_down(struct semaphore* sem);
void sema_up(struct semaphore* sem);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif