//
//  qe.c
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#include "qe.h"
#include "buffer.h"
#include "config.h"
#include "tray.h"
#include "window.h"

//#include <stdbool.h>
#include <SDL2/SDL.h>
#include <errno.h>
#include <sys/stat.h> // linux/mac: mkdir

// TODO: features
// jump back to prev char
// hold backspace + ...
//  ... w delete word
//  ... c delete char
//  ... l delete line
//  - or -
// ctrl-d to enter delete mode, then w for word, etc.
/*
 pgup - jump to top of visible lines
 C-left word left
 C-A-left beginning line
 C-up pgup
 C-A-up beginning of doc
 */

#define BLINK_MS 333

static const u8 * keys;
static SDL_Keymod mods;

static const char * edit_path;
static int top_line_num;
static int cx;
static int cy;
static bool cursor_blink = true;
static int next_blink_time;
static bool needs_refresh;

int CaseCompare(const char * a, const char * b)
{
    while (*a && (tolower(*a) == tolower(*b)) ) {
        a++;
        b++;
    }

    return tolower(*a) - tolower(*b);
}

void DieGracefully(void)
{
    // TODO: the gracefully part
    exit(EXIT_FAILURE);
}

static void Redraw(void)
{
    Line * top_line = GetLine(top_line_num);

    int line_num_cols = LineNumCols();
    int text_x = (line_num_cols + 1) * _char_w + _margin;

    int window_w = WindowWidth();
    int window_h = WindowHeight();
    float tray_bot = TrayBottom();

    SDL_Rect main_area = {
        .x = 0,
        .y = tray_bot,
        .w = text_x + (_col_limit * _char_w),
        .h = window_h - tray_bot
    };

    SDL_Rect side_area = {
        .x = main_area.w,
        .y = tray_bot,
        .w = window_w - main_area.w,
        .h = window_h - tray_bot
    };

    SDL_Rect separator = {
        .x = side_area.x,
        .y = tray_bot,
        .w = 1,
        .h = window_h - tray_bot
    };

    // Background Color
    FillRect(main_area, ColorToSDL(_bg_color));

    // Side Area
    SDL_Color side_area_color = AdjustTone(_bg_color, 8);
    FillRect(side_area, side_area_color);

    // Main Area / Side Area Separator Line
    SDL_Color separator_color = AdjustTone(_bg_color, 32);
    FillRect(separator, separator_color);

    int row = 0;
    int y = tray_bot + _margin;
    Line * line;
    int line_num = top_line_num;

    for ( line = top_line;
          line != NULL && y < window_h;
          line = line->next, row++, y += _char_h + _line_spacing, line_num++ )
    {
        DrawFormat(_margin,
                   y,
                   _line_number_color,
                   "%*d", line_num_cols, top_line_num + row + 1);

        // TODO: if beyond col limit, draw char with gray background.
        DrawString(text_x, y, _fg_color, line->chars);

        if ( line_num == cy ) {
            SDL_Rect cursor_rect = {
                .x = text_x + cx * _char_w,
                .y = y,
                .w = _char_w,
                .h = _char_h
            };

            if ( cursor_blink && !TrayIsOpen() ) {
                FillRect(cursor_rect, ColorToSDL(_fg_color));
                char cursor_ch[2] = { line->chars[cx], '\0' };
                DrawString(cursor_rect.x,  cursor_rect.y, _bg_color, cursor_ch);
            }
        }
    }

    if ( TrayIsOpen() ) {
        DrawTray(cursor_blink);
    }

    UpdateWindow();
}

void Scroll(size_t new_line_num)
{
    cy = (int)new_line_num;
    top_line_num = cy - WindowHeight() / _char_h / 2;
    if ( top_line_num < 0 ) {
        top_line_num = 0;
    }
    Redraw();
}

int EventWatch(void * user_data, SDL_Event * event)
{
    if ( event->type == SDL_WINDOWEVENT
        && event->window.event == SDL_WINDOWEVENT_EXPOSED )
    {
        WindowDidResize();
        Redraw();
    }

    return 1;
}

static void RestartCursor(void)
{
    next_blink_time = SDL_GetTicks() + BLINK_MS;
    cursor_blink = true;
    needs_refresh = true;
}

