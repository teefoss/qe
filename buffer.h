//
//  buffer.h
//  qe
//
//  Created by Thomas Foster on 7/2/24.
//

#ifndef buffer_h
#define buffer_h

#include "line.h"

size_t  LineCount(void);
void LoadBuffer(FILE * file);
void WriteBuffer(const char * edit_path);
Line * GetLine(int number);
void InsertLine(Line * line, Line * prev);
void AppendLine(Line * line);
void RemoveLine(Line * line);
int LineNumCols(void);

#endif /* buffer_h */
