//
//  tray.c
//  qe
//
//  Created by Thomas Foster on 6/28/24.
//

#include "tray.h"

#include "color.h"
#include "config.h"
#include "buffer.h"
#include "line.h"
#include "qe.h"
#include "window.h"
#include <errno.h>
#include <string.h>

#define TRAY_BUF_SIZE 1024

typedef enum {
    TRAY_CLOSED,
    TRAY_OPENING,
    TRAY_OPEN,
    TRAY_CLOSING
} TrayState;

static TrayState state;
static TrayType type;
static float bottom;
Line buffer;
static int cx;
static Buffer buf;

float TrayBottom(void)
{
    return bottom;
}

bool TrayIsOpen(void)
{
    return state != TRAY_CLOSED;
}

void OpenTray(TrayType typ)
{
    if ( state != TRAY_OPEN ) {
        state = TRAY_OPENING;
        type = typ;
    }
}

void CloseTray(void)
{
    if ( state != TRAY_CLOSED ) {
        state = TRAY_CLOSING;
        cx = 0;
        RemoveChars(&buffer, buffer.len, 0);
    }
}

void DoTrayKey(SDL_Keycode key)
{
    switch ( key ) {
        case SDLK_RIGHT:
            if ( cx < TRAY_BUF_SIZE - 1 ) {
                cx++;
            }
            break;
        case SDLK_LEFT:
            if ( cx > 0 ) {
                cx--;
            }
            break;
        case SDLK_RETURN:
            if ( type == TRAY_LINE_NUM ) {
                errno = 0;
                long line_num = strtol(buffer.chars, NULL, 10);
                if ( line_num == 0 && errno == EINVAL ) {
                    break;
                }

                if ( line_num > 0 && line_num <= buf.num_lines ) {
                    Scroll(line_num - 1);
                    CloseTray();
                }
            }
        break;
        case SDLK_ESCAPE:
            CloseTray();
            break;
        default:
            break;
    }
}

void DoTrayTextInput(char ch)
{
    InsertChars(&buffer, &ch, 1, cx);
    cx++;
}

bool UpdateTray(void)
{
    float tray_open_bottom = _char_h * 2 + _margin * 2;
    float lerp_factor = 0.4f;

    if ( state == TRAY_OPENING ) {
        bottom += (tray_open_bottom - bottom) * lerp_factor;
        if ( fabsf(bottom - tray_open_bottom) < 1.0f ) {
            bottom = tray_open_bottom;
            state = TRAY_OPEN;
        }
        return true;
    } else if ( state == TRAY_CLOSING ) {
        bottom -= bottom * lerp_factor;
        if ( fabsf(bottom) < 1.0f ) {
            bottom = 0.0f;
            state = TRAY_CLOSED;
        }
        return true;
    }

    return false;
}

void DrawTray(bool cursor_blink)
{
    int window_w = WindowWidth();

    // background

    SDL_Rect tray_rect = {
        .x = 0,
        .y = 0,
        .w = window_w,
        .h = bottom
    };
    FillRect(tray_rect, ColorToSDL(_bg_color));

    // separator

    SDL_Rect separator = {
        .x = 0,
        .y = bottom,
        .w = window_w,
        .h = 1
    };
    SDL_Color separator_color = AdjustTone(_bg_color, 32);
    FillRect(separator, separator_color);

    // title
    int y_offset = (_margin + _char_h) * 2;
    const char * title;
    switch ( type ) {
        case TRAY_LINE_NUM: title = "Line number"; break;
        default: break;
    }
    Color gray = ColorFromName("gray");
    DrawString(_margin, bottom - y_offset, gray, title);

    // buffer
    DrawString(_margin, bottom - (_margin + _char_h), _fg_color, buffer.chars);

    if ( cursor_blink ) {
        SDL_Rect cursor_rect = {
            .x = _margin + cx * _char_w,
            .y = bottom - (_margin + _char_h),
            .w = _char_w,
            .h = _char_h
        };
        FillRect(cursor_rect, ColorToSDL(_fg_color));
    }
}
