/**
 * Colors
 *
 * Display VGA colors.
 */

#include <conio.h>
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/nearptr.h>

#define VIDEO_INT 0x10                  // BIOS video interrupt
#define SET_MODE 0x00                   // BIOS function to set video mode
#define VGA_16_COLOR_MODE 0x12          // use to set 16 color VGA mode
#define VGA_256_COLOR_MODE 0x13         // use to set 256 color VGA mode
#define TEXT_MODE 0x03                  // use to set text mode
#define PIXEL_PLOT 0x0C                 // BIOS function to plot a pixel
#define VIDEO_MEMORY 0xA0000            // start of video memory
#define SYSTEM_CLOCK 0x046C             // system clock memory location
#define VGA_16_COLOR_SCREEN_WIDTH 640   // width in pixels of VGA mode 0x12
#define VGA_16_COLOR_SCREEN_HEIGHT 480  // height in pixels of VGA mode 0x12
#define VGA_16_COLOR_NUM_COLORS 16      // number of colors in VGA mode 0x12
#define VGA_256_COLOR_SCREEN_WIDTH 320  // width in pixels of VGA mode 0x13
#define VGA_256_COLOR_SCREEN_HEIGHT 200 // height in pixels of VGA mode 0x13
#define VGA_256_COLOR_NUM_COLORS 256    // number of colors in VGA mode 0x13
#define CLOCK_HZ 18.2                   // system clock HZ

typedef unsigned char byte;
typedef unsigned short ushort;

byte *vga = (byte *)VIDEO_MEMORY;
byte vga_mode;
ushort screen_width;
ushort *clk = (ushort *)SYSTEM_CLOCK;

void set_mode(byte mode) {
    union REGS regs;

    regs.h.ah = SET_MODE;
    regs.h.al = mode;
    int86(VIDEO_INT, &regs, &regs);
}

void sleep(int msec) {
    ushort ticks;
    ushort ts;

    ticks = msec * CLOCK_HZ / 1000;
    ts = *clk;

    while (*clk - ts < ticks) {
        *clk = *clk;                    // force compiler to properly loop
    }
}

void draw_pixel(ushort x, ushort y, byte color) {
    ushort offset;

    offset = y * screen_width + x;
    //offset = (y<<8) + (y<<6) + x;       // faster, but harder to understand
    vga[offset] = color;
}

void draw_box(ushort x1, ushort y1, ushort x2, ushort y2, byte color) {
    ushort x, y;

    if (y1 > y2) {
        y = y1;
        y1 = y2;
        y2 = y;
    }

    if (x1 > x2) {
        x = x1;
        x1 = x2;
        x2 = x;
    }

    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            draw_pixel(x, y, color);
            //printf("x: %d, y: %d, color: %d\n", x, y, color);
            sleep(100);
        }
    }
}

void draw_16_colors() {
    ushort x1, y1, x2, y2;

    for (ushort color = 0; color < VGA_16_COLOR_NUM_COLORS; color++) {
        x1 = color % 2 * VGA_16_COLOR_SCREEN_WIDTH / 2;
        x2 = x1 + VGA_16_COLOR_SCREEN_WIDTH / 2;
        y1 = color / 2 * 20;
        y2 = y1 + 15;
        draw_box(x1, y1, x2, y2, color);
    }
}

int main(void) {
    if (__djgpp_nearptr_enable() == 0) {
        printf("Could not get access to first 640K of memory\n");
        exit(EXIT_FAILURE);
    }

    vga += __djgpp_conventional_base;
    clk = (void *)clk + __djgpp_conventional_base;

    // seed number generator
    srand(*clk);

    // set vga mode (16), clear screen, and draw colors
    screen_width = VGA_16_COLOR_SCREEN_WIDTH;
    set_mode(VGA_16_COLOR_MODE);
    //draw_16_colors();
    draw_box(0, 0, 100, 100, 1);

    // wait for key-press
    getch();

    // set text mode and clear screen
    set_mode(TEXT_MODE);
    clrscr();

    __djgpp_nearptr_disable();

    return EXIT_SUCCESS;
}
