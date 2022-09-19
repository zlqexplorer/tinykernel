%line 1+1 intr_table.S
[bits 32]

%line 5+1 intr_table.S

[extern idt_table]

[section .data]
[global intr_entry_table]
intr_entry_table:

%line 35+1 intr_table.S

[section .text]
[global intr_exit]
intr_exit:
 add esp,4
 popad
 pop gs
 pop fs
 pop es
 pop ds
 add esp,4
 iretd

[section .text]
%line 48+0 intr_table.S
intr0x00entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x00

 call [idt_table+0x00*4]
 jmp intr_exit

[section .data]
 dd intr0x00entry

%line 49+1 intr_table.S
[section .text]
%line 49+0 intr_table.S
intr0x01entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x01

 call [idt_table+0x01*4]
 jmp intr_exit

[section .data]
 dd intr0x01entry

%line 50+1 intr_table.S
[section .text]
%line 50+0 intr_table.S
intr0x02entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x02

 call [idt_table+0x02*4]
 jmp intr_exit

[section .data]
 dd intr0x02entry

%line 51+1 intr_table.S
[section .text]
%line 51+0 intr_table.S
intr0x03entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x03

 call [idt_table+0x03*4]
 jmp intr_exit

[section .data]
 dd intr0x03entry

%line 52+1 intr_table.S
[section .text]
%line 52+0 intr_table.S
intr0x04entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x04

 call [idt_table+0x04*4]
 jmp intr_exit

[section .data]
 dd intr0x04entry

%line 53+1 intr_table.S
[section .text]
%line 53+0 intr_table.S
intr0x05entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x05

 call [idt_table+0x05*4]
 jmp intr_exit

[section .data]
 dd intr0x05entry

%line 54+1 intr_table.S
[section .text]
%line 54+0 intr_table.S
intr0x06entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x06

 call [idt_table+0x06*4]
 jmp intr_exit

[section .data]
 dd intr0x06entry

%line 55+1 intr_table.S
[section .text]
%line 55+0 intr_table.S
intr0x07entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x07

 call [idt_table+0x07*4]
 jmp intr_exit

[section .data]
 dd intr0x07entry

%line 56+1 intr_table.S
[section .text]
%line 56+0 intr_table.S
intr0x08entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x08

 call [idt_table+0x08*4]
 jmp intr_exit

[section .data]
 dd intr0x08entry

%line 57+1 intr_table.S
[section .text]
%line 57+0 intr_table.S
intr0x09entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x09

 call [idt_table+0x09*4]
 jmp intr_exit

[section .data]
 dd intr0x09entry

%line 58+1 intr_table.S
[section .text]
%line 58+0 intr_table.S
intr0x0aentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0a

 call [idt_table+0x0a*4]
 jmp intr_exit

[section .data]
 dd intr0x0aentry

%line 59+1 intr_table.S
[section .text]
%line 59+0 intr_table.S
intr0x0bentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0b

 call [idt_table+0x0b*4]
 jmp intr_exit

[section .data]
 dd intr0x0bentry

%line 60+1 intr_table.S
[section .text]
%line 60+0 intr_table.S
intr0x0centry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0c

 call [idt_table+0x0c*4]
 jmp intr_exit

[section .data]
 dd intr0x0centry

%line 61+1 intr_table.S
[section .text]
%line 61+0 intr_table.S
intr0x0dentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0d

 call [idt_table+0x0d*4]
 jmp intr_exit

[section .data]
 dd intr0x0dentry

%line 62+1 intr_table.S
[section .text]
%line 62+0 intr_table.S
intr0x0eentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0e

 call [idt_table+0x0e*4]
 jmp intr_exit

[section .data]
 dd intr0x0eentry

%line 63+1 intr_table.S
[section .text]
%line 63+0 intr_table.S
intr0x0fentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x0f

 call [idt_table+0x0f*4]
 jmp intr_exit

[section .data]
 dd intr0x0fentry

%line 64+1 intr_table.S
[section .text]
%line 64+0 intr_table.S
intr0x10entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x10

 call [idt_table+0x10*4]
 jmp intr_exit

[section .data]
 dd intr0x10entry

%line 65+1 intr_table.S
[section .text]
%line 65+0 intr_table.S
intr0x11entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x11

 call [idt_table+0x11*4]
 jmp intr_exit

[section .data]
 dd intr0x11entry

%line 66+1 intr_table.S
[section .text]
%line 66+0 intr_table.S
intr0x12entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x12

 call [idt_table+0x12*4]
 jmp intr_exit

[section .data]
 dd intr0x12entry

%line 67+1 intr_table.S
[section .text]
%line 67+0 intr_table.S
intr0x13entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x13

 call [idt_table+0x13*4]
 jmp intr_exit

[section .data]
 dd intr0x13entry

%line 68+1 intr_table.S
[section .text]
%line 68+0 intr_table.S
intr0x14entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x14

 call [idt_table+0x14*4]
 jmp intr_exit

[section .data]
 dd intr0x14entry

%line 69+1 intr_table.S
[section .text]
%line 69+0 intr_table.S
intr0x15entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x15

 call [idt_table+0x15*4]
 jmp intr_exit

[section .data]
 dd intr0x15entry

%line 70+1 intr_table.S
[section .text]
%line 70+0 intr_table.S
intr0x16entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x16

 call [idt_table+0x16*4]
 jmp intr_exit

[section .data]
 dd intr0x16entry

