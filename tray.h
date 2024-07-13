//
//  tray.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef tray_h
#define tray_h

#include <SDL2/SDL.h>

typedef enum {
    TRAY_LINE_NUM,
} TrayType;

float TrayBottom(void);
bool TrayIsOpen(void);
void OpenTray(TrayType typ);
void CloseTray(void);
void DoTrayKey(SDL_Keycode key);
void DoTrayTextInput(char ch);
bool UpdateTray(void);
void DrawTray(bool cursor_blink);

#endif /* tray_h */
