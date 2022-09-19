#include "process.h"
#include "stdint.h"
#include "global.h"
#include "memory.h"
#include "debug.h"
#include "tss.h"
#include "console.h"
#include "print.h"
#include "string.h"
#include "bitmap.h"
#include "interrupt.h"
#include "list.h"
#include "stdio_kernel.h"


extern void intr_exit(void);
extern struct list thread_ready_list,thread_all_list;

void start_process(void* _filename){
    void* function=_filename;
    struct task_struct* cur=running_thread();
    cur->self_kstack+=sizeof(struct thread_stack);
    struct intr_stack* proc_stack=(struct intr_stack*)cur->self_kstack;
    proc_stack->edi=proc_stack->esi=proc_stack->ebp=proc_stack->esp_dummy=0;
    proc_stack->ebx=proc_stack->edx=proc_stack->ecx=proc_stack->eax=0;
    proc_stack->gs=0;
    proc_stack->ds=proc_stack->es=proc_stack->fs=SELECTOR_U_DATA;
    proc_stack->eip=function;
    proc_stack->cs=SELECTOR_U_CODE;
    proc_stack->eflags=EFLAGS_IOPL_0|EFLAGS_MBS|EFLAGS_IF_1;
    proc_stack->esp=(void*)((uint32_t)get_a_page(PF_USER,USER_STACK3_VADDR)+PG_SIZE);
    proc_stack->ss=SELECTOR_U_DATA;
    asm volatile("mov %0,%%esp;jmp intr_exit"::"g"(proc_stack):"memory");
}

void page_dir_activate(struct task_struct* pcb){
    uint32_t paddr=0x100000;
    if(pcb->pgdir!=NULL){
        paddr=addr_v2p((uint32_t)pcb->pgdir);
    }
    asm volatile("mov %0,%%cr3"::"r"(paddr):"memory");
}

void process_activate(struct task_struct* pcb){
    ASSERT(pcb!=NULL);
    page_dir_activate(pcb);
    if(pcb->pgdir){
        update_tss_esp(pcb);
    }
}

uint32_t* create_page_dir(){
    uint32_t* kvaddr=get_kernel_pages(1);
    if(kvaddr==NULL){
        console_put_str("create_page_dir: get_kernel_pages failed!\n");
        return NULL;
    }
    memcpy((uint32_t*)((uint32_t)kvaddr+0x300*4), \
(uint32_t*)(0xfffff000+0x300*4),1024);
    uint32_t pvaddr=addr_v2p((uint32_t)kvaddr);
    kvaddr[1023]=pvaddr|PG_US_U|PG_RW_W|PG_P_1;
    return kvaddr;
}

void create_user_vaddr_bitmap(struct task_struct* upcb){
    upcb->userprog_vaddr.vaddr_start=USER_VADDR_START;
    uint32_t bitmap_pg_cnt=DIV_ROUND_UP((0xc0000000-USER_VADDR_START)/PG_SIZE/8,PG_SIZE);
    upcb->userprog_vaddr.vaddr_bitmap.bits=get_kernel_pages(bitmap_pg_cnt);
    upcb->userprog_vaddr.vaddr_bitmap.btmp_bytes_len= \
DIV_ROUND_UP((0xc0000000-USER_VADDR_START)/PG_SIZE,8);
    bitmap_init(&upcb->userprog_vaddr.vaddr_bitmap);
}

void process_execute(void* _filename,char* name){
    struct task_struct* upcb=get_kernel_pages(1);
    init_thread(upcb,name,DEFAULT_PRIO);
    create_user_vaddr_bitmap(upcb);
    thread_create(upcb,start_process,_filename);
    upcb->pgdir=create_page_dir();
    block_desc_init(upcb->u_block_desc);

    enum intr_status old_status=intr_disable();
    ASSERT(!elem_find(&thread_ready_list,&upcb->general_tag));
    list_append(&thread_ready_list,&upcb->general_tag);
    ASSERT(!elem_find(&thread_all_list,&upcb->all_list_tag));
    list_append(&thread_all_list,&upcb->all_list_tag);
    intr_set_status(old_status);
}
