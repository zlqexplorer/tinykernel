#include "fork.h"
#include "stdint.h"
#include "thread.h"
#include "memory.h"
#include "string.h"
#include "global.h"
#include "debug.h"
#include "process.h"
#include "file.h"
#include "interrupt.h"
#include "list.h"
#include "stdio_kernel.h"
#include "pipe.h"

extern void intr_exit(void);
extern struct list thread_ready_list;
extern struct list thread_all_list;
extern struct file file_table[MAX_FILE_OPEN];

static int32_t copy_pcb_vaddrbitmap_stack0( \
struct task_struct* child_thread,struct task_struct* parent_thread){
    memcpy(child_thread,parent_thread,PG_SIZE);
    child_thread->pid=fork_pid();
    child_thread->elapsed_ticks=0;
    child_thread->status=TASK_READY;
    child_thread->ticks=child_thread->priority;
    child_thread->parent_pid=parent_thread->pid;
    child_thread->general_tag.prev=child_thread->general_tag.next=NULL;
    child_thread->all_list_tag.prev=child_thread->all_list_tag.next=NULL;
    /*Different pcb lead to different free list, it can't be easily solved, and may cause the failue of sys_free, don't try to use malloc before fork!*/
    block_desc_init(child_thread->u_block_desc);
    uint32_t bitmap_pg_cnt=DIV_ROUND_UP((0xc0000000-USER_VADDR_START)/PG_SIZE/8,PG_SIZE);
    void* vaddr_btmp=get_kernel_pages(bitmap_pg_cnt);
    memcpy(vaddr_btmp,parent_thread->userprog_vaddr.vaddr_bitmap.bits,bitmap_pg_cnt*PG_SIZE);
    child_thread->userprog_vaddr.vaddr_bitmap.bits=vaddr_btmp;
    ASSERT(strlen(child_thread->name)<11);
    strcat(child_thread->name,"_fork");
    return 0;
}


static void copy_body_stack3(struct task_struct* \
child_thread,struct task_struct* parent_thread,void* buf_page){
    uint8_t* vaddr_btmp=parent_thread->userprog_vaddr.vaddr_bitmap.bits;
    uint32_t btmp_bytes_len=parent_thread->userprog_vaddr.vaddr_bitmap.btmp_bytes_len;
    uint32_t vaddr_start=parent_thread->userprog_vaddr.vaddr_start;
    uint32_t idx_byte=0;
    uint32_t idx_bit=0;
    uint32_t prog_vaddr=0;
    while(idx_byte<btmp_bytes_len){
        if(vaddr_btmp[idx_byte]){
            idx_bit=0;
            while(idx_bit<8){
                if((1<<idx_bit)&vaddr_btmp[idx_byte]){
                    prog_vaddr=vaddr_start+(idx_byte*8+idx_bit)*PG_SIZE;
                    memcpy(buf_page,(void*)prog_vaddr,PG_SIZE);
                    page_dir_activate(child_thread);
                    get_a_page_without_opvaddrbitmap(PF_USER,prog_vaddr);
                    memcpy((void*)prog_vaddr,buf_page,PG_SIZE);
                    page_dir_activate(parent_thread);
                }
                ++idx_bit;
            }
        }
        ++idx_byte;
    }
}

static int32_t build_child_stack(struct task_struct* child_thread){
    struct intr_stack* intr_0_stack=(struct intr_stack*) \
((uint32_t)child_thread+PG_SIZE-sizeof(struct intr_stack));
    intr_0_stack->eax=0;
    uint32_t* ret_addr_in_thread_stack=(uint32_t*)intr_0_stack-1;
    uint32_t* esi_ptr_in_thread_stack=(uint32_t*)intr_0_stack-2;
    uint32_t* edi_ptr_in_thread_stack=(uint32_t*)intr_0_stack-3;
    uint32_t* ebx_ptr_in_thread_stack=(uint32_t*)intr_0_stack-4;
    uint32_t* ebp_ptr_in_thread_stack=(uint32_t*)intr_0_stack-5;

    *ret_addr_in_thread_stack=(uint32_t)intr_exit;
    *ebp_ptr_in_thread_stack=*ebx_ptr_in_thread_stack \
=*edi_ptr_in_thread_stack=*esi_ptr_in_thread_stack=0;
    child_thread->self_kstack=ebp_ptr_in_thread_stack;
    return 0;
}

static void update_inode_open_cnts(struct task_struct* thread){
    int32_t local_fd=3,global_fd=0;
    while(local_fd<MAX_FILES_OPEN_PER_PROC){
        global_fd=thread->fd_table[local_fd];
        ASSERT(global_fd<MAX_FILE_OPEN);
        if(global_fd!=-1){
            if(is_pipe(local_fd)){
                ++file_table[global_fd].fd_pos;
            }else{
                ++file_table[global_fd].fd_inode->i_open_cnts;
            }
        }
        ++local_fd;
    }
}

static int32_t copy_process(struct task_struct* \
child_thread,struct task_struct* parent_thread){
    void* buf_page=get_kernel_pages(1);
    if(buf_page==NULL){
        return -1;
    }
    if(copy_pcb_vaddrbitmap_stack0(child_thread,parent_thread)==-1){
        return -1;
    }
    child_thread->pgdir=create_page_dir();
    if(child_thread->pgdir==NULL){
        return -1;
    }
    copy_body_stack3(child_thread,parent_thread,buf_page);
    build_child_stack(child_thread);
    update_inode_open_cnts(child_thread);
    mfree_page(PF_KERNEL,buf_page,1);
    return 0;
}

pid_t sys_fork(void){
    struct task_struct* parent_thread=running_thread();
    struct task_struct* child_thread=get_kernel_pages(1);
    if(child_thread==NULL){
        return -1;
    }
    ASSERT(INTR_OFF==intr_get_status()&&parent_thread->pgdir!=NULL);
    if(copy_process(child_thread,parent_thread)==-1){
        return -1;
    }
    ASSERT(!elem_find(&thread_all_list,&child_thread->all_list_tag));
    list_append(&thread_all_list,&child_thread->all_list_tag);
    ASSERT(!elem_find(&thread_ready_list,&child_thread->general_tag));
    list_append(&thread_ready_list,&child_thread->general_tag);
    return child_thread->pid;
}
