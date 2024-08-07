//
//  qe.h
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#ifndef qe_h
#define qe_h

#include "plat.h"

#include <stdint.h>
#include <stdio.h>
//#include <stdbool.h>
#include <SDL.h>

#define STR_ARR_MAX 64

#define QE_DEBUG
#ifdef  QE_DEBUG
    #include <assert.h>
    #define ASSERT(x) assert(x)
#else
    #define ASSERT(x)
#endif

#ifdef _WIN32
    #define PATH_SEP '\\'
#else
    #define PATH_SEP '/'
#endif

#ifdef __APPLE__
    #define CMD_KEY (KMOD_GUI | KMOD_CTRL)
#else
    #define CMD_KEY KMOD_CTRL
#endif

#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define STREQ(a, b) (strcmp(a, b) == 0)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(val, min, max) (val < min ? min : val > max : max : val)
#define ROUND(fl) (int)((fl) + 0.5f)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

void LoadDocument(const char * path, bool create, int line_number);
int ProgramLoop(void);
void RestartCursor(void);

#endif /* qe_h */
