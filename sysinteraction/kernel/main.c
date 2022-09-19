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
#include "fs.h"
#include "string.h"
#include "fork.h"
#include "shell.h"
#include "ide.h"

extern struct ide_channel channels[2];


int main(void){
    put_str("I am kernel\n");
    init_all();
    uint32_t file_size=6208;
    uint32_t sec_cnt=(file_size+511)/512;
    struct disk* sda=&channels[0].devices[0];
    void* prog_buf=sys_malloc(file_size);
    ide_read(sda,300,prog_buf,sec_cnt);
    int32_t fd=sys_open("/morecat",O_CREAT|O_RDWR);
    if(fd!=-1){
        if(sys_write(fd,prog_buf,file_size)==-1){
            printk("file write error\n");
            while(1);
        }
    }
    sys_free(prog_buf);
    cls_screen();
    console_put_str("[rabbit@localhost /]$ ");
    intr_enable();
    thread_exit(running_thread(),true);
    return 0;
}

void init(void){
    uint32_t ret_pid=fork();
    if(ret_pid){
        int32_t status;
        int32_t child_pid;
        while(1){
            child_pid=wait(&status);
            printf("I am init, my pid is 1, I receive a child, whose pid is %d and status is %d\n",child_pid,status);
            print_prompt();
        }
    }else{
        my_shell();
   }
    PANIC("init: should not be here\n");
}
