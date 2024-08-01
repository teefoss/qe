//
//  qe.c
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#include "qe.h"

#include "textview.h"
#include "config.h"
#include "font.h"
#include "tray.h"
#include "window.h"

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#define BLINK_MS 333

static const u8 * keys;
static SDL_Keymod mods;

static TextView document_view = {
    .actions = {
        [BUF_MOVE_LEFT] = true,
        [BUF_MOVE_UP] = true,
        [BUF_MOVE_DOWN] = true,
        [BUF_MOVE_RIGHT] = true,
        [BUF_PAGE_UP] = true,
        [BUF_PAGE_DOWN] = true,
        [BUF_INSERT_LINE] = true,
        [BUF_BACKSPACE] = true,
        [BUF_MOVE_TO_TOP] = true,
        [BUF_MOVE_TO_BOTTOM] = true,
    }
};
static const char * document_path;

SDL_Rect doc_buf_rect;

//static float top_line_num_f;
//static int top_line_num; // The line visible at the top

//static int cx, cy; // Cursor position
static bool cursor_blink = true;
static int next_blink_time;

static bool needs_refresh;

SDL_Rect DocumentBufferRect(void)
{
    return (SDL_Rect){ _margin, 0, WindowWidth(), WindowHeight() };
}

#if 0
static void Redraw(const Buffer * buffer)
{
    const int cx = buffer->cx;
    const int cy = buffer->cy;

    int line_num_digits = GetNumberOfDigits(document_buffer.num_lines);
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

    for ( line = GetLine(&document_buffer, (int)top_line_num_f);
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
#endif

void LoadDocument(const char * path, bool create, int line_number)
{
    FreeBuffer(&document_view.buffer);
    document_path = path;

    FILE * file = fopen(document_path, "r");
    if ( file == NULL ) {
        if ( create ) {
            file = fopen(document_path, "w");
            if ( file == NULL ) {
                printf("error: failed to create '%s'", document_path);
                exit(EXIT_FAILURE);
            }
            AppendLine(&document_view.buffer, NewLine());
        } else {
            printf("error: Could not open '%s'. Does it exist?\n",
                   document_path);
            exit(EXIT_FAILURE);
        }
    } else {
        LoadBuffer(&document_view.buffer, file);
    }

    fclose(file);
    LoadConfig(document_path);

    LoadFont();

    const char * display_name = strrchr(document_path, PATH_SEP);
    if ( display_name ) {
        display_name++;
    } else {
        display_name = path;
    }

    SetWindowTitle(display_name);
    DrawTextView(&document_view, doc_buf_rect, false);
}

/**
 *  Scroll editor to `new_line_num` (zero-indexed).
 */
#if 0
void Scroll(Buffer * buffer, size_t line_num)
{
    if ( line_num < 0 ) {
        line_num = 0;
    } else if ( line_num >= document_buffer.num_lines ) {
        line_num = document_buffer.num_lines - 1;
    }

    buffer->cy = (int)line_num;
    top_line_num = buffer->cy - WindowHeight() / _char_h / 2; // TODO: editor region

    if ( top_line_num < 0 ) {
        top_line_num = 0;
    }
//    Redraw();
}
#endif

int EventWatch(void * user_data, SDL_Event * event)
{
    if ( event->type == SDL_WINDOWEVENT
        && event->window.event == SDL_WINDOWEVENT_EXPOSED )
    {
        WindowDidResize();
        doc_buf_rect = DocumentBufferRect();
        DrawTextView(&document_view, doc_buf_rect, false);
    }

    return 1;
}

void RestartCursor(void)
{
    next_blink_time = SDL_GetTicks() + BLINK_MS;
    cursor_blink = true;
    needs_refresh = true;
}

// TODO: move to textview.c
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

// TODO: move to textview.c
void JumpToBeginningOfLine(TextView * view)
{
    Line * line = GetLine(&view->buffer, view->cy);
    if ( line->len == 0 ) {
        return;
    }

    int x;
    for ( x = 0; x < line->len; x++ ) {
        if ( !isspace(line->chars[x]) ) {
            break;
        }
    }

    if ( view->cx > x ) {
        view->cx = x;
    } else {
        view->cx = 0;
    }
}

static void DoEditorKey(SDL_Keycode key)
{
    if ( BufferRespond(&document_view, key, mods) ) {
        return;
    }

    switch ( key ) {
#if 0
        case SDLK_UP:
            if ( mods & CMD_KEY ) {
                Scroll(0);
            } else {
                MoveCursor(&document_buffer, UP);
            }
            break;
        case SDLK_DOWN:
            if ( mods & CMD_KEY ) {
                Scroll(document_buffer.num_lines - 1);
            } else {
                MoveCursor(&document_buffer, DOWN);
            }
            break;
        case SDLK_RIGHT: {
            Line * line = GetLine(&document_buffer, cy);
            if ( mods & CMD_KEY ) {
                cx = line->len;
            } else if ( mods & KMOD_ALT ) {
                JumpToEndOfWord(line, &cx);
            } else {
                MoveCursor(&document_buffer, RIGHT);
            }
            break;
        }
        case SDLK_LEFT:
            if ( mods & CMD_KEY ) {
                JumpToBeginningOfLine();
            } else if ( mods & KMOD_ALT ) {
                JumpToBeginningOfWord(GetLine(&document_buffer, cy), &cx);
            } else {
                MoveCursor(&document_buffer, LEFT);
            }
            break;
        case SDLK_PAGEUP: {
            int visible_rows = WindowHeight() / _char_h;
            Scroll(cy - visible_rows / 2);
            break;
        }
        case SDLK_PAGEDOWN: {
            int visible_rows = WindowHeight() / _char_h;
            Scroll(cy + visible_rows / 2);
            break;
        }

        case SDLK_RETURN:
            InsertNewLine();
            break;
        case SDLK_BACKSPACE:
            Backspace();
            break;
        case SDLK_i:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(&document_buffer, UP);
            }
            break;
        case SDLK_j:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(&document_buffer, LEFT);
            }
            break;
        case SDLK_k:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(&document_buffer, DOWN);
            }
            break;
        case SDLK_l:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(&document_buffer, RIGHT);
            } else if ( mods & CMD_KEY ) {
                OpenTray(TRAY_LINE_NUM);
            }
            break;
