//
//  palette.c
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#include "color.h"

typedef struct {
    const char * name;
    int value;
} PaletteColor;

static const PaletteColor palette[] = {
    { .value = 0x000000, .name = "BLACK" },
    { .value = 0xFFFFFF, .name = "WHITE" },
    { .value = 0x404040, .name = "DARK_GRAY" },
    { .value = 0x808080, .name = "GRAY" },
    { .value = 0xC0C0C0, .name = "LIGHT_GRAY" },
    { .value = 0x000080, .name = "DARK_BLUE" },
    { .value = 0x0080FF, .name = "BLUE" },
    { .value = 0x80C0FF, .name = "LIGHT_BLUE" },
    { .value = 0x008000, .name = "DARK_GREEN" },
    { .value = 0x00FF80, .name = "GREEN" },
    { .value = 0xC0FFC0, .name = "LIGHT_GREEN" },
    { .value = 0x008080, .name = "DARK_CYAN" },
    { .value = 0x00C0C0, .name = "CYAN" },
    { .value = 0x80FFFF, .name = "LIGHT_CYAN" },
    { .value = 0x800000, .name = "DARK_RED" },
    { .value = 0xFF0080, .name = "RED" },
    { .value = 0xFF80C0, .name = "LIGHT_RED" },
    { .value = 0x800080, .name = "DARK_PURPLE" },
    { .value = 0x8000FF, .name = "PURPLE" },
    { .value = 0xFF80FF, .name = "PINK" },
    { .value = 0x804000, .name = "BROWN" },
    { .value = 0xFF8000, .name = "ORANGE" },
    { .value = 0xFFFF40, .name = "YELLOW" }
};

SDL_Color ColorToSDL(Color color)
{
    SDL_Color c;
    GetColorComponents(color, &c.r, &c.g, &c.b);
    return c;
}

void GetColorComponents(Color color, u8 * r, u8 * g, u8 * b)
{
    *r = (palette[color].value & 0xFF0000) >> 16;
    *g = (palette[color].value & 0x00FF00) >> 8;
    *b = (palette[color].value & 0x0000FF);
}

Color ColorFromName(const char * name)
{
    int num_colors = ARR_SIZE(palette);

    for ( Color i = 0; i < num_colors; i++ ) {
        if ( CaseCompare(name, palette[i].name) == 0 ) {
            return i;
        }
    }

    return INVALID_COLOR;
}

const char * ColorName(Color color)
{
    return palette[color].name;
}

/** Darken light colors or lighten dark colors */
SDL_Color AdjustTone(Color color, int amount)
{
    SDL_Color c;
    GetColorComponents(color, &c.r, &c.g, &c.b);

    int average = (c.r + c.b + c.g) / 3;

    if ( amount > 127 ) {
        amount = 127;
    }

    int adjust = amount * (average < 128 ? 1 : -1);
    c.r += adjust;
    c.g += adjust;
    c.b += adjust;

    return c;
}
