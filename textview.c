//
//  textview.c
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#include "textview.h"

#include "config.h"
#include "font.h"
#include "misc.h"
#include "window.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void MoveCursorLeft(TextView * view)
{
    Line * current = GetLine(&view->buffer, view->cy);

    --view->cx;
    if ( view->cx < 0 ) {
        if ( view->cy > 0 ) { // go to end of prev line
            --view->cy;
            view->cx = current->prev->len;
        } else {
            view->cx = 0; // top of document
        }
    }
}

void MoveCursorRight(TextView * view)
{
    Line * current = GetLine(&view->buffer, view->cy);

    if ( view->cx < current->len ) {
        ++view->cx;
    } else if ( current->next ) {
        // passed EOL, move down if not bottom
        ++view->cy;
        view->cx = 0;
    }
}

void MoveCursorUp(TextView * view)
{
    Line * current = GetLine(&view->buffer, view->cy);

    if ( view->cy > 0 ) {
        --view->cy;

        current = GetLine(&view->buffer, view->cy);
        if ( view->cx > current->len ) {
            view->cx = current->len; // snap to end of line
        }
    }
}

void MoveCursorDown(TextView * view)
{
    Line * current = GetLine(&view->buffer, view->cy);

    if ( current->next == NULL ) {
        view->cx = current->len;
    } else {
        ++view->cy;
//                int rows = WindowHeight() / _char_h;
//                if ( buf->cy - top_line_num > rows - 5 ) {
//                    ++top_line_num;
//                }
    }
}

static void InsertNewLine(TextView * view)
{
    Line * current = GetLine(&view->buffer, view->cy);
    Line * newline = NewLine();

    if ( view->cy == 0 && view->cx == 0 ) {
        InsertLineBefore(&view->buffer, newline, current);
    } else if ( view->cx == 0 ) {
        InsertLineAfter(&view->buffer, newline, current->prev);
    } else if ( view->cx == current->len ) {
        InsertLineAfter(&view->buffer, newline, current);
    } else { // We are mid-line.
        InsertLineAfter(&view->buffer, newline, current);
        char * cursor_string = current->chars + view->cx;
        InsertChars(newline, cursor_string, (int)strlen(cursor_string), 0);
        RemoveChars(current, current->len - view->cx, view->cx);
    }

    view->cx = 0;
    view->cy++;
}

static void PasteFromClipboard(TextView * view)
{
    if ( !SDL_HasClipboardText() ) {
        return;
    }

    char * clipboard_text = SDL_GetClipboardText();
    char * this = clipboard_text;

    // Handle new line characters that may be present in the pasted text.
    do {
        Line * line = GetLine(&view->buffer, view->cy);
        char * next = strchr(this, '\n');

        // Calculate the length of 'this':
        int this_len;
        if ( next ) {
            this_len = (int)(next - this);
            next++; // Advance past the new line.
        } else {
            this_len = (int)strlen(this);
        }

        InsertChars(line, this, this_len, view->cx);
        view->cx += this_len;

        if ( next ) {
            InsertNewLine(view);
        }

        this = next;
    } while ( this != NULL );

    SDL_free(clipboard_text);
}

static void Backspace(TextView * view)
{
    if ( view->cx == 0 && view->cy == 0 ) {
        return;
    }

    Line * current = GetLine(&view->buffer, view->cy);

    if ( view->cx == 0 ) { // Append current line onto end of previous line.
        --view->cy;
        view->cx = current->prev->len;
        InsertChars(current->prev, current->chars, (int)strlen(current->chars), view->cx);
        RemoveLine(&view->buffer, current);
    } else { // We are mid-line.
        RemoveChars(current, 1, view->cx - 1);
        view->cx--;
    }
}

static void ClampCursor(TextView * view)
{
    if ( view->cy < 0 ) {
        view->cy = 0;
    } else if ( view->cy >= view->buffer.num_lines ) {
        view->cy = (int)view->buffer.num_lines - 1;
    }

    Line * current = GetLine(&view->buffer, view->cy);
    if ( view->cx > current->len ) {
        view->cx = current->len;
    }
}

void PageUp(TextView * buf)
{
    buf->cy -= 24;
    ClampCursor(buf);
}

void PageDown(TextView * buf)
{
    buf->cy += 24;
    ClampCursor(buf);
}

void MoveToTop(TextView * buf)
{
    buf->cy = 0;
    buf->cx = 0;
}

void MoveToBottom(TextView * view)
{
    view->cx = GetLine(&view->buffer, view->buffer.num_lines - 1)->len;
    view->cy = view->buffer.num_lines - 1;
}

