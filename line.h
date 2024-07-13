//
//  line.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef line_h
#define line_h

#include "qe.h"

typedef struct line {
    char * chars; // allocated size is always length + 1
    u16 len;
    struct line * prev;
    struct line * next;
} Line;

Line * NewLine(void);
void InsertChars(Line * line, char * string, size_t len, int x);
void RemoveChars(Line * line, size_t count, int x);

#endif /* line_h */
