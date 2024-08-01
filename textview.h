//
//  textview.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef textview_h
#define textview_h

#include "buffer.h"

// TODO: move to action.c/h BufferActionType -> ActionType
typedef enum {
    BUF_ACTION_NONE,
    BUF_MOVE_X,
    BUF_MOVE_Y,
    BUF_MOVE_UP,
    BUF_MOVE_DOWN,
    BUF_MOVE_LEFT,
    BUF_MOVE_RIGHT,
    BUF_PAGE_UP,
    BUF_PAGE_DOWN,
    BUF_MOVE_TO_TOP,
    BUF_MOVE_TO_BOTTOM,
    BUF_INSERT_LINE,
    BUF_BACKSPACE,

    NUM_BUF_ACTIONS,
} BufferActionType;

typedef struct {
    Buffer buffer;

    // Cursor position.
    int cx;
    int cy;

    // Pixel draw offset.
    // A y offset of 0 means the top of line 0 is at the top of the window.
    float offset_y;

    bool actions[NUM_BUF_ACTIONS]; // The actions this text view responds to.
    char * file_path;
} TextView;

typedef struct {
    SDL_Keycode key;
    SDL_Keymod mods;
} Input;

typedef struct {
    Input inputs[8];
    void (* func)(TextView *);
    int param;
} Action;

bool UpdateBufferOffsets(TextView * buf, float buf_height_pixels);
bool BufferRespond(TextView * buf, SDL_Keycode key, SDL_Keymod mods);
void DrawTextView(TextView * buf, SDL_Rect rect, bool show_cursor);

#endif /* textview_h */
