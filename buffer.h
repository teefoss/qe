//
//  buffer.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef buffer_h
#define buffer_h

#include "line.h"

typedef struct {
    Line * head;
    Line * tail;  // TODO: this may not be needed
    size_t num_lines;
} Buffer;

void    LoadBuffer(Buffer *, FILE * file);
void    FreeBuffer(Buffer * buffer);
void    WriteBuffer(Buffer *, const char * path);
Line *  GetLine(Buffer *, int number);
void    InsertLineBefore(Buffer *, Line * line, Line * next);
void    InsertLineAfter(Buffer *, Line * line, Line * prev);
void    AppendLine(Buffer *, Line * line);
void    RemoveLine(Buffer *, Line * line);
int     LineNumCols(Buffer *);

#endif /* buffer_h */
