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

void FreeBuffer(Buffer * buffer)
{
    Line * line = buffer->head;
    while ( line ) {
        Line * temp = line;
        line = line->next;
        free(temp->chars);
        free(temp);
    }
}

/**
 *  Insert `line` into `buffer` before `next`.
 */
void InsertLineBefore(Buffer * buffer, Line * line, Line * next)
{
    ASSERT(line != NULL);

    line->next = next;
    line->prev = next->prev;

    // Update surrounding nodes.
    if ( next->prev ) {
        next->prev->next = line;
    } else {
        buffer->head = line;
    }
    next->prev = line;

    buffer->num_lines++;
}

/** 
 *  Insert `line` into `buffer` after `prev`.
 */
void InsertLineAfter(Buffer * buffer, Line * line, Line * prev)
{
    ASSERT(line != NULL);

    // Link up new node.
    line->next = prev->next;
    line->prev = prev;

    // Update surrounding nodes.
    if ( prev->next ) {
        prev->next->prev = line;
    } else {
        buffer->tail = line;
    }
    prev->next = line;

    buffer->num_lines++;
}

void RemoveLine(Buffer * buffer, Line * line)
{
    if ( line->next ) {
        line->next->prev = line->prev;
    } else {
        buffer->tail = line->prev;
    }

    if ( line->prev ) {
        line->prev->next = line->next;
    } else {
        buffer->head = line->next;
    }

    free(line->chars);
    free(line);

    buffer->num_lines--;
}

void AppendLine(Buffer * buffer, Line * line)
{
    ASSERT(line != NULL);

    if ( buffer->tail == NULL ) {
        buffer->head = line;
        line->prev = NULL;
    } else {
        buffer->tail->next = line;
        line->prev = buffer->tail;
    }

    buffer->tail = line;
    line->next = NULL;

    buffer->num_lines++;
}

void LoadBuffer(Buffer * buffer, FILE * file)
{
    ASSERT(file != NULL);

    char string[2048];
    while ( fgets(string, 2048, file) != NULL ) {
        char * end = strchr(string, '\n');
        if ( end ) {
            *end = '\0';
        }

        Line * line = NewLine();
        InsertChars(line, string, (int)strlen(string), 0);
        AppendLine(buffer, line);
    }
}

void WriteBuffer(Buffer * buffer, const char * path)
{
    FILE * file = fopen(path, "w");
    if ( file == NULL ) {
        perror("Could not save file");
        return;
    }

    for ( Line * line = buffer->head; line != NULL; line = line->next ) {
        fprintf(file, "%s\n", line->chars);
    }

    fclose(file);
}

Line * GetLine(Buffer * buffer, int number)
{
    int i = 0;
    for ( Line * line = buffer->head; line != NULL; line = line->next, i++ ) {
        if ( i == number ) {
            return line;
        }
    }

    return NULL;
}

// TODO: remove
int LineNumCols(Buffer * buffer)
{
    if ( buffer->num_lines == 0 ) {
        return 1;
    }

    int width = 0;
    size_t x = buffer->num_lines;
    while ( x != 0 ) {
        x /= 10;
        width++;
    }

    return width;
}
