//
//  config.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef config_h
#define config_h

#define MAX_KEYWORDS 256
#define MAX_KEYWORD_LEN 64

#include "qe.h"
#include <stdbool.h>

extern bool _use_spaces;
extern bool _case_sensitive;
extern bool _line_numbers;
extern bool _highlight_line;
extern char _font_path[];
extern char _theme[];
extern int  _tab_size;
extern int  _font_size;
extern int  _line_spacing;
extern int  _col_limit;
extern int  _win_w;
extern int  _win_h;
extern SDL_Color _primary_color;
extern SDL_Color _secondary_color;
extern SDL_Color _bg_color;
extern SDL_Color _keyword_color;

extern int num_keywords;
extern char keywords[MAX_KEYWORDS][MAX_KEYWORD_LEN];

void LoadConfig(const char * file_name);
bool IsKeyword(const char * word);

#endif /* config_h */
