//
//  window.c
//  qe
//
//  Created by Thomas Foster on 5/16/24.
//

#include "window.h"
#include "buffer.h"
#include "config.h"
#include <stdarg.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>

const int font_sizes[] = {
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 30, 36, 42, 48, 60, 72
};

int _margin;
int _char_w;
int _char_h;

static SDL_Window * window;
static SDL_Surface * draw_surface;

static TTF_Font * font;
static int draw_scale;
static int num_font_sizes;
static int font_index; // index into `font_size`, current font size.

int WindowWidth(void)
{
    return draw_surface->w;
}

int WindowHeight(void)
{
    return draw_surface->h;
}

void UpdateWindow(void)
{
    SDL_UpdateWindowSurface(window);
}

void WindowDidResize(void)
{
    draw_surface = SDL_GetWindowSurface(window);
}

static int GetFontIndex(int size)
{
    for ( int i = 0; i < num_font_sizes; i++ ) {
        if ( font_sizes[i] >= size ) {
            return i;
        }
    }

    return num_font_sizes - 1;
}

void ChangeFontSize(int incrememnt)
{
    font_index += incrememnt;
    if ( font_index < 0 ) {
        font_index = 0;
    } else if ( font_index >= num_font_sizes ) {
        font_index = num_font_sizes - 1;
    }

    int size = font_sizes[font_index] * draw_scale;

    TTF_SetFontSize(font, size);
    TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &_char_w);
    _char_h = TTF_FontHeight(font);
//    _char_h = size;

    printf("font size set to %d\n", font_sizes[font_index]);
    printf("- char w: %d\n", _char_w);
    printf("- char h: %d\n", _char_h);
}

// TODO: have a key command for this?
void SetWindowWidthToFit(void)
{
    if ( _col_limit == 0 ) {
        return;
    }

    int line_nums_w = (LineNumCols() + 2) * _char_w;
    int main_area_w = _col_limit * _char_w;
    int w = _margin + line_nums_w + main_area_w;
    int h;
    SDL_GetWindowSize(window, NULL, &h);
    SDL_SetWindowSize(window, w, h);
}

void InitWindow(void)
{
    num_font_sizes = sizeof(font_sizes) / sizeof(font_sizes[0]);

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int win_w = _win_w == 0 ? 640 : _win_w;
    int win_h = _win_h == 0 ? 480 : _win_h;

    u32 window_flags = 0;
    window_flags |= SDL_WINDOW_RESIZABLE;
    window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow("", 128, 128, win_w, win_h, window_flags);
    draw_surface = SDL_GetWindowSurface(window);

    draw_scale = draw_surface->h / win_h;
    printf("render scale: %d\n", draw_scale);
    _margin = 8 * draw_scale;

    if ( TTF_Init() != 0 ) {
        fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    TTF_SetFontHinting(font, TTF_HINTING_MONO);

    font_index = GetFontIndex(_font_size);
    if ( font_sizes[font_index] != _font_size ) {
        // TODO: update user config to valid size.
    }

    font = TTF_OpenFont(_font_path, font_sizes[font_index] * draw_scale);
    if ( font == NULL ) {
        fprintf(stderr, "Could not load font: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    ChangeFontSize(0);

    if ( _win_w == 0 ) {
        SetWindowWidthToFit();
    }
}

static void Blit(SDL_Surface * surface, int x, int y)
{
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_BlitSurface(surface, NULL, draw_surface, &dst);
}

void FillRect(SDL_Rect rect, SDL_Color color)
{
    u32 color32 = SDL_MapRGB(draw_surface->format, color.r, color.g, color.b);
    SDL_FillRect(draw_surface, &rect, color32);
}

void DrawString(int x, int y, Color color, const char * string)
{
    SDL_Color sdl_fg = ColorToSDL(color);
    //    SDL_Color sdl_bg = GetColor(_bg_color);
    SDL_Surface * text = TTF_RenderText_Blended(font, string, sdl_fg);

    if ( text ) {
        Blit(text, x, y);
    } else {
        DieGracefully();
    }
}

void DrawFormat(int x, int y, Color color, const char * format, ...)
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
        char * temp = realloc(buffer, size_needed);
        if ( temp == NULL ) {
            DieGracefully();
        }
        buffer = temp;
        allocated = size_needed;
    }

    vsnprintf(buffer, len + 1, format, print_args);
    va_end(print_args);
    buffer[len] = '\0';

    DrawString(x, y, color, buffer);
}
