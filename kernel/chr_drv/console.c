#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>


#define ORIG_X          (*(unsigned char *)0x90000)
#define ORIG_Y          (*(unsigned char *)0x90001)
#define ORIG_VIDEO_PAGE     (*(unsigned short *)0x90004)
#define ORIG_VIDEO_MODE     ((*(unsigned short *)0x90006) & 0xff)
#define ORIG_VIDEO_COLS     (((*(unsigned short *)0x90006) & 0xff00) >> 8)
#define ORIG_VIDEO_LINES    ((*(unsigned short *)0x9000e) & 0xff)
#define ORIG_VIDEO_EGA_AX   (*(unsigned short *)0x90008)
#define ORIG_VIDEO_EGA_BX   (*(unsigned short *)0x9000a)
#define ORIG_VIDEO_EGA_CX   (*(unsigned short *)0x9000c)

#define VIDEO_TYPE_MDA      0x10    /* Monochrome Text Display  */
#define VIDEO_TYPE_CGA      0x11    /* CGA Display          */
#define VIDEO_TYPE_EGAM     0x20    /* EGA/VGA in Monochrome Mode   */
#define VIDEO_TYPE_EGAC     0x21    /* EGA/VGA in Color Mode    */

static unsigned char    video_type;     /* Type of display being used   */
static unsigned long    video_num_columns;  /* Number of text columns   */
static unsigned long    video_num_lines;    /* Number of test lines     */
static unsigned long    video_mem_base;     /* Base of video memory     */
static unsigned long    video_mem_end;     /* End of video memory      */
static unsigned long    video_size_row;     /* Bytes per row        */
static unsigned char    video_page;     /* Initial video page       */
static unsigned short   video_port_reg;     /* Video register select port   */
static unsigned short   video_port_val;     /* Video register value port    */
static unsigned short   video_erase_char; /* Character used to erase the screen */

static unsigned long origin;
static unsigned long scr_end;
static unsigned long pos;
static unsigned long x;
static unsigned long y;
static unsigned long top;
static unsigned long bottom;
static unsigned long attr = 0x07; // Default attribute for text mode (light gray on black)

static inline void gotoxy(int new_x, unsigned int new_y) {
    if(new_x > video_num_columns || new_y >= video_num_lines){
        return; // Out of bounds
    }

    x = new_x;
    y = new_y;
    // two bytes per character (character + attribute)
    pos = origin + y * video_size_row + (x << 1);
}

static inline void set_cursor(){
    cli();
    // Select high byte of cursor position
    outb_p(14, video_port_reg);
    // Set high byte of cursor position
    outb_p(0xff & ((pos - video_mem_base) >> 9), video_port_val);
    // Select low byte of cursor position
    outb_p(15, video_port_reg);
    // Set low byte of cursor position
    outb_p(0xff & ((pos - video_mem_base) >> 1), video_port_val);
    sti();
}

static inline void set_origin(){
    cli();
    // Select high byte of cursor position
    outb_p(12, video_port_reg);
    // Set high byte of cursor position
    outb_p(0xff & ((origin - video_mem_base) >> 9), video_port_val);
    // Select low byte of cursor position
    outb_p(13, video_port_reg);
    // Set low byte of cursor position
    outb_p(0xff & ((origin - video_mem_base) >> 1), video_port_val);
    sti();
}

static void scroll_up(){
    // Is current scrolling region contains the entire visible screen.
    if(!top && bottom == video_num_lines) {
        // position of the upper left corner of the screen
        origin += video_size_row;
        // position of the cursor
        pos += video_size_row;
        // end of the screen
        scr_end += video_size_row;

        // If the end of the screen exceeds the video memory end
        // then we need to scroll the screen back to where the video memory starts.
        if (scr_end > video_mem_end) {
            // copy the content of the screen to the beginning of the video memory
            __asm__("cld\n\t"
                    "rep\n\t"
                    "movsl\n\t"
                    "movl video_num_columns,%1\n\t"
                    "rep\n\t"
                    "stosw"
                    ::"a" (video_erase_char),
                    "c" ((video_num_lines-1)*video_num_columns>>1),
                    "D" (video_mem_base),
                    "S" (origin):);
            scr_end -= (origin - video_mem_base);
            pos -= (origin - video_mem_base);
            origin = video_mem_base;
        }
        else {
             __asm__("cld\n\t"
                     "rep\n\t"
                     "stosw"
                     ::"a" (video_erase_char),
                     "c" (video_num_columns),
                     "D" (scr_end-video_size_row):);
        }
        set_origin();
    }
    else{
        __asm__("cld\n\t"
                "rep\n\t"
                "movsl\n\t"
                "movl video_num_columns,%%ecx\n\t"
                "rep\n\t"
                "stosw"
                ::"a" (video_erase_char),
                "c" ((bottom-top-1)*video_num_columns>>1),
                "D" (origin+video_size_row*top),
                "S" (origin+video_size_row*(top+1)):);
    }
}

