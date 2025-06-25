#include <linux/tty.h>

void tty_init() {
    con_init();
}

void tty_write(unsigned channel, char* buf, int nr) {
    con_write(buf, nr);
}