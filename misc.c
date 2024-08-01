//
//  misc.c
//  qe
//
//  Created by Thomas Foster on 8/1/24.
//

#include "misc.h"
#include "qe.h"

void DieGracefully(const char * message, ...)
{
    // TODO: the gracefully part

    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void * CheckMemory(void * memory)
{
    if ( memory == NULL ) {
        DieGracefully("Out of memory!");
    }

    return memory;
}

void * Allocate(size_t bytes)
{
    return CheckMemory(calloc(bytes, 1));
}

void * Reallocate(void * ptr, size_t bytes)
{
    return CheckMemory(realloc(ptr, bytes));
}
