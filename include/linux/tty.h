#ifndef _TTY_H
#define _TTY_H

#define TTY_BUF_SIZE 1024

void con_init();
void tty_init();

void con_write(const char* buf, int nr);

struct tty_queue{
    unsigned long data;
    unsigned long head;
    unsigned long tail;
    struct task_struct *proc_list;
    char buf[TTY_BUF_SIZE];
};

unsigned long CHARS(struct tty_queue *q);
void PUTCH(char c, struct tty_queue *q);
char GETCH(struct tty_queue *q);
char EMPTY(struct tty_queue *q);

#endif