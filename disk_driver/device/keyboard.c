#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "stdint.h"
#include "ioqueue.h"

#define KBD_BUF_PORT 0x60

#define esc '\033'
#define backspace '\b'
#define tab '\t'
#define enter '\r'
#define delete '\177'

#define char_invisible 0
#define ctrl_l_char char_invisible
#define ctrl_r_char char_invisible
#define shift_l_char char_invisible
#define shift_r_char char_invisible
#define alt_l_char char_invisible
#define alt_r_char char_invisible
#define caps_lock_char char_invisible

#define shift_l_make 0x2a
#define shift_r_make 0x36
#define alt_l_make 0x38
#define alt_r_make 0xe038
#define alt_r_break 0xe0b8
#define ctrl_l_make 0x1d
#define ctrl_r_make 0xe01d
#define ctrl_r_break 0xe09d
#define caps_lock_make 0x3a

static bool ctrl_status,shift_status,alt_status,caps_lock_status,ext_scancode;

static char keymap[][2]={
    {0, 0},
    {esc, esc},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {backspace, backspace},
    {tab, tab},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {enter, enter},
    {ctrl_l_char, ctrl_l_char},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {shift_l_char, shift_l_char},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {shift_r_char, shift_r_char},
    {'*', '*'},
    {alt_r_char, alt_r_char},
    {' ', ' '},
    {caps_lock_char, caps_lock_char}
};

struct ioqueue kbd_buf;

static void intr_keyboard_handler(void){
    //bool ctrl_down_last=ctrl_status;
    bool shift_down_last=shift_status;
    bool caps_lock_last=caps_lock_status;

    uint16_t scancode=inb(KBD_BUF_PORT);
    
    if(scancode==0xe0){
        ext_scancode=true;
        return;
    }
    if(ext_scancode){
        scancode|=0xe000;
        ext_scancode=false;
    }
    if(scancode&0x0080){
        scancode&=0xff7f;
        switch(scancode){
            case ctrl_l_make:
            case ctrl_r_make:
                ctrl_status=false;
                break;
            case shift_l_make:
            case shift_r_make:
                shift_status=false;
                break;
            case alt_l_make:
            case alt_r_make:
                alt_status=false;
                break;
            default:
                ;
        }
        return;
    }
    if((scancode>0x00&&scancode<0x3b)||scancode==alt_r_make||scancode==ctrl_r_make){
        bool shift=false;
        if(scancode<0x0e||scancode==0x1a||scancode==0x1b|| \
            (scancode>=0x27&&scancode<=0x29)||scancode==0x2b|| \
            (scancode>=0x33&&scancode<=0x35)){
            if(shift_down_last){
                shift=true;
            }
        }else{
            if(shift_down_last+caps_lock_last==1){
                shift=true;
            }
        }
        uint8_t index=(scancode&=0x00ff);
        char cur_char=keymap[index][(uint32_t)shift];
        if(cur_char){
            if(!ioq_full(&kbd_buf)){
                put_char(cur_char);
                ioq_putchar(&kbd_buf,cur_char);
            }
            return;
        }
        switch(scancode){
            case ctrl_l_make:
            case ctrl_r_make:
                ctrl_status=true;
                break;
            case shift_l_make:
            case shift_r_make:
                shift_status=true;
                break;
            case alt_l_make:
            case alt_r_make:
                alt_status=true;
                break;
            case caps_lock_make:
                caps_lock_status=!caps_lock_status;
                break;
            default:
                ;
        }
    }else{
        put_str("unknown key\n");
    }
}

void keyboard_init(){
    put_str("keyboard init start\n");
    ioqueue_init(&kbd_buf);
    register_handler(0x21,intr_keyboard_handler);
    put_str("keyboard init done\n");
}   
