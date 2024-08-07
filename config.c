//
//  config.c
//  qe
//
//  Created by Thomas Foster on 5/16/24.
//

#include "config.h"
#include "plat.h"
#include "misc.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define R_MASK 0xFF0000
#define G_MASK 0x00FF00
#define B_MASK 0x0000FF

typedef enum {
    NO_OPT = -1,
    USE_SPACES,
    CASE_SENSITIVE,
    LINE_NUMBERS,
    LINE_HIGHLIGHT,
    FONT,
    TAB_SIZE,
    FONT_SIZE,
    LINE_SPACING,
    COLUMN_LIMIT,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    PRIMARY_COLOR,
    SECONDARY_COLOR,
    KEYWORD_COLOR,
    BACKGROUND_COLOR,

    NUM_OPTIONS
} OptionID;

typedef enum {
    INTEGER,
    STRING,
    BOOLEAN,
    COLOR,
    CONFIG,
} OptionType;

typedef struct {
    const char * name;
    OptionType type;
    void * value;
} OptionMapping;

typedef struct {
    OptionID id;
    union {
        bool bool_value;
        int int_value;
        char * string_value;
        SDL_Color color_value;
    };
} Option;

typedef struct {
    Option options[NUM_OPTIONS]; // NO_OPT-terminated list
    const char * keywords[MAX_KEYWORDS]; // NULL-terminated list
} Options;

#if defined(__APPLE__)
    #define DEFAULT_FONT "/System/Library/Fonts/Monaco.ttf"
#elif defined(PLATFORM_WINDOWS)
    #define DEFAULT_FONT "C:/Windows/Fonts/consola.ttf"
#elif defined (__linux__)
    #define DEFAULT_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#else
    #error "unsupported operating system!"
#endif

const Options global_options = {
    .options = {
        { .id = USE_SPACES, .bool_value = false },
        { .id = CASE_SENSITIVE, .bool_value = false },
        { .id = LINE_NUMBERS, .bool_value = false },
        { .id = LINE_HIGHLIGHT, .bool_value = false },
        { .id = FONT, .string_value = DEFAULT_FONT },
        { .id = TAB_SIZE, .int_value = 4 },
        { .id = FONT_SIZE, .int_value = 12 },
        { .id = LINE_SPACING, .int_value = 0 },
        { .id = COLUMN_LIMIT, .int_value = 0 },
        { .id = WINDOW_WIDTH, .int_value = 640 },
        { .id = WINDOW_HEIGHT, .int_value = 480 },
        { .id = PRIMARY_COLOR, .color_value = { 0x00, 0x00, 0x00 } },
        { .id = SECONDARY_COLOR, .color_value = { 0x08, 0x80, 0xFE } },
        { .id = KEYWORD_COLOR, .color_value = { 0xFE, 0x08, 0x08 } },
        { .id = BACKGROUND_COLOR, .color_value = { 0xFF, 0xFF, 0xFF } },
    },
    .keywords = { NULL }
};

Options c_options = {
    .options = {
        { .id = CASE_SENSITIVE, .bool_value = true },
        { .id = NO_OPT },
    },
    .keywords = {
        "auto", "double", "int", "struct", 
        "break", "else", "long", "switch",
        "case", "enum", "register", "typedef", 
        "char", "extern", "return", "union",
        "const", "float", "short", "unsigned", 
        "continue", "for", "signed", "void",
        "default", "goto", "sizeof", "volatile", 
        "do", "if", "static", "while", 
        NULL
    }
};

bool _use_spaces = 0;
bool _case_sensitive = 1;
bool _line_numbers = 0;
bool _highlight_line = 1;
char _font_path[PATH_MAX] = DEFAULT_FONT;
int _tab_size = 4;
int _font_size = 12;
int _line_spacing = 0;
int _col_limit = 80;
int _win_w = 0;
int _win_h = 0;
SDL_Color _primary_color = { 0x00, 0x00, 0x00 };
SDL_Color _secondary_color = { 0x08, 0x80, 0xFE };
SDL_Color _keyword_color = { 0xFE, 0x08, 0x08 };
SDL_Color _bg_color = { 0xFF, 0xFF, 0xFF };

// Keywords that were loaded from config
int num_keywords;
char keywords[MAX_KEYWORDS][64];

