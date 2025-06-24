#include <linux/tty.h>

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

void con_init() {
    volatile char *marker = (char *)0x7000;
    *marker = 'A';
    *(marker + 1) = 'B';
    *(marker + 2) = 'C';
    char * display_desc = "????";
    char * display_ptr;

    video_num_columns = ORIG_VIDEO_COLS;
    video_size_row = video_num_columns * 2;
    video_num_lines = ORIG_VIDEO_LINES;
    video_page = ORIG_VIDEO_PAGE;

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
        *display_ptr++;
    }
}

