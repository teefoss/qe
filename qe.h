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

#ifdef __APPLE__
#   define CMD_KEY (KMOD_GUI | KMOD_CTRL) // TODO: test
#else
#   define CMD_KEY KMOD_CTRL
#endif

#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define STREQ(a, b) (strcmp(a, b) == 0)
#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define STRSZ 79
typedef unsigned char String[STRSZ + 1];
typedef String StringArray[STR_ARR_MAX];

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

int CaseCompare(const char * a, const char * b);
void DieGracefully(void);
void Scroll(size_t new_line_num);

#endif /* qe_h */
