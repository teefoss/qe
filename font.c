//
//  font.c
//  qe
//
//  Created by Thomas Foster on 7/22/24.
//

#include "font.h"

#include "config.h"
#include "window.h"
#include <SDL_ttf.h>

int _char_w;
int _char_h;

static const int num_font_sizes = 20;
static const int font_sizes[] = {
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 30, 36, 42, 48, 60, 72
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
    TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &_char_w);
    _char_h = TTF_FontHeight(font);
//    _char_h = size;

    printf("font size set to %d\n", font_sizes[font_index]);
    printf("- char w: %d\n", _char_w);
    printf("- char h: %d\n", _char_h);
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

    TTF_SetFontHinting(font, TTF_HINTING_MONO);
    ChangeFontSize(0);
}

SDL_Surface * CreateTextSurface(const char * string, Color color)
{
    SDL_Color fg = ColorToSDL(color);
    SDL_Surface * text = TTF_RenderText_Blended(font, string, fg);

    if ( text == NULL ) {
        DieGracefully("Error: failed to create text");
    }

    return text;
}
