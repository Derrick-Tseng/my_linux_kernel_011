#include <linux/tty.h>

void tty_init() {
    con_init();
}

void tty_write(unsigned channel, char* buf, int nr) {
    con_write(buf, nr);
}

// Calculate the number of characters in the tty queue
unsigned long CHARS(struct tty_queue *q){
    // Calculate the number of characters in the queue when the head is wrap around and less than the tail
    return (q->head - q->tail) & (TTY_BUF_SIZE - 1);
}

// Put a character into the tty queue
void PUTCH(char c, struct tty_queue *q){
    q->buf[q->head++] = c;
    q->head &= (TTY_BUF_SIZE - 1); // Wrap around if needed
}

// Get a character from the tty queue
char GETCH(struct tty_queue *q){
    char c = q->buf[q->tail++];
    q->tail &= (TTY_BUF_SIZE - 1); // Wrap around if needed
    return c;
}

// Check if the tty queue is empty
char EMPTY(struct tty_queue *q){
    return (q->head == q->tail);
}