#include "sync.h"
#include "list.h"
#include "debug.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"

void sema_init(struct semaphore* sem,uint8_t val){
    sem->value=val;
    list_init(&sem->waiters);
}

void lock_init(struct lock* plock){
    plock->holder=NULL;
    plock->holder_repeat_nr=0;
    sema_init(&plock->sem,1);
}

void sema_down(struct semaphore* sem){
    struct task_struct* cur=running_thread();
    enum intr_status old_status=intr_disable();
    while(sem->value==0){
        ASSERT(!elem_find(&sem->waiters,&cur->general_tag));
        if(elem_find(&sem->waiters,&cur->general_tag)){
            PANIC("sema_sown: thread blocked has been in waiters_list\n");
        }
        list_append(&sem->waiters,&cur->general_tag);
        thread_block(TASK_BLOCKED);
    }
    --sem->value;
    ASSERT(sem->value==0);
    intr_set_status(old_status);
}

void sema_up(struct semaphore* sem){
    enum intr_status old_status=intr_disable();
    ASSERT(sem->value==0);
    ++sem->value;
    if(!list_empty(&sem->waiters)){
        struct task_struct* to_wake=elem2entry(struct task_struct, \
            general_tag,list_pop(&sem->waiters));
        thread_unblock(to_wake);
    }
    intr_set_status(old_status);
}

void lock_acquire(struct lock* plock){
    if(plock->holder!=running_thread()){
        sema_down(&plock->sem);
        plock->holder=running_thread();
        ASSERT(plock->holder_repeat_nr==0);
        plock->holder_repeat_nr=1;
    }else{
        ++plock->holder_repeat_nr;
    }
}

void lock_release(struct lock* plock){
    ASSERT(plock->holder==running_thread());
    if(plock->holder!=running_thread()){
        PANIC("lock_release: thread want to release lock before acquire\n");
    }
    if(--plock->holder_repeat_nr==0){
        plock->holder=NULL;
        sema_up(&plock->sem);
    }
}
