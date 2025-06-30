#ifndef _IO_H
#define _IO_H

// Outputs the byte value stored in the AL register to the I/O port specified by the DX register. 
#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

// Jumps forward two time to create a delay after the outb instruction
#define outb_p(value, port)     \
__asm__("outb %%al, %%dx\n"     \
        "\tjmp 1f\n"            \
        "1:\tjmp 1f\n"          \
        "1:" :: "a"(value), "d"(port))


#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
    "\tjmp 1f\n" \
    "1:\tjmp 1f\n" \
    "1:":"=a" (_v):"d" (port)); \
_v; \
})

#endif