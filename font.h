//
//  font.h
//  qe
//
//  Created by Thomas Foster on 7/22/24.
//

#ifndef font_h
#define font_h

#include <SDL.h>

extern float _char_w;
extern int _char_h;

void InitFont(void);
void LoadFont(void);
void ChangeFontSize(int incrememnt);
SDL_Surface * CreateTextSurface(const char * string, SDL_Color fg, SDL_Color bg);

#endif /* font_h */
