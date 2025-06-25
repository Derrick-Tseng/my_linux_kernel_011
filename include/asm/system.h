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

#endif