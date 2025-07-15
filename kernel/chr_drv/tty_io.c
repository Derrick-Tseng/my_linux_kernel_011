#include <linux/tty.h>

struct tty_queue read_q = {0, 0, 0, 0, ""};
struct tty_queue write_q = {0, 0, 0, 0, ""};

void tty_init() {
    read_q = (struct tty_queue){0, 0, 0, 0, ""};
    write_q = (struct tty_queue){0, 0, 0, 0, ""};
    con_init();
}

void tty_write(unsigned channel, char* buf, int nr) {
    con_print(buf, nr);
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

void copy_to_cooked() {
    signed char c;

    while (!EMPTY(&read_q)) {
        c = GETCH(&read_q);
        
        // Newline character
        if (c == 10) {
            PUTCH(10, &write_q);
            PUTCH(13, &write_q);
        }
        // Backspace, Tab, and Enter
        else if (c == 8 || c == 9 || c == 13) {
            PUTCH(c, &write_q);
        }
        // Printable characters
        else if (c >= 32 && c < 127) {
            PUTCH(c, &write_q);
        }
        // Control characters (e.g., Ctrl + A)
        else if (c < 32) {
            PUTCH('^', &write_q);
            PUTCH(c + 64, &write_q);
        }
        else
            PUTCH(c, &write_q);

        con_write();
    }
}

void do_tty_interrupt() {
    copy_to_cooked();
}