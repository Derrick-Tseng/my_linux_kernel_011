#ifndef _SYSTEM_H
#define _SYSTEM_H

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

#endif