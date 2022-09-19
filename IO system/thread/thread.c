#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "memory.h"
#include "list.h"
#include "interrupt.h"
#include "debug.h"
#include "print.h"

#define PG_SIZE 4096

struct task_struct* main_thread;
struct list thread_ready_list;
struct list thread_all_list;
static struct list_elem* thread_tag;

extern void switch_to(struct task_struct* cur,struct task_struct* next);

struct task_struct* running_thread(){
    uint32_t esp;
    asm volatile("mov %%esp,%0":"=g"(esp));
    return (struct task_struct*)(esp&0xfffff000);
}

static void kernel_thread(thread_func* function,void *func_arg){
    intr_enable();
    function(func_arg);
}

void thread_create(struct task_struct* pcb,thread_func* function,void* func_arg){
    pcb->self_kstack-=sizeof(struct intr_stack);
    pcb->self_kstack-=sizeof(struct thread_stack);
    struct thread_stack* kthread_stack=(struct thread_stack*)(pcb->self_kstack);
    kthread_stack->ebp=kthread_stack->ebx= \
    kthread_stack->esi=kthread_stack->edi=0;
    kthread_stack->eip=kernel_thread;
    kthread_stack->function=function;
    kthread_stack->func_arg=func_arg;
}

void init_thread(struct task_struct* pcb,char* name,int prio){
    memset(pcb,0,sizeof(*pcb));
    strcpy(pcb->name,name);
    if(pcb==main_thread){
        pcb->status=TASK_RUNNING;
    }else{
        pcb->status=TASK_READY;
    }

    pcb->self_kstack=(uint32_t*)((uint32_t)pcb+PG_SIZE);
    pcb->priority=prio;
    pcb->ticks=prio;
    pcb->elapsed_ticks=0;
    pcb->pgdir=NULL;
    pcb->stack_magic=0x19970508;
}

struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg){
    struct task_struct* pcb=get_kernel_pages(1);
    init_thread(pcb,name,prio);
    thread_create(pcb,function,func_arg);
    ASSERT(!elem_find(&thread_ready_list,&pcb->general_tag));
    list_append(&thread_ready_list,&pcb->general_tag);
    ASSERT(!elem_find(&thread_all_list,&pcb->all_list_tag));
    list_append(&thread_all_list,&pcb->all_list_tag);
    return pcb;
}

static void make_main_thread(void){
    main_thread=running_thread();
    init_thread(main_thread,"main",31);
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
}

void schedule(){
    ASSERT(intr_get_status()==INTR_OFF);
    struct task_struct* cur=running_thread();
    if(cur->status==TASK_RUNNING){
        ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
        list_append(&thread_ready_list,&cur->general_tag);
        cur->ticks=cur->priority;
        cur->status=TASK_READY;
    }else{

    }
    ASSERT(!list_empty(&thread_ready_list));
    thread_tag=list_pop(&thread_ready_list);
    struct task_struct* next=elem2entry(struct task_struct,general_tag,thread_tag);
    next->status=TASK_RUNNING;
    switch_to(cur,next);
}

void thread_block(enum task_status status){
    ASSERT(status==TASK_BLOCKED||status==TASK_WAITING||status==TASK_HANDING);
    enum intr_status old_status=intr_disable();
    struct task_struct* cur=running_thread();
    cur->status=status;
    schedule();
    intr_set_status(old_status);
}

void thread_unblock(struct task_struct* pcb){
    ASSERT(pcb->status==TASK_BLOCKED||pcb->status==TASK_WAITING||pcb->status==TASK_HANDING);
    enum intr_status old_status=intr_disable();
    if(pcb->status!=TASK_READY){
        ASSERT(!elem_find(&thread_ready_list,&pcb->general_tag));
        if(elem_find(&thread_ready_list,&pcb->general_tag)){
            PANIC("thread_unblock: blocked thread in ready_list\n");
        }
        list_push(&thread_ready_list,&pcb->general_tag);
        pcb->status=TASK_READY;
    }
    intr_set_status(old_status);
}

void thread_init(){
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    make_main_thread();
    put_str("thread_init end\n");
}