void lf(){
    if(y + 1 < bottom){
        y++;
        pos += video_size_row;
        return;
    }
    scroll_up();
}

static void cr(){
    pos -= (x << 1);
    x = 0;
}

static void del(){
    if(x){
        pos -= 2;
        x--;
        *(unsigned short *)pos = video_erase_char;
    }
}

void console_print(const char* buf, int nr){
    char* str = buf;
    
    while(nr--) {
        char c = *str++;
        if (c > 31 && c < 127) {
            if (x >= video_num_columns) {
                x -= video_num_columns;
                pos -= video_size_row;
                lf();
            }

            *(char *)pos = c;
            *(((char*)pos) + 1) = attr;
            pos += 2;
            x++;
        }
        else if (c == 10 || c == 11 || c == 12)
            lf();
        else if (c == 13)
            cr();
        else if (c == 127) {
            del();
        }
        else if (c == 8) {
            if (x) {
                x--;
                pos -= 2;
            }
        }
    }

    gotoxy(x, y);
    set_cursor();
}


void con_init() {
    char * display_desc = "????";
    char * display_ptr;

    video_num_columns = ORIG_VIDEO_COLS;
    video_size_row = video_num_columns * 2;
    video_num_lines = ORIG_VIDEO_LINES;
    video_page = ORIG_VIDEO_PAGE;
    video_erase_char = 0x0720; 

    /* Is this a single color display? */
    if (ORIG_VIDEO_MODE == 7) {
        video_mem_base = 0xb0000;
        video_port_reg = 0x3b4;
        video_port_val = 0x3b5;
        if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10) {
            video_type = VIDEO_TYPE_EGAM;
            video_mem_end = 0xb8000;
            display_desc = "EGAm";
        }
        else {
            video_type = VIDEO_TYPE_MDA;
            video_mem_end = 0xb2000;
            display_desc = "*MDA";
        }
    }
    else { /* color display */
        video_mem_base = 0xb8000;
        video_port_reg  = 0x3d4;
        video_port_val  = 0x3d5;

        if ((ORIG_VIDEO_EGA_BX & 0xff) != 0x10) {
            video_type = VIDEO_TYPE_EGAC;
            video_mem_end = 0xc0000;
            display_desc = "EGAc";
        }
        else {
            video_type = VIDEO_TYPE_CGA;
            video_mem_end = 0xba000;
            display_desc = "*CGA";
        }
    }

    // writing the display type description on the screen
    
    // calculates where the description will be written
    // The subtraction of 8 bytes leaves room for approximately 4 characters
    // Each character in text mode video memory requires 2 bytes
    // (one for the character code and one for display attributes like color)
    display_ptr = ((char *)video_mem_base) + video_size_row*2 - 8;
    while (*display_desc) {
        *display_ptr++ = *display_desc++;
        display_ptr++;
    }

    origin = video_mem_base;
    scr_end = video_mem_base + video_num_lines * video_size_row;
    top = 0;
    bottom = video_num_lines;

    gotoxy(ORIG_X, ORIG_Y);
    set_cursor();
}

void con_write(const char* buf, int nr) {
    char* t = (char*)pos;
    const char* s = buf;
    int i = 0;

    for (i = 0; i < nr; i++) {
        *t++ = *(s++);
        *t++ = 0xf;
        x++;
    }

    pos = (long)t;
    gotoxy(x, y);
    set_cursor();
}

