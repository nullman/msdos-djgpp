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
#define VIDEO_MEMORY 0xA0000;           // start of video memory
#define SYSTEM_CLOCK 0x046C;            // system clock memory location
#define SCREEN_WIDTH 320                // width in pixels of VGA mode 0x13
#define SCREEN_HEIGHT 200               // height in pixels of VGA mode 0x13
#define NUM_COLORS 256                  // number of colors in VGA mode
#define CLOCK_HZ 18.2                   // system clock HZ

// use all colors except black (0)
#define RANDOM_COLOR() (rand() % (NUM_COLORS - 1) + 1)

typedef unsigned char byte;
typedef unsigned short ushort;

byte* vga = (byte*)VIDEO_MEMORY;
ushort* clk = (ushort*)SYSTEM_CLOCK;

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

void draw_pixel(int x, int y, byte color) {
    int offset;

    offset = y * SCREEN_WIDTH + x;
    //offset = (y<<8) + (y<<6) + x;       // faster, but harder to understand
    vga[offset] = color;
}

/* void draw_line_long_x(int x1, int y1, int x2, int y2, byte color) { */
/*     int x, y, dx, dy, yi, d; */

/*     // reverse line if x1 > x2 */
/*     if (x1 > x2) { */
/*         x = x1; */
/*         x1 = x2; */
/*         x2 = x; */
/*         y = y1; */
/*         y1 = y2; */
/*         y2 = y; */
/*     } */

/*     dx = x2 - x1; */
/*     dy = y2 - y1; */
/*     yi = 1; */
/*     if (dy < 0) { */
/*         yi = -1; */
/*         dy = -dy; */
/*     } */
/*     d = 2 * dy - dx; */
/*     y = y1; */

/*     for (x = x1; x <= x2; x++) { */
/*         draw_pixel(x, y, color); */
/*         if (d > 0) { */
/*             y = y + yi; */
/*             d = d + 2 * (dy - dx); */
/*         } else { */
/*             d = d + 2 * dy; */
/*         } */
/*     } */
/* } */

/* void draw_line_long_y(int x1, int y1, int x2, int y2, byte color) { */
/*     int x, y, dx, dy, xi, d; */

/*     // reverse line if y1 > y2 */
/*     if (y1 > y2) { */
/*         x = x1; */
/*         x1 = x2; */
/*         x2 = x; */
/*         y = y1; */
/*         y1 = y2; */
/*         y2 = y; */
/*     } */

/*     dx = x2 - x1; */
/*     dy = y2 - y1; */
/*     xi = 1; */
/*     if (dx < 0) { */
/*         xi = -1; */
/*         dx = -dx; */
/*     } */
/*     d = 2 * dx - dy; */
/*     x = x1; */

/*     for (y = y1; y <= y2; y++) { */
/*         draw_pixel(x, y, color); */
/*         if (d > 0) { */
/*             x = x + xi; */
/*             d = d + 2 * (dx - dy); */
/*         } else { */
/*             d = d + 2 * dx; */
/*         } */
/*     } */
/* } */

/* void draw_line(int x1, int y1, int x2, int y2, byte color) { */
/*     // it looks better if we single step over the longer distance */
/*     if (abs(x2 - x1) > abs(y2 - y1)) { */
/*         draw_line_long_x(x1, y1, x2, y2, color); */
/*     } else { */
/*         draw_line_long_y(x1, y1, x2, y2, color); */
/*     } */
/* } */

void draw_line(int x1, int y1, int x2, int y2, byte color) {
    int x, y, dx, dy, sx, sy, e1, e2;

    dx = abs(x2 - x1);
    sx = (x1 < x2) ? 1 : -1;
    dy = abs(y2 - y1);
    sy = (y1 < y2) ? 1 : -1;
    e1 = dx + dy;

    x = x1;
    y = y1;

    while (1) {
        draw_pixel(x, y, color);
        if (x == x2 && y == y2) break;
        e2 = 2 * e1;
        if (e2 >= dy) {
            if (x == x2) break;
            e1 = e1 + dy;
            x = x + sx;
        }
        if (e2 <= dx) {
            if (y == y2) break;
            e1 = e1 + dx;
            y = y + sy;
        }
    }
}

double degrees_to_radians(int degrees) {
    return degrees * M_PI / 180.0;
}

// draw lines until a key is pressed
void draw_lines() {
    int x1, y1, x2, y2, deg, t;
    byte color;

    x1 = 0;
    y1 = 0;
    x2 = SCREEN_WIDTH - 1;
    y2 = 0;
    color = 1;

    // loop until key-press
    while (!kbhit()) {
        for (deg = 0; deg <= 45; deg += 3) {
            // draw line
            draw_line(x1, y1, x2, y2, color);
            //draw_line(10, 10, 310, 190, color);
            //draw_line(10, 10, 20, 20, color);

            // add degrees until 90
            y2 = (int)((SCREEN_HEIGHT - 1) * sin(degrees_to_radians(deg)));

            // pause to slow down draw speed
            for (t = 0; t < 1000000; t++) {}
        }
    }

    // consume key-press
    getch();
}

int main(void) {
    if (__djgpp_nearptr_enable() == 0) {
        printf("Could not get access to first 640K of memory\n");
        exit(EXIT_FAILURE);
    }

    vga += __djgpp_conventional_base;
    clk = (void*)clk + __djgpp_conventional_base;

    // seed number generator
    srand(*clk);

    // set vga mode and clear screen
    set_mode(VGA_256_COLOR_MODE);

    // main loop
    draw_lines();

    // set text mode and clear screen
    set_mode(TEXT_MODE);
    clrscr();

    __djgpp_nearptr_disable();

    return EXIT_SUCCESS;
}
