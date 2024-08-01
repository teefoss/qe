#include <stdio.h>

if ( variable == 5 && other_var != 34 ) {

}

int main(int argc, char ** argv)
{
    "Hello there";
    return 0;
    Does this look OK
}

static void Redraw(void)
{
    int line_num_digits = GetNumberOfDigits(buffer.num_lines);
    int line_numbers_width = ROUND((line_num_digits + 2) * _char_w);
    int text_x = line_numbers_width; // Text starts right after line numbers.

    int window_w = WindowWidth();
    int window_h = WindowHeight();
    float tray_bot = TrayBottom();

    // Background Color
    SDL_Rect main_area = {
        .x = 0,
        .y = tray_bot,
        .w = window_w,
        .h = window_h - tray_bot
    };
    FillRect(main_area, ColorToSDL(_bg_color));

    if ( _col_limit != 0 ) {
        int text_w = ROUND(_col_limit * _char_w);
        // Side Area
        SDL_Rect side_area = {
            .x = text_x + text_w,
            .y = tray_bot,
            .w = window_w - text_w,
            .h = window_h - tray_bot
        };
        SDL_Color side_area_color = AdjustTone(_bg_color, 8);
        FillRect(side_area, side_area_color);

        // Main Area / Side Area Separator Line
        SDL_Rect separator = {
            .x = side_area.x,
            .y = tray_bot,
            .w = 1,
            .h = window_h - tray_bot
        };
        SDL_Color separator_color = AdjustTone(_bg_color, 32);
        FillRect(separator, separator_color);
    }

    int row = 0;
    int y = tray_bot;
    Line * line;
    int line_num = top_line_num;
    float partial_line_y = top_line_num_f - floorf(top_line_num_f);
    int line_height = _char_h + _line_spacing;

    for ( line = GetLine(&buffer, (int)top_line_num_f);
          line != NULL && y < window_h;
          line = line->next, row++, y += line_height, line_num++ )
    {
        int draw_y = (float)y - partial_line_y * (float)_char_h;
        int line_num2 = line_num;
        if ( partial_line_y > 0.0f ) {
            line_num2--;
        }

        //
        // Line hightlight box
        //

        SDL_Color line_bg_color = ColorToSDL(_bg_color);
        if ( _highlight_line && line_num == cy ) {
            SDL_Rect highlight_rect = {
                .x = 0,
                .y = draw_y,
                .w = window_w,
                .h = line_height
            };
            line_bg_color = AdjustTone(_bg_color, 16);
            FillRect(highlight_rect, line_bg_color);
        }

        //
        // Line numbers
        //

        SDL_Color line_number_color = AdjustTone(_primary_color, 128);
        DrawFormat(0, draw_y,
                   line_number_color,
                   ColorToSDL(_bg_color),
                   " %*d ", line_num_digits, line_num2 + 1);

        //
        // Line
        //

//        DrawString(text_x, draw_y, ColorToSDL(_primary_color), line->chars);

        UpdateLineColor(line);
        for ( int i = 0; i < line->num_tokens; i++ ) {
            Token t = line->tokens[i];
            char * word = calloc(t.len + 1, 1); // TODO: wut... no
            strncpy(word, line->chars + t.start, t.len);
            DrawString(text_x + ROUND(t.start * _char_w),
                       draw_y,
                       ColorToSDL(t.color),
                       line_bg_color,
                       word);
            free(word);
        }

        //
        // Cursor
        //

        if ( line_num == cy ) {
            SDL_Rect cursor_rect = {
                .x = text_x + ROUND(cx * _char_w),
                .y = draw_y,
                .w = _char_w, // TODO: round this?
                .h = _char_h
            };

            if ( cursor_blink && !TrayIsOpen() ) {
//                FillRect(cursor_rect, ColorToSDL(_secondary_color));
                char cursor_ch[2] = { line->chars[cx], '\0' };
                DrawString(cursor_rect.x, cursor_rect.y, 
                           ColorToSDL(_bg_color),
                           ColorToSDL(_secondary_color),
                           cursor_ch);
            }
        }
    }

    if ( TrayIsOpen() ) {
        DrawTray(cursor_blink);
    }

    UpdateWindow();
}
