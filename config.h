//
//  config.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef config_h
#define config_h

#include "qe.h"
#include "color.h"
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
extern Color _fg_color;
extern Color _bg_color;
extern Color _line_number_color;

void LoadConfig(const char * file_name);

#endif /* config_h */
