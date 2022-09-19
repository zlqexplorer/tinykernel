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
#include "ide.h"
#include "dir.h"

//extern struct partition* cur_part;

int main(void){
    put_str("I am kernel\n");
    init_all();
    
    char cwd_buf[32]={0};
    sys_getcwd(cwd_buf,32);
    printf("cwd: %s\n",cwd_buf);
    sys_chdir("/dir1");
    printf("change csd now\n");
    sys_getcwd(cwd_buf,32);
    printf("cwd: %s\n",cwd_buf);
    struct stat obj_stat;
    sys_stat("/",&obj_stat);
    printf("root_dir info:\ni_no: %d\nsize: %d\nfiletype: %s\n",\
obj_stat.st_ino,obj_stat.st_size,obj_stat.st_filetype==2?"directory":"regular");
    while(1);
    return 0;
}
