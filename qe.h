//
//  qe.h
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#ifndef qe_h
#define qe_h

#include <stdint.h>
#include <stdio.h>
//#include <stdbool.h>
#include <SDL2/SDL.h>

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
#define EXTRACT_FIELD(value, mask)  (((value) & (mask)) >> __builtin_ctz(mask))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

int CaseCompare(const char * a, const char * b);
void DieGracefully(const char * message, ...);
void Scroll(size_t new_line_num);
int Edit(const char * path);

#endif /* qe_h */
