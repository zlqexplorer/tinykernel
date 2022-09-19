%line 1+1 intr_table.S
[bits 32]

%line 5+1 intr_table.S

[extern put_str]

[section .data]
intr_str db "interrupt occur!",0xa,0
[global intr_entry_table]
intr_entry_table:

%line 32+1 intr_table.S

[section .text]
%line 33+0 intr_table.S
intr0x00entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x00entry

%line 34+1 intr_table.S
[section .text]
%line 34+0 intr_table.S
intr0x01entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x01entry

%line 35+1 intr_table.S
[section .text]
%line 35+0 intr_table.S
intr0x02entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x02entry

%line 36+1 intr_table.S
[section .text]
%line 36+0 intr_table.S
intr0x03entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x03entry

%line 37+1 intr_table.S
[section .text]
%line 37+0 intr_table.S
intr0x04entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x04entry

%line 38+1 intr_table.S
[section .text]
%line 38+0 intr_table.S
intr0x05entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x05entry

%line 39+1 intr_table.S
[section .text]
%line 39+0 intr_table.S
intr0x06entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x06entry

%line 40+1 intr_table.S
[section .text]
%line 40+0 intr_table.S
intr0x07entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x07entry

%line 41+1 intr_table.S
[section .text]
%line 41+0 intr_table.S
intr0x08entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x08entry

%line 42+1 intr_table.S
[section .text]
%line 42+0 intr_table.S
intr0x09entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x09entry

%line 43+1 intr_table.S
[section .text]
%line 43+0 intr_table.S
intr0x0aentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0aentry

%line 44+1 intr_table.S
[section .text]
%line 44+0 intr_table.S
intr0x0bentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0bentry

%line 45+1 intr_table.S
[section .text]
%line 45+0 intr_table.S
intr0x0centry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0centry

%line 46+1 intr_table.S
[section .text]
%line 46+0 intr_table.S
intr0x0dentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0dentry

%line 47+1 intr_table.S
[section .text]
%line 47+0 intr_table.S
intr0x0eentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0eentry

%line 48+1 intr_table.S
[section .text]
%line 48+0 intr_table.S
intr0x0fentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x0fentry

%line 49+1 intr_table.S
[section .text]
%line 49+0 intr_table.S
intr0x10entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x10entry

%line 50+1 intr_table.S
[section .text]
%line 50+0 intr_table.S
intr0x11entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x11entry

%line 51+1 intr_table.S
[section .text]
%line 51+0 intr_table.S
intr0x12entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x12entry

%line 52+1 intr_table.S
[section .text]
%line 52+0 intr_table.S
intr0x13entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x13entry

%line 53+1 intr_table.S
[section .text]
%line 53+0 intr_table.S
intr0x14entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x14entry

%line 54+1 intr_table.S
[section .text]
%line 54+0 intr_table.S
intr0x15entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x15entry

%line 55+1 intr_table.S
[section .text]
%line 55+0 intr_table.S
intr0x16entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x16entry

%line 56+1 intr_table.S
[section .text]
%line 56+0 intr_table.S
intr0x17entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x17entry

%line 57+1 intr_table.S
[section .text]
%line 57+0 intr_table.S
intr0x18entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x18entry

%line 58+1 intr_table.S
[section .text]
%line 58+0 intr_table.S
intr0x19entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x19entry

%line 59+1 intr_table.S
[section .text]
%line 59+0 intr_table.S
intr0x1aentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1aentry

%line 60+1 intr_table.S
[section .text]
%line 60+0 intr_table.S
intr0x1bentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1bentry

%line 61+1 intr_table.S
[section .text]
%line 61+0 intr_table.S
intr0x1centry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1centry

%line 62+1 intr_table.S
[section .text]
%line 62+0 intr_table.S
intr0x1dentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1dentry

%line 63+1 intr_table.S
[section .text]
%line 63+0 intr_table.S
intr0x1eentry:
 nop
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1eentry

%line 64+1 intr_table.S
[section .text]
%line 64+0 intr_table.S
intr0x1fentry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x1fentry

%line 65+1 intr_table.S
[section .text]
%line 65+0 intr_table.S
intr0x20entry:
 push 0
 push intr_str
 call put_str
 add esp,4

 mov al,0x20
 out 0xa0,al
 out 0x20,al

 add esp,4
 iret

[section .data]
 dd intr0x20entry

