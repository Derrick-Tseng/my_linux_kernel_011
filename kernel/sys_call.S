.code32
.text
.globl system_call, timer_interrupt

EAX        = 0x00
EBX        = 0x04
ECX        = 0x08
EDX        = 0x0C
FS         = 0x10
ES         = 0x14
DS         = 0x18
EIP        = 0x1c
CS         = 0x20
EFLAGS     = 0x24
OLDESP     = 0x28
OLDSS      = 0x2c

int_msg:
    .asciz  "In kernel interrupt\n\r"

system_call:
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ds
    pushl %es
    pushl %fs
    movl  $0x10, %eax
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %fs
    /* call  _printk */
    pushl $int_msg
    call  printk
    popl  %eax
    popl  %fs
    popl  %es
    popl  %ds
    popl  %edx
    popl  %ecx
    popl  %eax
    iret

ret_from_sys_call:
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %fs
    popl %es
    popl %ds
    iret

.align 4
timer_interrupt:
    pushl %ds
    pushl %es
    pushl %fs
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    movl  $0x10, %eax
    movw  %ax, %ds
    movw  %ax, %es
    movl  $0x17, %eax
    movw  %ax, %fs
    movb  $0x20, %al
    outb  %al, $0x20
    movl  CS(%esp), %eax
    andl  $3, %eax
    pushl %eax
    call  do_timer
    addl  $4, %esp
    jmp   ret_from_sys_call

