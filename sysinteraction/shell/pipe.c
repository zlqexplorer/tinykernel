#include "pipe.h"
#include "fs.h"
#include "file.h"
#include "memory.h"
#include "ioqueue.h"
#include "thread.h"

extern struct file file_table[MAX_FILE_OPEN];

bool is_pipe(uint32_t local_fd){
    uint32_t global_fd=fd_local2global(local_fd);
    return file_table[global_fd].fd_flag==PIPE_FLAG;
}

int32_t sys_pipe(int32_t pipefd[2]){
    int32_t global_fd=get_free_slot_int_global();
    file_table[global_fd].fd_inode=get_kernel_pages(1);
    if(file_table[global_fd].fd_inode==NULL){
        return -1;
    }
    ioqueue_init((struct ioqueue*)file_table[global_fd].fd_inode);
    file_table[global_fd].fd_flag=PIPE_FLAG;
    file_table[global_fd].fd_pos=2;
    pipefd[0]=pcb_fd_install(global_fd);
    pipefd[1]=pcb_fd_install(global_fd);
    return 0;
}

uint32_t pipe_read(int32_t fd,void* buf,uint32_t count){
    char* buffer=buf;
    uint32_t bytes_read=0;
    uint32_t global_fd=fd_local2global(fd);
    struct ioqueue* ioq=(struct ioqueue*)file_table[global_fd].fd_inode;
    uint32_t ioq_len=ioq_length(ioq);
    uint32_t size=ioq_len>count?count:ioq_len;

    while(bytes_read<size){
        *buffer++=ioq_getchar(ioq);
        ++bytes_read;
    }
    return bytes_read;
}

uint32_t pipe_write(int32_t fd,const void* buf,uint32_t count){
    uint32_t bytes_written=0;
    uint32_t global_fd=fd_local2global(fd);
    struct ioqueue* ioq=(struct ioqueue*)file_table[global_fd].fd_inode;
    uint32_t ioq_left=BUFSIZE-ioq_length(ioq);
    uint32_t size=ioq_left>count?count:ioq_left;

    const char* buffer=buf;
    while(bytes_written<size){
        ioq_putchar(ioq,*buffer++);
        ++bytes_written;
    }
    return bytes_written;
}

void sys_fd_redirect(uint32_t old_local_fd,uint32_t new_local_fd){
    struct task_struct* cur=running_thread();
    if(new_local_fd<3){
        cur->fd_table[old_local_fd]=new_local_fd;
    }else{
        cur->fd_table[old_local_fd]=cur->fd_table[new_local_fd];
    }
}