#ifndef _TTY_H
#define _TTY_H

void con_init();
void tty_init();

void con_write(const char* buf, int nr);

#endif