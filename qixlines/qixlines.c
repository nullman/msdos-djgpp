/**
 * QIX Lines
 *
 * Draw QIX lines with alternating colors.
 */

#include <conio.h>
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define COLOR_BG 0                      // default background color
#define COLOR_FG 1                      // default foreground color
#define MAX_SIN 180                     // maximum allowed value for sin math
#define HISTORY_SIZE 10                 // how many lines to display at once
#define STEP 8                          // line spacing
#define STEP_RANGE 6                    // spacing plus/minus range

typedef unsigned char byte;
typedef unsigned short ushort;

typedef struct {
    short x1;
    short y1;
    short x2;
    short y2;
    byte color;
} line_s;

typedef struct {
    byte help;
    byte vga_mode;
} args_s;

byte *vga = (byte *)VIDEO_MEMORY;
byte vga_mode;
ushort screen_width, screen_height, num_colors;
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

byte random_color() {
    if (vga_mode == vga_mode == VGA_256_COLOR_MODE) {
        return rand() % num_colors;
    } else {
        // use all colors except black (0)
        return rand() % (num_colors - 1) + 1;
    }
}

void linecpy(line_s *target_line, line_s *source_line) {
    target_line->x1 = source_line->x1;
    target_line->y1 = source_line->y1;
    target_line->x2 = source_line->x2;
    target_line->y2 = source_line->y2;
    target_line->color = source_line->color;
}

void draw_pixel(ushort x, ushort y, byte color) {
    ushort offset;

    offset = y * screen_width + x;
    //offset = (y<<8) + (y<<6) + x;       // faster, but harder to understand
    vga[offset] = color;
}

