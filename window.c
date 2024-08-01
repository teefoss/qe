//
//  window.c
//  qe
//
//  Created by Thomas Foster on 5/16/24.
//

#include "window.h"

#include "buffer.h"
#include "config.h"
#include "font.h"
#include "misc.h"

#include <stdarg.h>
#include <stdlib.h>

int _draw_scale;
int _margin;

static SDL_Window * window;
static SDL_Surface * window_surface;

int WindowWidth(void)
{
    return window_surface->w;
}

int WindowHeight(void)
{
    return window_surface->h;
}

void UpdateWindow(void)
{
    SDL_UpdateWindowSurface(window);
}

void WindowDidResize(void)
{
    window_surface = SDL_GetWindowSurface(window);
}

// TODO: have a key command for this?
void SetWindowWidthToFit(int num_line_cols)
{
    if ( _col_limit == 0 ) {
        return;
    }

    int line_nums_w = (num_line_cols + 2) * _char_w;
    int main_area_w = _col_limit * _char_w;
    int w = _margin + line_nums_w + main_area_w;
    int h;
    SDL_GetWindowSize(window, NULL, &h);
    SDL_SetWindowSize(window, w, h);
}

void SetWindowTitle(const char * title)
{
    SDL_SetWindowTitle(window, title);
}

void InitWindow(void)
{
    u32 window_flags = 0;
    window_flags |= SDL_WINDOW_RESIZABLE;
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    const int h = 480;
    window = SDL_CreateWindow("", 128, 128, 640, h, window_flags);
    window_surface = SDL_GetWindowSurface(window);

    _draw_scale = window_surface->h / h;
    printf("render scale: %d\n", _draw_scale);
    _margin = 2 * _draw_scale;
}

void FillRect(SDL_Rect rect, SDL_Color color)
{
    u32 color32 = SDL_MapRGB(window_surface->format, color.r, color.g, color.b);
    SDL_FillRect(window_surface, &rect, color32);
}

int DrawString(int x, int y, SDL_Color fg, SDL_Color bg, const char * string)
{
    if ( string == NULL || *string == '\0' ) {
        return 0;
    }

    SDL_Surface * text = CreateTextSurface(string, fg, bg);

    SDL_Rect dst = { x, y, text->w, text->h };
    SDL_UpperBlitScaled(text, NULL, window_surface, &dst);
    int width = text->w;
    SDL_FreeSurface(text);

    return width;
}

int DrawFormat(int x, int y, SDL_Color fg, SDL_Color bg, const char * format, ...)
{
    static char * buffer = NULL; // N.B. let OS free on program end.
    static size_t allocated = 0;

    va_list len_args, print_args;
    va_start(len_args, format);
    va_copy(print_args, len_args);

    int len = vsnprintf(NULL, 0, format, len_args);
    va_end(len_args);

    size_t size_needed = len + 1;
    if ( allocated < size_needed ) {
        buffer = Reallocate(buffer, size_needed);
        allocated = size_needed;
    }

    vsnprintf(buffer, len + 1, format, print_args);
    va_end(print_args);
    buffer[len] = '\0';

    return DrawString(x, y, fg, bg, buffer);
}