//
// Specify options' types and map them to their global variable.
//
#define ENTRY(e, type, loc) [e] = { #e, type, loc }
static OptionMapping opt_map[] = {
    ENTRY( USE_SPACES,          BOOLEAN,    &_use_spaces ),
    ENTRY( CASE_SENSITIVE,      BOOLEAN,    &_case_sensitive ),
    ENTRY( LINE_NUMBERS,        BOOLEAN,    &_line_numbers ),
    ENTRY( LINE_HIGHLIGHT,      BOOLEAN,    &_highlight_line ),
    ENTRY( FONT,                STRING,     _font_path ),
    ENTRY( TAB_SIZE,            INTEGER,    &_tab_size ),
    ENTRY( FONT_SIZE,           INTEGER,    &_font_size ),
    ENTRY( LINE_SPACING,        INTEGER,    &_line_spacing ),
    ENTRY( COLUMN_LIMIT,        INTEGER,    &_col_limit ),
    ENTRY( WINDOW_WIDTH,        INTEGER,    &_win_w ),
    ENTRY( WINDOW_HEIGHT,       INTEGER,    &_win_h ),
    ENTRY( PRIMARY_COLOR,       COLOR,      &_primary_color ),
    ENTRY( SECONDARY_COLOR,     COLOR,      &_secondary_color ),
    ENTRY( KEYWORD_COLOR,       COLOR,      &_keyword_color ),
    ENTRY( BACKGROUND_COLOR,    COLOR,      &_bg_color ),
};
#undef ENTRY

bool IsKeyword(const char * word)
{
    for ( int i = 0; i < num_keywords; i++ ) {
        if ( (_case_sensitive && SDL_strcmp(word, keywords[i]) == 0)
            || (!_case_sensitive && SDL_strcasecmp(word, keywords[i]))) {
            return true;
        }
    }

    return false;
}

#if 0
static char * GetOrCreateApplicationDirectory(void)
{
    if ( !ApplicationDirectoryExists() ) {
        CreateApplicationDirectory();
    }

    return ApplicationDirectory();
}
#endif

static char * ConfigFilePath(const char * file_name)
{
    ASSERT(ApplicationDirectoryExists());

    char * app_dir = ApplicationDirectory();
    static char path[PATH_MAX] = { 0 };
    strcpy(path, app_dir);
    strcat(path, "/");
    strcat(path, file_name);
    strcat(path, ".qe");

    return path;
}

static void ChangeCase(char * string, int (* to_case)(int))
{
    char * c = string;
    while ( *c ) {
        *c = to_case(*c);
        c++;
    }
}

void WriteConfig(const Options * options, const char * path)
{
    FILE * file = fopen(path, "w");
    if ( file == NULL ) {
        fprintf(stderr, "%s error: failed to create '%s\n'", __func__, path);
        return;
    }

    for ( int i = 0; i < NUM_OPTIONS; i++ ) {
        const Option * opt = &options->options[i];

        if ( opt->id == NO_OPT ) {
            break; // end of list
        }

        OptionMapping mapping = opt_map[opt->id];

        char name[80] = { 0 };
        strncpy(name, mapping.name, sizeof(name));
        ChangeCase(name, tolower);

        int count = fprintf(file, "%s ", name);
        while ( count++ < 20 )
            fprintf(file, " ");

        switch ( mapping.type ) {
            case INTEGER:
                fprintf(file, "%d\n", opt->int_value);
                break;
            case STRING:
                fprintf(file, "%s\n", opt->string_value);
                break;
            case BOOLEAN:
                fprintf(file, "%s\n", opt->bool_value ? "yes" : "no" );
                break;
            case COLOR: {
                int hex = 0;
                hex |= opt->color_value.r << 16;
                hex |= opt->color_value.g << 8;
                hex |= opt->color_value.b;
                fprintf(file, "#%06X\n", hex);
                break;
            }
            default:
                fprintf(stderr,
                        "%s: programmer effed up (weird format for option '%s')\n",
                        __func__, name);
                exit(EXIT_FAILURE);
                break;
        }
    }

    fprintf(file, "\n");
    for ( int i = 0; i < MAX_KEYWORDS; i++ ) {
        if ( options->keywords[i] == NULL ) {
            break;
        }

        fprintf(file, "keyword %s\n", options->keywords[i]);
    }

    fclose(file);
}

static void StripComments(char * line)
{
    char * c = line;

    while ( *c ) {
        if ( *c == ';' ) {
            *c = '\0';
            return;
        }
        c++;
    }
}

static bool IsEmptyLine(const char * line)
{
    const char * c = line;

    while ( *c ) {
        if ( !isspace(*c) )
            return false;
        c++;
    }

    return true;
}