Action actions[] = {
    [BUF_MOVE_UP] = {
        .inputs = {
            { SDLK_UP },
            { SDLK_i, KMOD_CAPS },
        },
        .func = MoveCursorUp,
    },
    [BUF_MOVE_DOWN] = {
        .inputs = {
            { SDLK_DOWN },
            { SDLK_k, KMOD_CAPS },
        },
        .func = MoveCursorDown,
    },
    [BUF_MOVE_LEFT] = {
        .inputs = {
            { SDLK_LEFT },
            { SDLK_j, KMOD_CAPS },
        },
        .func = MoveCursorLeft,
    },
    [BUF_MOVE_RIGHT] = {
        .inputs = {
            { SDLK_RIGHT },
            { SDLK_l, KMOD_CAPS },
        },
        .func = MoveCursorRight,
    },
    [BUF_PAGE_UP] = {
        .inputs = {
            { SDLK_PAGEUP },
            { SDLK_UP, KMOD_ALT },
            { SDLK_i, KMOD_CAPS | KMOD_ALT },
        },
        .func = PageUp,
    },
    [BUF_PAGE_DOWN] = {
        .inputs = {
            { SDLK_PAGEDOWN },
            { SDLK_DOWN, KMOD_ALT },
            { SDLK_k, KMOD_CAPS | KMOD_ALT },
        },
        .func = PageDown,
    },
    [BUF_MOVE_TO_TOP] = {
        .inputs = {
            { SDLK_UP, CMD_KEY }
        },
        .func = MoveToTop
    },
    [BUF_MOVE_TO_BOTTOM] = {
        .inputs = {
            { SDLK_DOWN, CMD_KEY }
        },
        .func = MoveToBottom
    },
    [BUF_INSERT_LINE] = {
        .inputs = {
            { SDLK_RETURN }
        },
        .func = InsertNewLine,
    },
    [BUF_BACKSPACE] = {
        .inputs = {
            { SDLK_BACKSPACE },
            { SDLK_d, KMOD_CAPS | KMOD_SHIFT },
        },
        .func = Backspace
    },
};

bool BufferRespond(TextView * buf, SDL_Keycode key, SDL_Keymod mods)
{
    for ( BufferActionType type = 0; type < NUM_BUF_ACTIONS; type++ ) {
        if ( type == BUF_PAGE_DOWN ) {

        }
        if ( buf->actions[type] ) {
            for ( Input * in = actions[type].inputs; in->key != 0; in++ ) {
                if ( in->key == key && ((in->mods & mods) == mods) ) {
                    actions[type].func(buf);
                    return true;
                }
            }
        }
    }

    return false;
}

float GetBufferYOffset(void)
{
    return 0;
}

bool UpdateBufferOffsets(TextView * buf, float buf_height_pixels)
{
    const float buf_cy_pixel = buf->cy * _char_h;
    const float margin = buf_height_pixels * 0.3f;

    // Check if the distance between the cursor line and y offset is too
    // large or small and calculate where the offset should be (dest_y).
    float dy = buf_cy_pixel - buf->offset_y;
    float dest_y;

    if ( dy < margin ) {
        dest_y = buf_cy_pixel - margin;
    } else if ( dy > buf_height_pixels - margin ) {
        dest_y = buf_cy_pixel - (buf_height_pixels - margin);
    } else {
        return false;
    }

    if ( dest_y < -(float)_margin) {
        dest_y = -_margin;
    }

    // Move offset_y toward dest_y
    buf->offset_y -= (buf->offset_y - dest_y) * 0.05f;
    if ( fabsf(buf->offset_y - dest_y) < 0.5f ) { // Close enough?
        buf->offset_y = dest_y;
    }

    return true;
}

static int GetNumberOfDigits(size_t number)
{
    int num_digits = 0;
    while ( number != 0 ) {
        number /= 10;
        num_digits++;
    }

    return num_digits;
}

const char * GetTokenString(const Token * token)
{
    static char * string = NULL;
    static size_t allocated = 0;

    if ( string == NULL ) {
        allocated = 256;
        string = Allocate(allocated);
    } else if ( allocated < token->len + 1 ) {
        while ( allocated < token->len + 1 ) {
            allocated *= 2;
        }
        string = Reallocate(string, allocated);
    }

    strncpy(string, token->start, token->len);
    string[token->len] = '\0';

    return string;
}

void DrawTextView(TextView * view, SDL_Rect rect, bool show_cursor)
{
    const int top_visible_line = view->offset_y / (float)_char_h;
    const int num_lines = rect.h / _char_h + 1;
    const int num_line_num_digits = GetNumberOfDigits(view->buffer.num_lines + 1);
    const SDL_Color bg = ColorToSDL(_bg_color);

    FillRect(rect, ColorToSDL(_bg_color));

    for ( int line_num = top_visible_line < 0 ? 0 : top_visible_line;
          line_num < top_visible_line + num_lines;
          line_num++ )
    {
        if ( line_num >= view->buffer.num_lines ) {
            break;
        }

        Line * line = GetLine(&view->buffer, line_num);

        int x = rect.x;
        int y = rect.y + (line_num * _char_h) - view->offset_y;

        if ( _line_numbers ) {
            x += DrawFormat(x, y, AdjustTone(_bg_color, 64), bg,
                            "%*d ", num_line_num_digits, line_num);
        }
        
//        if ( 1 ) {
        if ( num_keywords == 0 ) {
            DrawString(x, y, ColorToSDL(_primary_color), bg, line->chars);
        } else {
            UpdateTokens(line);
            for ( int i = 0; i < line->num_tokens; i++ ) {
                const Token * token = &line->tokens[i];

                SDL_Color fg = ColorToSDL(token->is_keyword
                                          ? _secondary_color
                                          : _primary_color);

                const char * token_string = GetTokenString(token);
                int token_x = x + (int)(token->start - line->chars) * _char_w;
                DrawString(token_x, y, fg, bg, token_string);
            }
        }

        if ( view->cy == line_num && show_cursor ) {
            SDL_Rect r = {
                .x = x + ROUND(_char_w * view->cx),
                .y = y,
                .w = ROUND(_char_w),
                .h = _char_h,
            };
//            FillRect(r, ColorToSDL(_primary_color));
            
        }
    }

    UpdateWindow();
}
