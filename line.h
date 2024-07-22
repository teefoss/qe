//
//  line.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef line_h
#define line_h

#include "qe.h"
#include "color.h"

typedef struct {
    size_t start;
    size_t len;
    Color color;
} Token;

typedef struct line {
    char * chars; // allocated size is always length + 1
    Color * char_colors;
    int len;
    struct line * prev;
    struct line * next;
} Line;

Line * NewLine(void);
void InsertChars(Line * line, char * string, int len, int x);
void RemoveChars(Line * line, int count, int x);
void JumpToEndOfWord(Line * line, int * cx);
void JumpToBeginningOfWord(Line * line, int * cx);

// TODO: DrawLine(const Line * line, int line_num, bool highlighted)

#endif /* line_h */
