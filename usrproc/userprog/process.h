#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H

#include "thread.h"

#define USER_VADDR_START 0x8048000
#define DEFAULT_PRIO 8

void start_process(void* _filename);
void page_dir_activate(struct task_struct* pcb);
void process_activate(struct task_struct* pcb);
uint32_t* create_page_dir(void);
void create_user_vaddr_bitmap(struct task_struct* upcb);
void process_execute(void* _filename,char* name);


#endif