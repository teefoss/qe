//
//  config.c
//  qe
//
//  Created by Thomas Foster on 5/16/24.
//

#include "config.h"
#include "color.h"
#include <errno.h>
#include <sys/stat.h>

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
} Option;

static int num_options;

#ifdef __APPLE__
    #define DEFAULT_FONT "/System/Library/Fonts/Monaco.ttf"
#elifdef _WIN32
    #define DEFAULT_FONT "C:/Windows/Fonts/consola.ttf"
#elifdef __linux__
    #define DEFAULT_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#else
    #error "unsupported operating system!"
#endif


bool _use_spaces = 0;        // tabs become spaces
bool _case_sensitive = 1;    // how to apply keyword highlighting
bool _line_numbers = 0;      // show line numbers
bool _highlight_line = 1;    // highlight the current line
char _font_path[PATH_MAX] = DEFAULT_FONT;
char _theme[PATH_MAX] = "default";
int  _tab_size = 4;          // tab size in spaces
int  _font_size = 12;
int  _line_spacing = 1;      // number of pixels between lines
int  _col_limit = 80;        // column limit guide, 0 == not visible
int  _win_w = 0;
int  _win_h = 0;
Color _fg_color = 0;
Color _bg_color = 1;
Color _line_number_color = 0;

Option options[] = {
    { "USE_SPACES",         BOOLEAN,    &_use_spaces },
    { "CASE_SENSITIVE",     BOOLEAN,    &_case_sensitive },
    { "LINE_NUMBERS",       BOOLEAN,    &_line_numbers },
    { "LINE_HIGHLIGHT",     BOOLEAN,    &_highlight_line },
    { "FONT",               STRING,     _font_path },
    { "TAB_SIZE",           INTEGER,    &_tab_size },
    { "FONT_SIZE",          INTEGER,    &_font_size },
    { "LINE_SPACING",       INTEGER,    &_line_spacing },
    { "COLUMN_LIMIT",       INTEGER,    &_col_limit },
    { "WINDOW_WIDTH",       INTEGER,    &_win_w },
    { "WINDOW_HEIGHT",      INTEGER,    &_win_h },
    { "FOREGROUND_COLOR",   COLOR,      &_fg_color },
    { "BACKGROUND_COLOR",   COLOR,      &_bg_color },
    { "LINE_NUMBER_COLOR",  COLOR,      &_line_number_color },
};

static char * GetOrCreateApplicationDirectory(void)
{
    static char path[PATH_MAX] = { 0 };

    if ( path[0] == '\0' ) {
        const char * home = getenv("HOME");
        if ( home == NULL ) {
            fprintf(stderr, "could not get user home directory\n");
            exit(EXIT_FAILURE);
        }

        strcpy(path, home);
        strcat(path, "/.qe");

        errno = 0;
        if ( mkdir(path, 0755) == -1 ) {
            if ( errno != EEXIST ) {
                fprintf(stderr, "could not create directory '%s': %s\n",
                        path,
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
        } else {
            printf("created directory '%s'\n", path);
        }
    }

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

void WriteConfig(const char * path)
{
    FILE * file = fopen(path, "w");
    if ( file == NULL ) {
        fprintf(stderr, "%s error: failed to create '%s\n'", __func__, path);
        return;
    }

    for ( int i = 0; i < num_options; i++ ) {
        char name[80] = { 0 };
        strncpy(name, options[i].name, sizeof(name));
        ChangeCase(name, tolower);

        int count = fprintf(file, "%s ", name);
        while ( count++ < 20 )
            fprintf(file, " ");

        switch ( options[i].type ) {
            case INTEGER:
                fprintf(file, "%d\n", *(int *)options[i].value);
                break;
            case STRING:
                fprintf(file, "%s\n", (char *)options[i].value);
                break;
            case BOOLEAN:
                fprintf(file, "%s\n", *(bool *)options[i].value ? "yes" : "no" );
                break;
            case COLOR:
                fprintf(file, "%s\n", ColorName(*(Color *)options[i].value));
                break;
            default:
                fprintf(stderr,
                        "%s: programmed effed up (weird format for option '%s')\n",
                        __func__, options[i].name);
                exit(EXIT_FAILURE);
                break;
        }
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

static void ParseConfig(FILE * file)
{
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
        } else if ( sscanf(line, "%s %s\n", config_name, str_param) == 2 ) {
            int_param = ColorFromName(str_param);
            if ( int_param != INVALID_COLOR ) {
                type = COLOR;
            } else if ( STREQ(str_param, "yes") ) {
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

        ChangeCase(config_name, toupper); // case insensitive

        for ( int i = 0; i < num_options; i++ ) {
            if ( CaseCompare(config_name, options[i].name) == 0 ) {
                switch ( type ) {
                    case COLOR:
                        *(Color *)options[i].value = int_param;
                        break;
                    case BOOLEAN:
                        *(bool *)options[i].value = int_param;
                        break;
                    case INTEGER:
                        *(int *)options[i].value = int_param;
                        break;
                    case STRING:
                        strncpy(options[i].value, str_param, PATH_MAX);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return;
syntax_error:
    printf("syntax error on line %d:\n", line_num);
    printf("  %s", line);
}

void LoadConfig(const char * file_name)
{
    num_options = ARR_SIZE(options);

    // The the extension, i.e., the language for this file.
    // e.g. for file_name == 'main.c', load 'c.qe'
    const char * ext = rindex(file_name, '.') + 1;

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
}