#endif
        case SDLK_s:
            if ( mods & CMD_KEY ) {
                WriteBuffer(&document_view.buffer, document_path);
            }
            break;
        case SDLK_ESCAPE:
            CloseTray();
            break;
        default:
            break;
    }
}

int ProgramLoop(void)
{
    SDL_AddEventWatch(EventWatch, NULL);
    keys = SDL_GetKeyboardState(NULL);
    SDL_StartTextInput();

    bool quit_requested = false;
    RestartCursor();

    FillRect((SDL_Rect){0, 0, WindowWidth(), WindowHeight()}, _bg_color);
    doc_buf_rect = DocumentBufferRect();
    DrawTextView(&document_view, doc_buf_rect, false);

    while ( !quit_requested ) {
        mods = SDL_GetModState();
        needs_refresh = false;

        int now = SDL_GetTicks();
        if ( now >= next_blink_time ) {
            next_blink_time += BLINK_MS;
            cursor_blink = !cursor_blink;
            needs_refresh = true;
        }

        SDL_Event event;
        while ( SDL_PollEvent(&event) ) {
            switch ( event.type ) {
                case SDL_QUIT:
                    if ( !(mods & KMOD_SHIFT) ) {
                        WriteBuffer(&document_view.buffer, document_path);
                    }
                    quit_requested = true;
                    break;
                case SDL_KEYDOWN: {
                    needs_refresh = true;
                    RestartCursor();

                    switch ( event.key.keysym.sym ) {
                        case SDLK_EQUALS:
                            if ( mods & CMD_KEY ) {
                                ChangeFontSize(+1);
                            }
                            break;
                        case SDLK_MINUS:
                            if ( mods & CMD_KEY ) {
                                ChangeFontSize(-1);
                            }
                            break;
                        default:
                            break;
                    }

                    if ( TrayIsOpen() ) {
                        DoTrayKey(event.key.keysym.sym);
                    } else {
                        DoEditorKey(event.key.keysym.sym);
                    }
                    break;
                }
                case SDL_TEXTINPUT: {
                    if ( isprint(event.text.text[0]) ) {
                        if ( TrayIsOpen() ) {
                            DoTrayTextInput(event.text.text[0]);
                        } else {
                            Line * line = GetLine(&document_view.buffer, document_view.cy);
                            InsertChars(line, &event.text.text[0], 1, document_view.cx);
                            document_view.cx++;
                        }
                        needs_refresh = true;
                        RestartCursor();
                    }
                    break;
                }
                default:
                    break;
            }
        }

        // Smooth scrolling!
#if 0
        float dy = top_line_num_f - (float)top_line_num;
        if ( fabsf(dy) > 0.1f ) {
            top_line_num_f -= dy * 0.4f;
            needs_refresh = true;
        } else {
            top_line_num_f = top_line_num;
        }
#endif
        if ( UpdateBufferOffsets(&document_view, doc_buf_rect.h) ) {
            needs_refresh = true;
        }

        if ( UpdateTray() ) {
            needs_refresh = true;
        }

        if ( needs_refresh ) {
            FillRect((SDL_Rect){0, 0, WindowWidth(), WindowHeight()}, _bg_color);
            DrawTextView(&document_view, doc_buf_rect, cursor_blink);
        } else {
            SDL_Delay(20);
        }
    }

    return EXIT_SUCCESS;
}
