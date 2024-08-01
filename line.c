//
//  line.c
//  qe
//
//  Created by Thomas Foster on 6/28/24.
//

#include "line.h"

#include "buffer.h"
#include "config.h"
#include "misc.h"

Line * NewLine(void)
{
    Line * line = calloc(1, sizeof(*line));
    line->chars = calloc(1, sizeof(*line->chars));

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
    while ( *cx > 0 ) {
        (*cx)--;
        char this = line->chars[*cx];
        char prev = line->chars[*cx - 1];
        if ( IsIdentifierChar(this) && !IsIdentifierChar(prev) ) {
            break;
        }
    };
}

static void AddToken(Line * line, char * start, size_t len, bool is_keyword)
{
    line->num_tokens++;
    if ( line->num_tokens > line->allocated_tokens ) {
        size_t new_size = (line->allocated_tokens + 1) * sizeof(*line->tokens);
        line->tokens = realloc(line->tokens, new_size);
        line->allocated_tokens++;
    }

    line->tokens[line->num_tokens - 1] = (Token){ start, len, is_keyword };
}

void UpdateTokens(Line * line)
{
    line->num_tokens = 0;
    char * str = line->chars;
    char * end = NULL;

    // TODO: comments

    while ( *str != '\0' ) {
        if ( SDL_isalpha(*str) || *str == '_' ) { // Start of keyword or identifier.
            end = str;
            while ( *end && (SDL_isalnum(*end) || *end == '_') ) {
                end++;
            }

            size_t len = end - str;
            char * word = calloc(len + 1, sizeof(char));
            strncpy(word, str, len);
            AddToken(line, str, len, IsKeyword(word));
            free(word);
            str = end;
        } else {
            end = str;
            while ( *end && (!SDL_isalpha(*end) && *end != '_') ) {
                end++;
            }

            AddToken(line, str, end - str, _primary_color);
            str = end;
        }
    }

#if 0
    printf("line tokens:\n");
    for ( int i = 0; i < line->num_tokens; i++ ) {
        Token t = line->tokens[i];
        char * buf = calloc(t.len + 1, 1);
        strncpy(buf, &line->chars[t.start], t.len);
        printf("%s\n", buf);
        free(buf);
    }
#endif
}
