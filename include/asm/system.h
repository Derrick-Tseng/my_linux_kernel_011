#ifndef _SYSTEM_H
#define _SYSTEM_H

#define move_to_user_mode() \
__asm__("movl %%esp, %%eax\n\t" \
        "pushl $0x17\n\t"       \
        "pushl %%eax\n\t"       \
        "pushfl\n\t"            \
        "pushl $0x0f\n\t"       \
        "pushl $1f\n\t"         \
        "iret\n\t"              \
        "1:\tmovl $0x17, %%eax\n\t" \
        "movw %%ax, %%ds\n\t"   \
        "movw %%ax, %%es\n\t"   \
        "movw %%ax, %%fs\n\t"   \
        "movw %%ax, %%gs\n\t"   \
        :::"ax")

// Sets the interrupt flag (IF) in the processor's flags register
// Allowing the CPU to respond to external interrupt requests from devices like keyboards, timers, or network cards.
#define sti() __asm__("sti"::)

// Clears the interrupt flag (IF) in the processor's flags register
#define cli() __asm__("cli"::)

// It performs no operation other than consuming one CPU cycle and advancing the instruction pointer
#define nop() __asm__("nop"::)

// This macro is used to return from an interrupt handler
#define iret() __asm__("iret"::)

#define _set_gate(gate_addr, type, dpl, addr) \
__asm__("movw %%dx, %%ax\n\t"   \
        "movw %0, %%dx\n\t"     \
        "movl %%eax, %1\n\t"    \
        "movl %%edx, %2"        \
        :                       \
        :"i"((short)(0x8000 + (dpl << 13) + (type << 8))),  \
        "o"(*((char*)(gate_addr))),                         \
        "o"(*(4 + (char*)(gate_addr))),                     \
        "d"((char*)(addr)), "a" (0x00080000))


#define set_intr_gate(n, addr) \
    _set_gate(&idt[n], 14, 0, addr)

#define set_trap_gate(n, addr) \
    _set_gate(&idt[n], 15, 0, addr)

#define set_system_gate(n, addr) \
    _set_gate(&idt[n], 15, 3, addr)

#define _set_tssldt_desc(n, addr, type) \
__asm__("movw $104, %1\n\t"             \
        "movw %%ax, %2\n\t"             \
        "rorl $16, %%eax\n\t"           \
        "movb %%al, %3\n\t"             \
        "movb $" type ", %4\n\t"        \
        "movb $0x00, %5\n\t"            \
        "movb %%ah, %6\n\t"             \
        "rorl $16, %%eax\n\t"           \
        ::"a"(addr), "m"(*(n)), "m"(*(n+2)), "m"(*(n+4)), \
        "m"(*(n+5)), "m"(*(n+6)), "m"(*(n+7)) \
       )

#define set_tss_desc(n, addr) _set_tssldt_desc(((char*)(n)), addr, "0x89")
#define set_ldt_desc(n, addr) _set_tssldt_desc(((char*)(n)), addr, "0x82")

#endif