static void MoveCursor(Direction direction)
{
    Line * current = GetLine(cy);

    switch ( direction ) {
        case UP:
            if ( cy > 0 ) {
                --cy;
                int screen_cursor = cy - top_line_num;
                if ( screen_cursor < 5 ) {
                    --top_line_num;
                    if ( top_line_num < 0 )
                        top_line_num = 0;
                }
            }
            break;
        case DOWN:
            if ( current->next == NULL ) {
                cx = current->len;
            } else {
                ++cy;
                int rows = WindowHeight() / _char_h;
                if ( cy - top_line_num > rows - 5 ) {
                    ++top_line_num;
                }
            }
            break;
        case LEFT:
            --cx;
            if ( cx < 0 ) {
                if ( cy > 0 ) { // go to end of prev line
                    --cy;
                    cx = current->prev->len;
                } else {
                    cx = 0; // top of document
                }
            }
            break;
        case RIGHT:
            if ( cx < current->len ) {
                ++cx;
            } else if ( current->next ) {
                // passed EOL, move down if not bottom
                ++cy;
                cx = 0;
            }
            break;
        default:
            return;
    }

    Line * line = GetLine(cy);

    if ( cx > line->len ) {
        cx = line->len; // snap to end of line
    }

    RestartCursor();
}

static void InsertNewLine(void)
{
    Line * current = GetLine(cy);
    Line * newline = NewLine();

    if ( cx == 0 ) {
        InsertLine(newline, current->prev);
    } else if ( cx == current->len ) {
        InsertLine(newline, current);
    } else { // We are mid-line.
        InsertLine(newline, current);
        char * cursor_string = current->chars + cx;
        InsertChars(newline, cursor_string, strlen(cursor_string), 0);
        RemoveChars(current, current->len - cx, cx);
    }

    cx = 0;
    cy++;
}

static void Backspace(void)
{
    if ( cx == 0 && cy == 0 ) {
        return;
    }

    Line * current = GetLine(cy);

    if ( cx == 0 ) { // Append current line onto end of previous line.
        --cy;
        cx = current->prev->len;
        InsertChars(current->prev, current->chars, strlen(current->chars), cx);
        RemoveLine(current);
    } else { // We are mid-line.
        RemoveChars(current, 1, cx - 1);
        cx--;
    }
}

static void PasteFromClipboard(void)
{
    if ( !SDL_HasClipboardText() ) {
        return;
    }

    char * clipboard_text = SDL_GetClipboardText();
    char * this = clipboard_text;

    // Handle new line characters that may be present in the pasted text.
    do {
        Line * line = GetLine(cy);
        char * next = strchr(this, '\n');

        // Calculate the length of 'this':
        size_t this_len;
        if ( next ) {
            this_len = next - this;
            next++; // Advance past the new line.
        } else {
            this_len = strlen(this);
        }

        InsertChars(line, this, this_len, cx);
        cx += this_len;

        if ( next ) {
            InsertNewLine();
        }

        this = next;
    } while ( this != NULL );

    SDL_free(clipboard_text);
}

static void DoEditorKey(SDL_Keycode key)
{
    switch ( key ) {
        case SDLK_UP:
            MoveCursor(UP);
            break;
        case SDLK_DOWN:
            MoveCursor(DOWN);
            break;
        case SDLK_RIGHT:
            MoveCursor(RIGHT);
            break;
        case SDLK_LEFT:
            MoveCursor(LEFT);
            break;
        case SDLK_RETURN:
            InsertNewLine();
            break;
        case SDLK_BACKSPACE:
            Backspace();
            break;
        case SDLK_i:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(UP);
            }
            break;
        case SDLK_j:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(LEFT);
            }
            break;
        case SDLK_k:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(DOWN);
            }
            break;
        case SDLK_l:
            if ( keys[SDL_SCANCODE_CAPSLOCK] ) {
                MoveCursor(RIGHT);
            } else if ( mods & CMD_KEY ) {
                OpenTray(TRAY_LINE_NUM);
            }
            break;
        case SDLK_s:
            if ( mods & CMD_KEY ) {
                WriteBuffer(edit_path);
            }
            break;
        case SDLK_v:
            if ( mods & CMD_KEY ) {
                PasteFromClipboard();
            }
            break;
        case SDLK_ESCAPE:
            CloseTray();
            break;
        default:
            break;
    }
}

int main(int argc, const char * argv[argc])
{
    LoadConfig(argv[1]);
    InitWindow();

    // TODO: monospace font hint

    SDL_AddEventWatch(EventWatch, NULL);
    keys = SDL_GetKeyboardState(NULL);
    SDL_StartTextInput();

    FILE * file;
    if ( argc == 2 ) {
        edit_path = argv[1];
        file = fopen(argv[1], "r");
        if ( file == NULL ) {
            AppendLine(NewLine());
        } else {
            LoadBuffer(file);
            fclose(file);
        }
    } else {
        fprintf(stderr, "Error: no file specified\n");
        fprintf(stderr, "Usage: %s [path to edit]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Redraw();

    bool quit_requested = false;
    RestartCursor();

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
                    quit_requested = true;
                    break;
                case SDL_KEYDOWN: {
                    needs_refresh = true;

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
                            InsertChars(GetLine(cy), &event.text.text[0], 1, cx);
                            cx++;
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

        if ( UpdateTray() ) {
            needs_refresh = true;
        }

        if ( needs_refresh ) {
            Redraw();
        }

        SDL_Delay(20);
    }
}
