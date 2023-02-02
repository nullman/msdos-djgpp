/**
 * Lines
 *
 * Draw lines using Bresenham's algorithm:
 *
 * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */

#include <conio.h>
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/nearptr.h>

#define VIDEO_INT 0x10                  // BIOS video interrupt
#define SET_MODE 0x00                   // BIOS function to set video mode
#define VGA_256_COLOR_MODE 0x13         // use to set 256 VGA color mode
#define TEXT_MODE 0x03                  // use to set text mode
#define PIXEL_PLOT 0x0C                 // BIOS function to plot a pixel
#define VIDEO_MEMORY 0xA0000            // start of video memory
#define SYSTEM_CLOCK 0x046C             // system clock memory location
#define SCREEN_WIDTH 320                // width in pixels of VGA mode 0x13
#define SCREEN_HEIGHT 200               // height in pixels of VGA mode 0x13
#define NUM_COLORS 256                  // number of colors in VGA mode
#define CLOCK_HZ 18.2                   // system clock HZ

// use all colors except black (0)
#define RANDOM_COLOR() (rand() % (NUM_COLORS - 1) + 1)

typedef unsigned char byte;
typedef unsigned short ushort;

byte *vga = (byte *)VIDEO_MEMORY;
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

    ticks = msec * CLOCK_HZ / 1000.0;
    ts = *clk;

    while (*clk - ts < ticks) {
        *clk = *clk;                    // force compiler to properly loop
    }
}

void draw_pixel(ushort x, ushort y, byte color) {
    ushort offset;

    offset = y * SCREEN_WIDTH + x;
    //offset = (y<<8) + (y<<6) + x;       // faster, but harder to understand
    vga[offset] = color;
}

void draw_line(ushort x1, ushort y1, ushort x2, ushort y2, byte color) {
    ushort x, y;
    int dx, dy, sx, sy, e1, e2;

    dx = x2 - x1;
    if (dx < 0) dx = -dx;
    sx = (x1 < x2) ? 1 : -1;
    dy = y2 - y1;
    if (dy > 0) dy = -dy;
    sy = (y1 < y2) ? 1 : -1;
    e1 = dx + dy;

    x = x1;
    y = y1;

    while (1) {
        if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
            draw_pixel(x, y, color);
        }
        if (x == x2 && y == y2) break;
        e2 = 2 * e1;
        if (e2 >= dy) {
            if (x == x2) break;
            e1 += dy;
            x += sx;
        }
        if (e2 <= dx) {
            if (y == y2) break;
            e1 += dx;
            y += sy;
        }
    }
}

double degrees_to_radians(ushort degree) {
    return degree * M_PI / 180.0;
}

void draw_lines() {
    ushort x1, y1, x2, y2, deg;
    byte color;

    x1 = 0;
    y1 = 0;
    x2 = SCREEN_WIDTH - 1;
    y2 = 0;
    color = 1;

    for (deg = 0; deg <= 90; deg += 1) {
        // draw line
        draw_line(x1, y1, x2, y2, color);

        // add degrees until 90
        y2 = (ushort)((SCREEN_HEIGHT - 1) * sin(degrees_to_radians(deg)));
    }
    y2 = SCREEN_HEIGHT - 1;
    for (deg = 90; deg <= 180; deg += 1) {
        // draw line
        draw_line(x1, y1, x2, y2, color);

        // add degrees until 90
        x2 = (ushort)((SCREEN_WIDTH - 1) * sin(degrees_to_radians(deg)));
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

    // set vga mode and clear screen
    set_mode(VGA_256_COLOR_MODE);

    // main loop
    draw_lines();

    // wait for key-press
    getch();

    // set text mode and clear screen
    set_mode(TEXT_MODE);
    clrscr();

    __djgpp_nearptr_disable();

    return EXIT_SUCCESS;
}
