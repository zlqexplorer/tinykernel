#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "memory.h"
#include "list.h"
#include "interrupt.h"
#include "debug.h"
#include "print.h"
#include "process.h"
#include "sync.h"
#include "stdio.h"
#include "fs.h"
#include "file.h"
#include "bitmap.h"

#define PG_SIZE 4096

struct task_struct* main_thread;
struct list thread_ready_list;
struct list thread_all_list;
static struct list_elem* thread_tag;

struct task_struct* idle_thread;

uint8_t pid_bitmap_bits[128]={0};

struct pid_pool{
    struct bitmap pid_bitmap;
    uint32_t pid_start;
    struct lock pid_lock;
}pid_pool;

static void pid_pool_init(void){
    pid_pool.pid_start=1;
    pid_pool.pid_bitmap.bits=pid_bitmap_bits;
    pid_pool.pid_bitmap.btmp_bytes_len=128;
    bitmap_init(&pid_pool.pid_bitmap);
    lock_init(&pid_pool.pid_lock);
}

static pid_t allocate_pid(void){
    lock_acquire(&pid_pool.pid_lock);
    int32_t bit_idx=bitmap_scan(&pid_pool.pid_bitmap,1);
    bitmap_set(&pid_pool.pid_bitmap,bit_idx,1);
    lock_release(&pid_pool.pid_lock);
    return (bit_idx+pid_pool.pid_start);
}

static void release_pid(pid_t pid){
    lock_acquire(&pid_pool.pid_lock);
    int32_t bit_idx=pid-pid_pool.pid_start;
    bitmap_set(&pid_pool.pid_bitmap,bit_idx,0);
    lock_release(&pid_pool.pid_lock);
}

pid_t fork_pid(void){
    return allocate_pid();
}

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
    pcb->parent_pid=-1;
    pcb->pid=allocate_pid();
    pcb->priority=prio;
    pcb->ticks=prio;
    pcb->elapsed_ticks=0;
    pcb->fd_table[0]=0;
    pcb->fd_table[1]=1;
    pcb->fd_table[2]=2;
    uint8_t fd_idx=3;
    while(fd_idx<MAX_FILES_OPEN_PER_PROC){
        pcb->fd_table[fd_idx]=-1;
        ++fd_idx;
    }
    pcb->pgdir=NULL;
    pcb->cwd_inode_nr=0;
    pcb->exit_status=0;
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

static void idle(void* arg __attribute__((unused))){
    while(1){
        thread_block(TASK_BLOCKED);
        asm volatile("sti;hlt":::"memory");
    }
}

void schedule(){
    ASSERT(intr_get_status()==INTR_OFF);
    struct task_struct* cur=running_thread();
    if(cur->status==TASK_RUNNING){
        ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
        list_append(&thread_ready_list,&cur->general_tag);
        cur->ticks=cur->priority;
        cur->status=TASK_READY;
    }
    if(list_empty(&thread_ready_list)){
        thread_unblock(idle_thread);
    }
    thread_tag=list_pop(&thread_ready_list);
    struct task_struct* next=elem2entry(struct task_struct,general_tag,thread_tag);
    next->status=TASK_RUNNING;
    process_activate(next);
    switch_to(cur,next);
}

void thread_yield(void){
    struct task_struct* cur=running_thread();
    enum intr_status old_status=intr_disable();
    ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
    list_append(&thread_ready_list,&cur->general_tag);
    cur->status=TASK_READY;
    schedule();
    intr_set_status(old_status);
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

void thread_exit(struct task_struct* thread_over,bool need_schedule){
    enum intr_status old_status=intr_disable();
    thread_over->status=TASK_DIED;
    if(elem_find(&thread_ready_list,&thread_over->general_tag)){
        list_remove(&thread_over->general_tag);
    }
    if(thread_over->pgdir!=NULL){
        mfree_page(PF_KERNEL,thread_over->pgdir,1);
    }
    list_remove(&thread_over->all_list_tag);
    release_pid(thread_over->pid);
    if(thread_over!=main_thread){
        mfree_page(PF_KERNEL,thread_over,1);
    }
    if(need_schedule){
        schedule();
        PANIC("thread_exit: should not be here\n");
    }
    intr_set_status(old_status);
}

static bool pid_check(struct list_elem* pelem,int32_t pid){
    struct task_struct* pthread=elem2entry(struct task_struct,all_list_tag,pelem);
    if(pthread->pid==pid){
        return true;
    }
    return false;
}

struct task_struct* pid2thread(int32_t pid){
    struct list_elem* pelem=list_traversal(&thread_all_list,pid_check,pid);
    if(pelem==NULL){
        return NULL;
    }
    struct task_struct* thread=elem2entry(struct task_struct,all_list_tag,pelem);
    return thread;
}

static void pad_print(char* buf,int32_t buf_len,char* ptr,char format){
    memset(buf,0,buf_len);
    uint8_t out_pad_0idx=0;
    switch(format){
        case 's':
            out_pad_0idx=sprintf(buf,"%s",ptr);
            break;
        case 'd':
            out_pad_0idx=sprintf(buf,"%d",*((uint32_t*)ptr));
    }
    while(out_pad_0idx<buf_len){
        buf[out_pad_0idx++]=' ';
    }
    sys_write(stdout_no,buf,buf_len-1);
}

static bool elem2thread_info(struct list_elem* pelem,int arg __attribute__((unused))){
    struct task_struct* pthread=elem2entry(struct task_struct,all_list_tag,pelem);
    char out_pad[16];
    pad_print(out_pad,16,(char*)(&pthread->pid),'d');
    if(pthread->parent_pid==-1){
        pad_print(out_pad,16,"NULL",'s');
    }else{
        pad_print(out_pad,16,(char*)(&pthread->parent_pid),'d');
    }
    switch(pthread->status){
        case TASK_RUNNING:
            pad_print(out_pad,16,"RUNNING",'s');
            break;
        case TASK_READY:
            pad_print(out_pad,16,"READY",'s');
            break;
        case TASK_BLOCKED:
            pad_print(out_pad,16,"BLOCKED",'s');
            break;
        case TASK_WAITING:
            pad_print(out_pad,16,"WAITING",'s');
            break;
        case TASK_HANDING:
            pad_print(out_pad,16,"HANDING",'s');
            break;
        case TASK_DIED:
            pad_print(out_pad,16,"DIED",'s');
            break;
        default:
               ;
    }
    pad_print(out_pad,16,(char*)(&pthread->elapsed_ticks),'d');
    memset(out_pad,0,16);
    ASSERT(strlen(pthread->name)<15);
    strcpy(out_pad,pthread->name);
    strcat(out_pad,"\n");
    sys_write(stdout_no,out_pad,strlen(out_pad));
    return false;
}

void sys_ps(void){
    char* ps_title="PID          PPID          STAT          TICKS          COMMAND\n";
    sys_write(stdout_no,ps_title,strlen(ps_title));
    list_traversal(&thread_all_list,elem2thread_info,0);
}

void thread_init(){
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    pid_pool_init();
    process_execute(init,"init");
    make_main_thread();
    idle_thread=thread_start("idle",10,idle,NULL);
    put_str("thread_init end\n");
}
