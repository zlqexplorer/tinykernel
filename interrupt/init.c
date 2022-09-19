#include "print.h"
#include "interrupt.h"
#include "init.h"

void init_all(){
    put_str("init all\n");
    idt_init();
}
