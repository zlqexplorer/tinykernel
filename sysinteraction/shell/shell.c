#include "shell.h"
#include "stdio.h"
#include "stdint.h"
#include "debug.h"
#include "syscall.h"
#include "file.h"
#include "string.h"
#include "fs.h"
#include "builtin_cmd.h"
#include "exec.h"
#include "pipe.h"

#define MAX_ARG_NR 16

static char cmd_line[MAX_PATH_LEN]={0};

char cwd_cache[MAX_PATH_LEN]={0};

char* argv[MAX_ARG_NR]={0};

int32_t argc=-1;

char final_path[MAX_PATH_LEN]={0};

void print_prompt(void){
    printf("[rabbit@localhost %s]$ ",cwd_cache);
}

static void readline(char* buf,int32_t count){
    ASSERT(buf!=NULL&&count>0);
    char* pos=buf;
    while(read(stdin_no,pos,1)!=-1&&(pos-buf)<count){
        switch(*pos){
            case '\n':
            case '\r':
                *pos=0;
                putchar('\n');
                return;

            case '\b':
                if(buf[0]!='\b'){
                    --pos;
                    putchar('\b');
                }
                break;
            
            case ('l'-'a'):
                *pos=0;
                clear();
                print_prompt();
                printf("%s",buf);
                break;
            
            case ('u'-'a'):
                while(buf!=pos){
                    putchar('\b');
                    *pos--=0;
                }
                break;

            default:
                putchar(*pos++);
        }
    }
    printf("readline: can't find enter_key in the cmd_line, max num of char is 128\n");
}

static int32_t cmd_parse(char* cmd_str,char** argv,char token){
    ASSERT(cmd_str!=NULL);
    int32_t arg_idx=0;
    while(arg_idx<MAX_ARG_NR){
        argv[arg_idx++]=NULL;
    }
    char* next=cmd_str;
    int32_t argc=0;
    while(*next){
        while(*next==token){
            ++next;
        }
        if(*next==0){
            break;
        }
        argv[argc]=next;
        while(*next&&*next!=token){
            ++next;
        }
        if(*next){
            *next++=0;
        }
        if(++argc>MAX_ARG_NR){
            return -1;
        }
    }
    return argc;
}

static void cmd_execute(uint32_t argc,char** argv){
    if(!strcmp("ls",argv[0])){
        builtin_ls(argc,argv);
    }else if(!strcmp("cd",argv[0])){
        if(builtin_cd(argc,argv)!=NULL){
            memset(cwd_cache,0,MAX_PATH_LEN);
            strcpy(cwd_cache,final_path);
        }
    }else if(!strcmp("pwd",argv[0])){
        builtin_pwd(argc,argv);
    }else if(!strcmp("ps",argv[0])){
        builtin_ps(argc,argv);
    }else if(!strcmp("clear",argv[0])){
        builtin_clear(argc,argv);
    }else if(!strcmp("mkdir",argv[0])){
        builtin_mkdir(argc,argv);
    }else if(!strcmp("rmdir",argv[0])){
        builtin_rmdir(argc,argv);
    }else if(!strcmp("rm",argv[0])){
        builtin_rm(argc,argv);
    }else if(!strcmp("help",argv[0])){
        help();
    }else{
        int32_t pid=fork();
        if(pid){
            int32_t status;
            int32_t child_pid=wait(&status);
            if(child_pid==-1){
                PANIC("my_shell: no child\n");
            }
            printf("child_pid %d, it's status: %d\n",child_pid,status);
        }else{
            memset(final_path,0,MAX_PATH_LEN);
            make_clear_abs_path(argv[0],final_path);
            argv[0]=final_path;
            struct stat file_stat;
            memset(&file_stat,0,sizeof(struct stat));
            if(stat(argv[0],&file_stat)==-1){
                printf("my_shell: cannot acess %s: NO such file or directory\n",argv[0]);
                exit(-1);
            }else{
                execv(argv[0],argv);
            }
        }
    }
    int32_t arg_idx=0;
    while(arg_idx<MAX_ARG_NR){
        argv[arg_idx++]=NULL;
    } 
}

void my_shell(void){
    cwd_cache[0]='/';
    cwd_cache[1]=0;
    while(1){
        print_prompt();
        memset(final_path,0,MAX_PATH_LEN);
        memset(cmd_line,0,MAX_PATH_LEN);
        readline(cmd_line,MAX_PATH_LEN);
        if(cmd_line[0]==0){
            continue;
        }

        char* pipe_symbol=strchr(cmd_line,'|');
        if(pipe_symbol){
            int32_t fd[2]={-1};
            pipe(fd);
            fd_redirect(1,fd[1]);
            char* each_cmd=cmd_line;
            pipe_symbol=strchr(each_cmd,'|');
            *pipe_symbol=0;
            argc=-1;
            argc=cmd_parse(cmd_line,argv,' ');
            cmd_execute(argc,argv);
            each_cmd=pipe_symbol+1;
            fd_redirect(0,fd[0]);
            while((pipe_symbol=strchr(each_cmd,'|'))){
                *pipe_symbol=0;
                argc=-1;
                argc=cmd_parse(cmd_line,argv,' ');
                cmd_execute(argc,argv);
                each_cmd=pipe_symbol+1;
            }
            fd_redirect(1,1);
            argc=cmd_parse(cmd_line,argv,' ');
            cmd_execute(argc,argv);

            fd_redirect(0,0);
            close(fd[0]);
            close(fd[1]);
        }else{
            argc=-1;
            argc=cmd_parse(cmd_line,argv,' ');
            if(argc==-1){
                printf("num of arg exceed %d\n",MAX_ARG_NR);
            }
            cmd_execute(argc,argv);
        }
    }
    PANIC("my_shell: should not be here\n");
}