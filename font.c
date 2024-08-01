//
//  font.c
//  qe
//
//  Created by Thomas Foster on 7/22/24.
//

#include "font.h"

#include "config.h"
#include "window.h"
#include "misc.h"
#include <SDL_ttf.h>

float _char_w;
int _char_h;

static const int num_font_sizes = 19;
static const int font_sizes[] = {
    6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 30, 36, 42, 48, 60, 72
};

static TTF_Font * font;
static int font_index; // index into `font_size`, current font size.

static int GetFontIndex(int size)
{
    for ( int i = 0; i < num_font_sizes; i++ ) {
        if ( font_sizes[i] >= size ) {
            return i;
        }
    }

    return num_font_sizes - 1;
}

void InitFont(void)
{
    if ( TTF_Init() != 0 ) {
        fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void ChangeFontSize(int incrememnt)
{
    font_index += incrememnt;
    if ( font_index < 0 ) {
        font_index = 0;
    } else if ( font_index >= num_font_sizes ) {
        font_index = num_font_sizes - 1;
    }

    int size = font_sizes[font_index] * _draw_scale;

    TTF_SetFontSize(font, size);

    int min_x, max_x, min_y, max_y, advance;
    TTF_GlyphMetrics(font, 'A', &min_x, &max_x, &min_y, &max_y, &advance);
//    _char_w = advance;
    _char_h = TTF_FontHeight(font);

    printf("set font size: %d\n", size);
#if 1
    printf("- advance: %d\n", advance);
    printf("- height:  %d\n", TTF_FontHeight(font));
    printf("- ascent:  %d\n", TTF_FontAscent(font));
//    printf("- char w:  %f\n", _char_w);
    printf("- char h:  %d\n", _char_h);
#endif

    // Calculate floating point advance
    #define TEST_BUF_SIZE 100
    char buf[TEST_BUF_SIZE + 1];
    memset(buf, 'A', TEST_BUF_SIZE);
    buf[TEST_BUF_SIZE] = '\0';

    SDL_Surface * surface = CreateTextSurface(buf, (SDL_Color){0}, (SDL_Color){0});
    _char_w = (float)surface->w / TEST_BUF_SIZE;
    printf("char w: %f\n", _char_w);
    SDL_FreeSurface(surface);
    #undef TEST_BUF_SIZE
}

void LoadFont(void)
{
    font_index = GetFontIndex(_font_size);
    if ( font_sizes[font_index] != _font_size ) {
        // TODO: update user config to valid size.
    }

    if ( font != NULL ) {
        TTF_CloseFont(font);
    }

    font = TTF_OpenFont(_font_path, font_sizes[font_index] * _draw_scale);
    if ( font == NULL ) {
        fprintf(stderr, "Could not load font: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    ChangeFontSize(0);
}

SDL_Surface * CreateTextSurface(const char * string, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface * text = NULL;

//    text = TTF_RenderText_LCD(font, string, fg, bg);
    if ( text == NULL ) {
        text = TTF_RenderText_Shaded(font, string, fg, bg);
        if ( text == NULL ) {
            DieGracefully("Error: failed to create text (%s)", SDL_GetError());
        }
    }

    return text;
}
