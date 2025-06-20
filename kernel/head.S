.code32
.text
.globl startup_32
pg_dir:
startup_32:
    movl $0x10, %eax
    movw %ax,   %ds
    movw %ax,   %es
    movw %ax,   %fs
    movw %ax,   %gs

    call setup_idt
    call setup_gdt

    movl $0x18, %eax
    movw %ax, %gs
    movl $0x0, %edi
    movb $0xf, %ah
    movb $0x42, %al
    movw %ax, %gs:(%edi)

loop:
    jmp loop

setup_idt:
    leal ignore_int, %edx
    movl $0x00080000, %eax
    movw %dx, %ax
    movw $0x8e00, %dx
    leal idt, %edi
    movl $256, %ecx
rp_sidt:
    movl %eax, (%edi)
    movl %edx, 4(%edi)
    addl $8, %edi
    decl %ecx
    jne  rp_sidt
    lidt idt_descr
    ret

setup_gdt:
    lgdt gdt_descr
    ret

ignore_int:
    /* we do not have function _printk now, so trick it */
    pushl %eax
    pushl %ecx
    pushl %edx
    pushw %ds
    pushw %es
    pushw %fs
    movl  $0x10, %eax
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %fs
    /* call  _printk */
    movl  $0x96, %edi
    movb  $'I', %al
    movb  $0x0c, %ah
    movw  %ax,   %gs:(%edi)
    popw  %fs
    popw  %es
    popw  %ds
    popl  %edx
    popl  %ecx
    popl  %eax
    iret
    
.align 4
.word 0
idt_descr:
    .word 256*8-1
    .long idt

.align 4
.word 0
gdt_descr:
    .word 256 * 8 - 1
    .long gdt

.align 8
idt:
    .fill 256, 8, 0

gdt:
    .quad 0x0000000000000000
    .quad 0x00c09a0000000fff
    .quad 0x00c0920000000fff
    .quad 0x00c0f20b8000ffff
    .quad 0x0000000000000000
    .fill 251, 8,0
