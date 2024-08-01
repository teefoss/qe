//
//  window.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>

extern int _draw_scale;
extern int _margin;

void InitWindow(void);
void LoadFont(void);
int  WindowWidth(void);
int  WindowHeight(void);
void UpdateWindow(void);
void WindowDidResize(void);
void SetWindowWidthToFit(int num_line_cols);
void SetWindowTitle(const char * title);
void FillRect(SDL_Rect rect, SDL_Color color);
int DrawString(int x, int y, SDL_Color fg, SDL_Color bg, const char * string);
int DrawFormat(int x, int y, SDL_Color fg, SDL_Color bg, const char * format, ...);
void ChangeFontSize(int incrememnt);
int Margin(void);
SDL_Color AdjustTone(SDL_Color color, int amount);

#endif /* WINDOW_H */