%line 71+1 intr_table.S
[section .text]
%line 71+0 intr_table.S
intr0x17entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x17

 call [idt_table+0x17*4]
 jmp intr_exit

[section .data]
 dd intr0x17entry

%line 72+1 intr_table.S
[section .text]
%line 72+0 intr_table.S
intr0x18entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x18

 call [idt_table+0x18*4]
 jmp intr_exit

[section .data]
 dd intr0x18entry

%line 73+1 intr_table.S
[section .text]
%line 73+0 intr_table.S
intr0x19entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x19

 call [idt_table+0x19*4]
 jmp intr_exit

[section .data]
 dd intr0x19entry

%line 74+1 intr_table.S
[section .text]
%line 74+0 intr_table.S
intr0x1aentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1a

 call [idt_table+0x1a*4]
 jmp intr_exit

[section .data]
 dd intr0x1aentry

%line 75+1 intr_table.S
[section .text]
%line 75+0 intr_table.S
intr0x1bentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1b

 call [idt_table+0x1b*4]
 jmp intr_exit

[section .data]
 dd intr0x1bentry

%line 76+1 intr_table.S
[section .text]
%line 76+0 intr_table.S
intr0x1centry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1c

 call [idt_table+0x1c*4]
 jmp intr_exit

[section .data]
 dd intr0x1centry

%line 77+1 intr_table.S
[section .text]
%line 77+0 intr_table.S
intr0x1dentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1d

 call [idt_table+0x1d*4]
 jmp intr_exit

[section .data]
 dd intr0x1dentry

%line 78+1 intr_table.S
[section .text]
%line 78+0 intr_table.S
intr0x1eentry:
 nop
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1e

 call [idt_table+0x1e*4]
 jmp intr_exit

[section .data]
 dd intr0x1eentry

%line 79+1 intr_table.S
[section .text]
%line 79+0 intr_table.S
intr0x1fentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x1f

 call [idt_table+0x1f*4]
 jmp intr_exit

[section .data]
 dd intr0x1fentry

%line 80+1 intr_table.S
[section .text]
%line 80+0 intr_table.S
intr0x20entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x20

 call [idt_table+0x20*4]
 jmp intr_exit

[section .data]
 dd intr0x20entry

%line 81+1 intr_table.S
[section .text]
%line 81+0 intr_table.S
intr0x21entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x21

 call [idt_table+0x21*4]
 jmp intr_exit

[section .data]
 dd intr0x21entry

%line 82+1 intr_table.S
[section .text]
%line 82+0 intr_table.S
intr0x22entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x22

 call [idt_table+0x22*4]
 jmp intr_exit

[section .data]
 dd intr0x22entry

%line 83+1 intr_table.S
[section .text]
%line 83+0 intr_table.S
intr0x23entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x23

 call [idt_table+0x23*4]
 jmp intr_exit

[section .data]
 dd intr0x23entry

%line 84+1 intr_table.S
[section .text]
%line 84+0 intr_table.S
intr0x24entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x24

 call [idt_table+0x24*4]
 jmp intr_exit

[section .data]
 dd intr0x24entry

%line 85+1 intr_table.S
[section .text]
%line 85+0 intr_table.S
intr0x25entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x25

 call [idt_table+0x25*4]
 jmp intr_exit

[section .data]
 dd intr0x25entry

%line 86+1 intr_table.S
[section .text]
%line 86+0 intr_table.S
intr0x26entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x26

 call [idt_table+0x26*4]
 jmp intr_exit

[section .data]
 dd intr0x26entry

%line 87+1 intr_table.S
[section .text]
%line 87+0 intr_table.S
intr0x27entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x27

 call [idt_table+0x27*4]
 jmp intr_exit

[section .data]
 dd intr0x27entry

%line 88+1 intr_table.S
[section .text]
%line 88+0 intr_table.S
intr0x28entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x28

 call [idt_table+0x28*4]
 jmp intr_exit

[section .data]
 dd intr0x28entry

%line 89+1 intr_table.S
[section .text]
%line 89+0 intr_table.S
intr0x29entry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x29

 call [idt_table+0x29*4]
 jmp intr_exit

[section .data]
 dd intr0x29entry

%line 90+1 intr_table.S
[section .text]
%line 90+0 intr_table.S
intr0x2aentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2a

 call [idt_table+0x2a*4]
 jmp intr_exit

[section .data]
 dd intr0x2aentry

%line 91+1 intr_table.S
[section .text]
%line 91+0 intr_table.S
intr0x2bentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2b

 call [idt_table+0x2b*4]
 jmp intr_exit

[section .data]
 dd intr0x2bentry

%line 92+1 intr_table.S
[section .text]
%line 92+0 intr_table.S
intr0x2centry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2c

 call [idt_table+0x2c*4]
 jmp intr_exit

[section .data]
 dd intr0x2centry

%line 93+1 intr_table.S
[section .text]
%line 93+0 intr_table.S
intr0x2dentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2d

 call [idt_table+0x2d*4]
 jmp intr_exit

[section .data]
 dd intr0x2dentry

%line 94+1 intr_table.S
[section .text]
%line 94+0 intr_table.S
intr0x2eentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2e

 call [idt_table+0x2e*4]
 jmp intr_exit

[section .data]
 dd intr0x2eentry

%line 95+1 intr_table.S
[section .text]
%line 95+0 intr_table.S
intr0x2fentry:
 push 0
 push ds
 push es
 push fs
 push gs
 pushad

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 push 0x2f

 call [idt_table+0x2f*4]
 jmp intr_exit

[section .data]
 dd intr0x2fentry

