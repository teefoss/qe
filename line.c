//
//  line.c
//  qe
//
//  Created by Thomas Foster on 6/28/24.
//

#include "line.h"
#include "buffer.h"

Line * NewLine(void)
{
    Line * line = calloc(1, sizeof(*line));
    line->chars = calloc(1, sizeof(*line->chars));
    line->len = 0;

    return line;
}

void InsertChars(Line * line, char * string, size_t len, int x)
{
    if ( len == 0 ) {
        return;
    }

    // Calculate the later part of the string that will need to be shifted right.
    size_t amt_to_shift = line->len - x;
    line->len += len;

    // Reallocate.
    char * temp = realloc(line->chars, line->len + 1);
    if ( temp == NULL ) {
        DieGracefully();
    }
    line->chars = temp;
    line->chars[line->len] = '\0';

    // Move everything down.
    char * insert_point = line->chars + x;
    memmove(line->chars + x + len,
            insert_point,
            amt_to_shift);

    // Copy in the string at x.
    strncpy(insert_point, string, len);
}

void RemoveChars(Line * line, size_t count, int x)
{
    memmove(line->chars + x,
            line->chars + x + count,
            (line->len + 1) - (x + count));
    line->len -= count;
}
