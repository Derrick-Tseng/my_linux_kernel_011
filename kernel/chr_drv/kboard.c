#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/tty.h>

typedef void (*key_fn)();
void do_self();
void ctrl();
void func(){};

void cursor(){};

void lshift();
void rshift();
void unlshift();
void unrshift();

void caps();
void uncaps();

void alt() {}
void unalt() {}
void unctrl();
void minus(){};
void num();
void scroll();

void kb_wait();

static key_fn key_table[] = {
    0,do_self,do_self,do_self, /* 00-03 s0 esc 1 2 */
    do_self,do_self,do_self,do_self,  /* 04-07 3 4 5 6 */
    do_self,do_self,do_self,do_self,  /* 08-0B 7 8 9 0 */
    do_self,do_self,do_self,do_self,  /* 0C-0F + ' bs tab */
    do_self,do_self,do_self,do_self,  /* 10-13 q w e r */
    do_self,do_self,do_self,do_self,  /* 14-17 t y u i */
    do_self,do_self,do_self,do_self,  /* 18-1B o p } ^ */
    do_self,ctrl,do_self,do_self, /* 1C-1F enter ctrl a s */
    do_self,do_self,do_self,do_self,  /* 20-23 d f g h */
    do_self,do_self,do_self,do_self,  /* 24-27 j k l | */
    do_self,do_self,lshift,do_self,   /* 28-2B { para lshift , */
    do_self,do_self,do_self,do_self,  /* 2C-2F z x c v */
    do_self,do_self,do_self,do_self,  /* 30-33 b n m , */
    do_self,minus,rshift,do_self, /* 34-37 . - rshift * */
    alt,do_self,caps,func,    /* 38-3B alt sp caps f1 */
    func,func,func,func,      /* 3C-3F f2 f3 f4 f5 */
    func,func,func,func,      /* 40-43 f6 f7 f8 f9 */
    func,num,scroll,cursor,       /* 44-47 f10 num scr home */
    cursor,cursor,do_self,cursor, /* 48-4B up pgup - left */
    cursor,cursor,do_self,cursor, /* 4C-4F n5 right + end */
    cursor,cursor,cursor,cursor,  /* 50-53 dn pgdn ins del */
    0,0,do_self,func,       /* 54-57 sysreq ? < f11 */
    func,0,0,0,      /* 58-5B f12 ? ? ? */
    0,0,0,0,      /* 5C-5F ? ? ? ? */
    0,0,0,0,      /* 60-63 ? ? ? ? */
    0,0,0,0,      /* 64-67 ? ? ? ? */
    0,0,0,0,      /* 68-6B ? ? ? ? */
    0,0,0,0,      /* 6C-6F ? ? ? ? */
    0,0,0,0,      /* 70-73 ? ? ? ? */
    0,0,0,0,      /* 74-77 ? ? ? ? */
    0,0,0,0,      /* 78-7B ? ? ? ? */
    0,0,0,0,      /* 7C-7F ? ? ? ? */
    0,0,0,0,      /* 80-83 ? br br br */
    0,0,0,0,      /* 84-87 br br br br */
    0,0,0,0,      /* 88-8B br br br br */
    0,0,0,0,      /* 8C-8F br br br br */
    0,0,0,0,      /* 90-93 br br br br */
    0,0,0,0,      /* 94-97 br br br br */
    0,0,0,0,      /* 98-9B br br br br */
    0,unctrl,0,0,    /* 9C-9F br unctrl br br */
    0,0,0,0,      /* A0-A3 br br br br */
    0,0,0,0,      /* A4-A7 br br br br */
    0,0,unlshift,0,      /* A8-AB br br unlshift br */
    0,0,0,0,      /* AC-AF br br br br */
    0,0,0,0,      /* B0-B3 br br br br */
    0,0,unrshift,0,      /* B4-B7 br br unrshift br */
    unalt,0,uncaps,0,       /* B8-BB unalt br uncaps br */
    0,0,0,0,      /* BC-BF br br br br */
    0,0,0,0,      /* C0-C3 br br br br */
    0,0,0,0,      /* C4-C7 br br br br */
    0,0,0,0,      /* C8-CB br br br br */
    0,0,0,0,      /* CC-CF br br br br */
    0,0,0,0,      /* D0-D3 br br br br */
    0,0,0,0,      /* D4-D7 br br br br */
    0,0,0,0,      /* D8-DB br ? ? ? */
    0,0,0,0,      /* DC-DF ? ? ? ? */
    0,0,0,0,      /* E0-E3 e0 e1 ? ? */
    0,0,0,0,      /* E4-E7 ? ? ? ? */
    0,0,0,0,      /* E8-EB ? ? ? ? */
    0,0,0,0,      /* EC-EF ? ? ? ? */
    0,0,0,0,      /* F0-F3 ? ? ? ? */
    0,0,0,0,      /* F4-F7 ? ? ? ? */
    0,0,0,0,      /* F8-FB ? ? ? ? */
    0,0,0,0,      /* FC-FF ? ? ? ? */
};

