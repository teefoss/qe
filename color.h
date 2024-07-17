//
//  color.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef color_h
#define color_h

#include "qe.h"

#define R_MASK 0xFF0000
#define G_MASK 0x00FF00
#define B_MASK 0x0000FF
#define INVALID_COLOR 255

typedef uint8_t Color;

typedef enum {
    BLACK,
    WHITE,
    DARK_GRAY,
    GRAY,
    LIGHT_GRAY,
    DARK_BLUE,
    BLUE,
    LIGHT_BLUE,
    DARK_GREEN,
    GREEN,
    LIGHT_GREEN,
    DARK_CYAN,
    CYAN,
    LIGHT_CYAN,
    DARK_RED,
    RED,
    LIGHT_RED,
    DARK_PURPLE,
    PURPLE,
    PINK,
    BROWN,
    ORANGE,
    YELLOW,
} Colors;

SDL_Color ColorToSDL(Color color);
Color ColorFromName(const char * name);
const char * ColorName(Color color);
SDL_Color AdjustTone(Color color, int amount);

#endif /* color_h */
