#include "print.h"
#include "interrupt.h"
#include "init.h"
#include "timer.h"

void init_all(){
    put_str("init all\n");
    idt_init();
    timer_init();
}
