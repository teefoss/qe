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

void InsertChars(Line * line, char * string, int len, int x)
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
        DieGracefully("Error: could not insert text (out of memory)\n");
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

void RemoveChars(Line * line, int count, int x)
{
    memmove(line->chars + x,
            line->chars + x + count,
            (line->len + 1) - (x + count));
    line->len -= count;
}

static bool IsIdentifierChar(char ch)
{
    return SDL_isalnum(ch) || ch == '_';
}

/**
 *  Move cursor (`cx`) to the position after the current or next alpha-numeric
 *  word.
 */
void JumpToEndOfWord(Line * line, int * cx)
{
    bool in_identifier = false; // Cursor is currently inside an identifier.
    bool is_identifier_char = false; // Character at cx is an identifier character.

    do {
        if ( *cx == line->len ) {
            break;
        }

        (*cx)++;
        is_identifier_char = IsIdentifierChar(line->chars[*cx]);

        if ( is_identifier_char ) {
            in_identifier = true;
        }
    } while ( !in_identifier || is_identifier_char );
}

void JumpToBeginningOfWord(Line * line, int * cx)
{
    bool in_identifier = false; // Cursor is currently inside an identifier.
    bool next_is_identifier_char = false;

    while ( *cx > 0 ) {
        (*cx)--;
        char this = line->chars[*cx];
        char prev = line->chars[*cx - 1];
        if ( IsIdentifierChar(this) && !IsIdentifierChar(prev) ) {
            break;
        }
    };
}
