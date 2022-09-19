#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"
#include "stdint.h"
#include "console.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "process.h"
#include "syscall_init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "stdio_kernel.h"


void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);

int main(void){
    put_str("I am kernel\n");
    init_all();
    process_execute(u_prog_a,"u_prog_a");
    process_execute(u_prog_b,"u_prog_b");
    printk(" main_pid:0x %x\n",sys_getpid());
    thread_start("consumer_a",31,k_thread_a,"argA ");
    thread_start("consumer_b",31,k_thread_b,"argB ");
    
    intr_enable();
    while(1);
    return 0;
}


void k_thread_a(void* arg){
    void *addr1,*addr2,*addr3;
    addr1=sys_malloc(256);
    addr2=sys_malloc(255);
    addr3=sys_malloc(254);
    printk(" thread_a malloc addr:0x %x,0x %x,0x %x\n",\
(uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while(1);
}

void k_thread_b(void* arg){
    void *addr1,*addr2,*addr3;
    addr1=sys_malloc(256);
    addr2=sys_malloc(255);
    addr3=sys_malloc(254);
    printk(" thread_b malloc addr:0x %x,0x %x,0x %x\n",\
(uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while(1);
}

void u_prog_a(void){
    void *addr1,*addr2,*addr3;
    addr1=malloc(256);
    addr2=malloc(255);
    addr3=malloc(254);
    printf(" prog_a malloc addr:0x %x,0x %x,0x %x\n",\
(uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}

void u_prog_b(void){
    void *addr1,*addr2,*addr3;
    addr1=malloc(256);
    addr2=malloc(255);
    addr3=malloc(254);
    printf(" prog_b malloc addr:0x %x,0x %x,0x %x\n",\
(uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}
