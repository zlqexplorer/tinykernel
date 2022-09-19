#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"
#include "stdint.h"


void k_thread_a(void*);
void k_thread_b(void*);


int main(void){
    put_str("I am kernel\n");
    init_all();
    ASSERT(thread_start("k_thread_a",31,k_thread_a,"argA ")!=NULL);
    ASSERT(thread_start("k_thread_b",8,k_thread_b,"argB ")!=NULL);

    intr_enable();
    while(1){
        put_str("Main ");
    }

    return 0;
}

void k_thread_a(void* arg){
    char* para=(char*)arg;
    while(1){
        put_str(para);
    }
}

void k_thread_b(void* arg){
    char* para=(char*)arg;
    while(1){
        put_str(para);
    }
}
