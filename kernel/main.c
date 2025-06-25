#define __LIBRARY__

#include <linux/tty.h>
#include <linux/kernel.h>

void main(void) {
    tty_init();
    printk("hello world %d", 1234);
    __asm__("int $0x80 \n\r"::);
    __asm__ __volatile__(
            "loop:\n\r"
            "jmp loop"
            ::);
}