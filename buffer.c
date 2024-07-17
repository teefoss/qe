//
//  buffer.c
//  qe
//
//  Created by Thomas Foster on 5/14/24.
//

#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static Line * head;
static Line * tail;  // TODO: this may not be needed
static size_t num_lines;

size_t LineCount(void)
{
    return num_lines;
}

void InsertLineBefore(Line * line, Line * next)
{
    ASSERT(line != NULL);

    line->next = next;
    line->prev = next->prev;

    // Update surrounding nodes.
    if ( next->prev ) {
        next->prev->next = line;
    } else {
        head = line;
    }
    next->prev = line;

    num_lines++;
}

/** Insert line into buffer after line `prev`. */
void InsertLineAfter(Line * line, Line * prev)
{
    ASSERT(line != NULL);

    // Link up new node.
    line->next = prev->next;
    line->prev = prev;

    // Update surrounding nodes.
    if ( prev->next ) {
        prev->next->prev = line;
    } else {
        tail = line;
    }
    prev->next = line;

    num_lines++;
}

void RemoveLine(Line * line)
{
    if ( line->next ) {
        line->next->prev = line->prev;
    } else {
        tail = line->prev;
    }

    if ( line->prev ) {
        line->prev->next = line->next;
    } else {
        head = line->next;
    }

    free(line->chars);
    free(line);

    num_lines--;
}

void AppendLine(Line * line)
{
    ASSERT(line != NULL);

    if ( tail == NULL ) {
        head = line;
        line->prev = NULL;
    } else {
        tail->next = line;
        line->prev = tail;
    }

    tail = line;
    line->next = NULL;

    num_lines++;
}

void LoadBuffer(FILE * file)
{
    ASSERT(file != NULL);

    char * string = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ( (linelen = getline(&string, &linecap, file)) > 0 ) {
        char * end = strchr(string, '\n');
        if ( end ) {
            *end = '\0';
        }

        Line * line = NewLine();
        InsertChars(line, string, strlen(string), 0);
        AppendLine(line);
    }
}

void WriteBuffer(const char * edit_path)
{
    FILE * file = fopen(edit_path, "w");
    if ( file == NULL ) {
        perror("Could not save file");
        return;
    }

    for ( Line * line = head; line != NULL; line = line->next ) {
        fprintf(file, "%s\n", line->chars);
    }

    fclose(file);
}

Line * GetLine(int number)
{
    int i = 0;
    for ( Line * line = head; line != NULL; line = line->next, i++ ) {
        if ( i == number ) {
            return line;
        }
    }

    return NULL;
}

int LineNumCols(void)
{
    if ( num_lines == 0 ) {
        return 1;
    }

    int width = 0;
    size_t x = num_lines;
    while ( x != 0 ) {
        x /= 10;
        width++;
    }

    return width;
}
