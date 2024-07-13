//
//  color.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef color_h
#define color_h

#include "qe.h"

#define INVALID_COLOR 255

typedef uint8_t Color;

SDL_Color ColorToSDL(Color color);
void GetColorComponents(Color color, u8 * r, u8 * g, u8 * b);
Color ColorFromName(const char * name);
const char * ColorName(Color color);
SDL_Color AdjustTone(Color color, int amount);

#endif /* color_h */