typedef enum { ACCESS_READ, ACCESS_WRITE } Access;

FILE * OpenConfig(const char * file_name, Access access)
{
    char * path = ConfigFilePath(file_name);

    // TODO: OpenRead, OpenWrite, and FileExists functions -> misc.c
    FILE * file = fopen(path, access == ACCESS_READ ? "r" : "w" );
    if ( file == NULL && access == ACCESS_WRITE ) {
        DieGracefully("Could not create %s: %s", path, strerror(errno));
    }

    return file;
}

static void ParseConfig(const char * file_name)
{
    FILE * file = OpenConfig(file_name, ACCESS_READ);

    if ( file == NULL ) {
        printf("Config file '%s.qe' not found\n", file_name);
        return;
    }

    char line[512] = { 0 };
    int line_num = 0;

    while ( fgets(line, sizeof(line) - 1, file) != NULL ) {
        ++line_num;
        StripComments(line);

        char config_name[80];
        char str_param[80];
        int  int_param;
        OptionType type;

        if ( sscanf(line, "%s %i\n", config_name, &int_param) == 2 ) {
            type = INTEGER;
        } else if ( sscanf(line, "%s #%X\n", config_name, &int_param) == 2 ) {
            type = COLOR;
        } else if ( sscanf(line, "%s %s\n", config_name, str_param) == 2 ) {
            if ( STREQ(str_param, "yes") ) {
                type = BOOLEAN;
                int_param = 1;
            } else if ( STREQ(str_param, "no" ) ) {
                type = BOOLEAN;
                int_param = 0;
            } else {
                type = STRING;
            }
        } else {
            if ( IsEmptyLine(line) )
                continue;
            goto syntax_error;
        }

        if ( type == STRING && SDL_strcasecmp(config_name, "KEYWORD") == 0 ) {
            size_t len = strlen(str_param);
            if ( len > 79 ) {
                len = 79;
            }
            memcpy(&keywords[num_keywords++][0], str_param, len);
            continue;
        }

        for ( int i = 0; i < NUM_OPTIONS; i++ ) {
            if ( SDL_strcasecmp(config_name, opt_map[i].name) == 0 ) {
                switch ( type ) {
                    case COLOR: {
                        SDL_Color color;
                        color.r = (int_param & 0xFF0000) >> 16;
                        color.g = (int_param & 0x00FF00) >> 8;
                        color.b = (int_param & 0x0000FF);
                        *(SDL_Color *)opt_map[i].value = color;
                        break;
                    }
                    case BOOLEAN:
                        *(bool *)opt_map[i].value = int_param;
                        break;
                    case INTEGER:
                        *(int *)opt_map[i].value = int_param;
                        break;
                    case STRING:
                        strncpy(opt_map[i].value, str_param, PATH_MAX);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    fclose(file);

    return;
syntax_error:
    fclose(file);
    printf("syntax error on line %d:\n", line_num);
    printf("  %s", line);
}

void CreateConfigFile(const char * file_name, const Options * options)
{
    FILE * file = OpenConfig(file_name, ACCESS_WRITE); // Create the file.
    WriteConfig(options, ConfigFilePath(file_name));
    fclose(file);
}

void LoadConfig(const char * file_name)
{
    if ( !ApplicationDirectoryExists() ) {
        // Need to 'install' qe:
        CreateApplicationDirectory();
        CreateConfigFile("global", &global_options);
        CreateConfigFile("c", &c_options);
    }

    num_keywords = 0;

    ParseConfig("global");

    // Get the extension, i.e., the language for this file.
    // e.g. for file_name == 'main.c', load 'c.qe'
    const char * ext = strrchr(file_name, '.') + 1;
    ParseConfig(ext);

#if 0
    char * app_dir = GetOrCreateApplicationDirectory();
    char path[PATH_MAX] = { 0 };
    snprintf(path, sizeof(path) - 1, "%s/%s.qe", app_dir, ext);
    FILE * config_file = fopen(path, "r");

    if ( ext == NULL || config_file == NULL ) { // Load default config.
        snprintf(path, sizeof(path) - 1, "%s/default.qe", app_dir);
        config_file = fopen(path, "r");
        if ( config_file == NULL ) { // Default config file doesn't exist.
            WriteConfig(path); // Create it.
            return;
        }
    }

    ParseConfig(config_file);
    fclose(config_file);
    printf("loaded configuration in %s\n", path);
#endif
}
