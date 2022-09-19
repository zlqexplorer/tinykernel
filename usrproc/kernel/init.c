#include "print.h"
#include "interrupt.h"
#include "init.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"

void init_all(){
    put_str("init all\n");
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
}