static char key_map[0x7f] = {
    0, 27,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    127, 9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    10, 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
    '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, '*', 0, 32,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+',
    0,0,0,0,0,0,0,
    '>',
    0,0,0,0,0,0,0,0,0,0
};

// When the shift key is pressed
static char shift_map[0x7f] = {
    0,27,
    '!', '@', '#', '$', '%', '^', '&', '*','(',')','_','+',
    127,9,
    'Q','W','E','R','T','Y','U','I','O','P','{','}',
    10,0,
    'A','S','D','F','G','H','J','K','L',':','\"',
    '~',0,
    '|','Z','X','C','V','B','N','M','<','>','?',
    0,'*',0,32,                                  //36h-39h
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,             //3Ah-49h
    '-',0,0,0,'+',                               //4A-4E
    0,0,0,0,0,0,0,                               //4F-55
    '>',
    0,0,0,0,0,0,0,0,0,0
};

unsigned char scan_code, leds, mode, e0;

void keyboard_handler(void){
    unsigned char scan_code;

    scan_code = inb_p(0x60);
    outb_p(0x20, 0x20);
    
    // E0 prefix indicates extended key (e.g., arrow keys, right ctrl/alt)
    if(scan_code == 0xE0){
        e0 = 1;
    }
    // E1 prefix indicates pause/break key sequence
    else if(scan_code == 0xE1){
        e0 = 2;
    }
    else{
       key_fn func = key_table[scan_code];
       if(func)
           func();
    }

    do_tty_interrupt();
}

void do_self() {
    char* rmap = key_map;
    char c = 0;
    if (mode & 0x3) // shift
        rmap = shift_map;

    c = rmap[scan_code];
    if (!c)
        return;

    // caps lock handling
    if (mode & 0x40) {
        // If caps lock is on and the character is a letter
        if (c >= 'a' && c <= '}')
            c -= 0x20;
        // If ctrl is pressed
        if (mode & 0xc && c >= 64 && c < 96)
            c -= 0x40;
    }

    PUTCH(c, &read_q);
}

/*
Mode
bit 0: left shift
bit 1: right shift
bit 2: left ctrl
bit 3: right ctrl
bit 4: left alt
bit 5: right alt
bit 6: caps lock
bit 7: if caps key is pressed
*/

// Left shift key pressed
void lshift(){
    mode |= 0x01;
}

// Left shift key released
void unlshift(){
    mode &= 0xfe;
}

// Right shift key pressed
void rshift(){
    mode |= 0x02;
}

// Right shift key released
void unrshift(){
    mode &= 0xfd;
}

// Ctrl key pressed
void ctrl(){
    if(e0){
        mode |= 0x8;
    }
    else{
        mode |= 0x04;
    }
}

// Ctrl key released
void unctrl(){
    if(e0){
        mode &= 0xf7;
    }
    else{
        mode &= 0xfb;
    }
}

// Waits for the keyboard controller to become ready for input
void kb_wait(){
    unsigned char a;
    while(1){
        a = inb_p(0x64);
        if(a == 0x2)
            return; // Keyboard is ready
    }
}

// Sets the LEDs on the keyboard
void set_leds(){
    kb_wait();
    outb_p(0xed, 0x60); // Command to set LEDs
    kb_wait();
    outb_p(leds, 0x60); // Send the LED state
}

// Cap lock key pressed
// It keeps triggering if the key is unreleased
void caps(){
    if(mode & 0x80)
        return; // Caps lock is already on

    leds ^= 0x04; // Toggle caps lock LED
    mode ^= 0x40; // Toggle caps lock mode
    mode |= 0x80; // Set the caps lock pressed flag
    set_leds();
}

// Caps lock key released
void uncaps(){
    mode &= 0x7f;
}

// Scroll lock key pressed/released
// It only trigger once even if the key is unreleased
void scroll() {
    leds ^= 0x1; // Toggle scroll lock LED (bit 0)
    set_leds();
}

// Num lock key pressed/released
// It only trigger once even if the key is unreleased
void num() {
    leds ^= 0x2; // Toggle num lock LED (bit 1)
    set_leds();
}