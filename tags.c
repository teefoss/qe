//
//  tags.c
//  qe
//
//  Created by Thomas Foster on 8/1/24.
//

#include "tags.h"

#include "misc.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    TAG_FUNCTION    = 'f',
    TAG_ENUMERATION = 'e',
    TAG_TYPE        = 't',
    TAG_DEFINE      = 'd',
    TAG_VARIABLE    = 'v'
} TagType;

typedef struct {
    char * name;
    char * file;
    char * signature;
    TagType type;
    char * type_info;
    bool private;
} Tag;

static Tag * tags;
static int num_tags;
static int allocated_tags;

bool GetToken(char ** field, char * string)
{
    if ( string == NULL ) {
        return false;
    }
    size_t len = strlen(string);
    *field = Allocate(len + 1);
    strcpy(*field, string);

    return true;
}

bool GetTag(char * line, Tag * tag)
{
    const char * param_error_string = NULL;

    char * token = strtok(line, "\t");
    if ( !GetToken(&tag->name, token) ) {
        param_error_string = "identifier";
        goto error;
    }

    token = strtok(NULL, "\t");
    if ( !GetToken(&tag->file, token) ) {
        param_error_string = "file";
        goto error;
    }

    token = strtok(NULL, "\t/^$");
    if ( token == NULL ) {
        param_error_string = "signature";
        goto error;
    }

    strtok(NULL, "/:\""); // Burn the signature closing delimiter

    size_t len = strlen(token);
    tag->signature = Allocate(len + 1);
    strncpy(tag->signature, token, len);
    tag->signature[len] = '\0';

    token = strtok(NULL, "\t");
    char * string;
    if ( !GetToken(&string, token) ) {
        param_error_string = "type";
        goto error;
    }

    tag->type = string[0];
    free(string);

    // Beginning of optional fields:

    token = strtok(NULL, "\t");
    if ( !GetToken(&tag->type_info, token) ) {
        return true; // No more parameters
    }

    if ( strcmp(tag->type_info, "file:") != 0 ) {
        tag->private = true;
    } else {
        // See if there's a 'file:' field after the type info.
        token = strtok(NULL, "\t\n");
        if ( strcmp(token, "file:") == 0 ) {
            tag->private = true;
        }
    }

    return true;
error:
    printf("ParseTags: Could not read %s field from line:\n'%s'\n",
           param_error_string, line);
    return false;
}

void ParseTags(void)
{
    FILE * file = fopen("tags", "r");
    if ( file == NULL ) {
        printf("Could not open tags file!\n");
        return;
    }

    num_tags = 0;

    char line[1024];
    while ( fgets(line, 1024, file) != NULL ) {
        if ( line[0] == '!' ) {
            continue;
        }

        Tag tag = { 0 };
        if ( !GetTag(line, &tag) ) {
            return;
        }

        // Resize tags array as needed.
        if ( num_tags + 1 > allocated_tags ) {
            allocated_tags = allocated_tags == 0 ? 32 : allocated_tags * 2;
            tags = Reallocate(tags, allocated_tags * sizeof(*tags));
        }

        tags[num_tags++] = tag;
    }

#if 1
    for ( int i = 0; i < num_tags; i++ ) {
        Tag tag = tags[i];
        printf("\ntag name: %s\n", tag.name);
        printf("tag file: %s\n", tag.file);
        printf("tag sig: %s\n", tag.signature);
        printf("tag type: %c\n", (char)tag.type);
        printf("tag type info: %s\n", tag.type_info);
        printf("tag scope: %s\n", tag.private ? "internal" : "global");
    }
#endif

    fclose(file);
}
