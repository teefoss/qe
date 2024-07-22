//
//  window.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef WINDOW_H
#define WINDOW_H

#include "color.h"
#include <SDL.h>

extern int _margin;
extern int _char_w;
extern int _char_h;

void InitWindow(void);
void LoadFont(void);
int  WindowWidth(void);
int  WindowHeight(void);
void UpdateWindow(void);
void WindowDidResize(void);
void SetWindowWidthToFit(int num_line_cols);
void SetWindowTitle(const char * title);
void FillRect(SDL_Rect rect, SDL_Color color);
void DrawString(int x, int y, Color color, const char * string);
void DrawFormat(int x, int y, Color color, const char * format, ...);
void ChangeFontSize(int incrememnt);

#endif /* WINDOW_H */
