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

#define PALETTE_ENTRY(val, e) [e] = { .value = val, .name = #e }

static const PaletteColor palette[] = {
    PALETTE_ENTRY( 0x000000, BLACK ),
    PALETTE_ENTRY( 0xFFFFFF, WHITE ),
    PALETTE_ENTRY( 0x404040, DARK_GRAY ),
    PALETTE_ENTRY( 0x808080, GRAY ),
    PALETTE_ENTRY( 0xC0C0C0, LIGHT_GRAY ),
    PALETTE_ENTRY( 0x000080, DARK_BLUE ),
    PALETTE_ENTRY( 0x0080FF, BLUE ),
    PALETTE_ENTRY( 0x80C0FF, LIGHT_BLUE ),
    PALETTE_ENTRY( 0x008000, DARK_GREEN ),
    PALETTE_ENTRY( 0x00FF80, GREEN ),
    PALETTE_ENTRY( 0xC0FFC0, LIGHT_GREEN ),
    PALETTE_ENTRY( 0x008080, DARK_CYAN ),
    PALETTE_ENTRY( 0x00C0C0, CYAN ),
    PALETTE_ENTRY( 0x80FFFF, LIGHT_CYAN ),
    PALETTE_ENTRY( 0x800000, DARK_RED ),
    PALETTE_ENTRY( 0xFF0080, RED ),
    PALETTE_ENTRY( 0xFF80C0, LIGHT_RED ),
    PALETTE_ENTRY( 0x800080, DARK_PURPLE ),
    PALETTE_ENTRY( 0x8000FF, PURPLE ),
    PALETTE_ENTRY( 0xFF80FF, PINK ),
    PALETTE_ENTRY( 0x804000, BROWN ),
    PALETTE_ENTRY( 0xFF8000, ORANGE ),
    PALETTE_ENTRY( 0xFFFF40, YELLOW ),
};

SDL_Color ColorToSDL(Color color)
{
    return (SDL_Color){
        .r = EXTRACT_FIELD(palette[color].value, R_MASK),
        .g = EXTRACT_FIELD(palette[color].value, G_MASK),
        .b = EXTRACT_FIELD(palette[color].value, B_MASK)
    };
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

static int AdjustComponent(Color color, int component_mask, int amount)
{
    int component = EXTRACT_FIELD(palette[color].value, component_mask);

    if ( component >= 128 ) {
        return MAX(0, component - amount);
    } else {
        return MIN(255, component + amount);
    }
}

/** 
 *  Darken light colors or lighten dark colors by a given amount.
 *  - parameter color: The color to adjust.
 *  - parameter amount: A positive value by which to lighten or darken `color`.
 */
SDL_Color AdjustTone(Color color, int amount)
{
    ASSERT(amount > 0);

    int r = AdjustComponent(color, R_MASK, amount);
    int g = AdjustComponent(color, G_MASK, amount);
    int b = AdjustComponent(color, B_MASK, amount);

    return (SDL_Color){ r, g, b, 255 };
}