void draw_line(line_s *line) {
    ushort x1, y1, x2, y2, x, y;
    byte color;
    int dx, dy, sx, sy, e1, e2;

    x1 = line->x1;
    y1 = line->y1;
    x2 = line->x2;
    y2 = line->y2;
    color = line->color;

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
        if (x < screen_width && y < screen_height) {
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

ushort next_degree(ushort degree) {
    // add randomly to the degree
    ushort d = degree + STEP + rand() % (STEP_RANGE * 2 + 1) - STEP_RANGE;
    if (d >= MAX_SIN) d = d - MAX_SIN;
    return d;
}

double degrees_to_radians(ushort degree) {
    return degree * M_PI / 180.0;
}

void next_line(line_s *line, line_s *line_delta, line_s *line_degree) {
    // randomly add to the degrees
    line_degree->x1 = next_degree(line_degree->x1);
    line_degree->y1 = next_degree(line_degree->y1);
    line_degree->x2 = next_degree(line_degree->x2);
    line_degree->y2 = next_degree(line_degree->y2);

    // add using sin modified by a delta for each coordinate dimension
    line->x1 += (ushort)(line_delta->x1 * sin(degrees_to_radians(line_degree->x1)));
    line->y1 += (ushort)(line_delta->y1 * sin(degrees_to_radians(line_degree->y1)));
    line->x2 += (ushort)(line_delta->x2 * sin(degrees_to_radians(line_degree->x2)));
    line->y2 += (ushort)(line_delta->y2 * sin(degrees_to_radians(line_degree->y2)));

    // if any coordinates are out of range, reverse their direction and change color
    if (line->x1 < 0) {
        line->x1 = 0 - line->x1;
        line_delta->x1 = -line_delta->x1;
        line->color = random_color();
    }
    if (line->x1 >= screen_width) {
        line->x1 = screen_width - (line->x1 - screen_width);
        line_delta->x1 = -line_delta->x1;
        line->color = random_color();
    }
    if (line->y1 < 0) {
        line->y1 = 0 - line->y1;
        line_delta->y1 = -line_delta->y1;
        line->color = random_color();
    }
    if (line->y1 >= screen_height) {
        line->y1 = screen_height - (line->y1 - screen_height);
        line_delta->y1 = -line_delta->y1;
        line->color = random_color();
    }
    if (line->x2 < 0) {
        line->x2 = 0 - line->x2;
        line_delta->x2 = -line_delta->x2;
        line->color = random_color();
    }
    if (line->x2 >= screen_width) {
        line->x2 = screen_width - (line->x2 - screen_width);
        line_delta->x2 = -line_delta->x2;
        line->color = random_color();
    }
    if (line->y2 < 0) {
        line->y2 = 0 - line->y2;
        line_delta->y2 = -line_delta->y2;
        line->color = random_color();
    }
    if (line->y2 >= screen_height) {
        line->y2 = screen_height - (line->y2 - screen_height);
        line_delta->y2 = -line_delta->y2;
        line->color = random_color();
    }
}

// draw lines until a key is pressed
void draw_lines() {
    line_s line, line_delta, line_degree, line_history[HISTORY_SIZE];
    ushort i, history_index;

    // randomize starting values
    line.x1 = rand() % screen_width;
    line.y1 = rand() % screen_height;
    line.x2 = rand() % screen_width;
    line.y2 = rand() % screen_height;
    line.color = COLOR_BG;

    line_delta.x1 = STEP;
    line_delta.y1 = STEP;
    line_delta.x2 = STEP;
    line_delta.y2 = STEP;

    line_degree.x1 = rand() % MAX_SIN;
    line_degree.y1 = rand() % MAX_SIN;
    line_degree.x2 = rand() % MAX_SIN;
    line_degree.y2 = rand() % MAX_SIN;

    // initialize history
    for (i = 0; i < HISTORY_SIZE; i++) {
        linecpy(&line_history[i], &line);
    }
    history_index = 0;

    // loop until key-press
    while (!kbhit()) {
        // get next line
        next_line(&line, &line_delta, &line_degree);

        // draw line
        draw_line(&line);

        // undraw oldest line
        line_history[history_index].color = COLOR_BG;
        draw_line(&line_history[history_index]);

        // add to history
        linecpy(&line_history[history_index++], &line);
        if (history_index >= HISTORY_SIZE) history_index = 0;

        // pause to slow down draw speed
        sleep(100);
    }

    // consume key-press
    getch();
}

void parse_args(int argc, char *argv[], args_s *args) {
    int i;

    // set defaults
    args->help = 0;
    args->vga_mode = VGA_256_COLOR_MODE;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "lo") == 0) {
            args->vga_mode = VGA_256_COLOR_MODE;
        } else if (strcmp(argv[i], "hi") == 0) {
            args->vga_mode = VGA_16_COLOR_MODE;
        } else {
            args->help = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    args_s args;

    if (__djgpp_nearptr_enable() == 0) {
        printf("Could not get access to first 640K of memory\n");
        exit(EXIT_FAILURE);
    }

    vga += __djgpp_conventional_base;
    clk = (void *)clk + __djgpp_conventional_base;

    // seed number generator
    srand(*clk);

    // defaults
    args.help = 0;
    args.vga_mode = VGA_256_COLOR_MODE;

    // parse parameters
    parse_args(argc, argv, &args);

    // display help
    if (args.help) {
        printf("Usage: %s [lo|hi]\n", argv[0]);
        printf("Where:\n");
        printf("  lo - VGA 256 color mode (320x200)\n");
        printf("  hi - VGA 16 color mode (640x480)\n");
        return EXIT_FAILURE;
    }

    // set modes
    vga_mode = args.vga_mode;
    if (vga_mode == VGA_256_COLOR_MODE) {
        screen_width = VGA_256_COLOR_SCREEN_WIDTH;
        screen_height = VGA_256_COLOR_SCREEN_HEIGHT;
        num_colors = VGA_256_COLOR_NUM_COLORS;
    } else {
        screen_width = VGA_16_COLOR_SCREEN_WIDTH;
        screen_height = VGA_16_COLOR_SCREEN_HEIGHT;
        num_colors = VGA_16_COLOR_NUM_COLORS;
    }

    // set vga mode and clear screen
    set_mode(vga_mode);

    // main loop
    draw_lines();

    // set text mode and clear screen
    set_mode(TEXT_MODE);
    clrscr();

    __djgpp_nearptr_disable();

    return EXIT_SUCCESS;
}
