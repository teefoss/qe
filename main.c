#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define LINELEN 80

#define ctrl(x) ((x)&0x1f)

enum colorpairs {
    CP_KEYWORD = 1,
    CP_NUMBER,
    CP_TEXT
};

#define NUM_KEYWORDS 32
const char *keywords[NUM_KEYWORDS] =
{
"auto",
"double",
"int",
"struct",
"break",
"else",
"long",
"switch",
"case",
"enum",
"register",
"typedef",
"char",
"extern",
"return",
"union",
"const",
"float",
"short",
"unsigned",
"continue",
"for",
"signed",
"void",
"default",
"goto",
"sizeof",
"volatile",
"do",
"if",
"static",
"while"
};

const char * numchars = { "1234567890xXabcdefABCDEFuUlL+-.eE" };

typedef struct
{
    char *  buffer;     // no \0, no \n!
    int     length;
    size_t  size;       // size of buffer
} line_t;



char filepath[PATH_MAX]; // the name of the currently open file

int maxy, maxx;     // the maximum x and y position
int cx, cy;         // cursor
int xoffs, yoffs;   // scroll offset

int numlines;
line_t * textbuffer;

void MoveCursorVert (int step);
void MoveCursorHoriz (int step);



void Quit (void)
{
    endwin();
    exit(0);
}

void Error (const char * message)
{
    endwin();
    perror(message);
    exit(1);
}

void LOG (const char * msg)
{
    mvprintw(0, 0, "%s\n", msg);
    printw("press any key...\n");
    refresh();
    getch();
}

bool isseparator (int c)
{
    return isspace(c) || strchr(",.()+-/*=~%<>[];&|^", c) != NULL;
}

bool isdelimiter (int c)
{
    return strchr("'\"()[]{}<>", c);
}




#pragma mark - LINE OPERATIONS

void AppendLine (char * data, int len)
{
    line_t * line;
    int i;
    
    textbuffer = realloc(textbuffer, sizeof(line_t) * (numlines + 1));
    
    line = &textbuffer[numlines];
    line->length = len;
    
    i = 1;
    while (i * LINELEN < len)
        i++;
    line->size = i * LINELEN;
    line->buffer = malloc(LINELEN * i);
    memcpy(line->buffer, data, len);
    numlines++;
}

// index: the index into textbuffer at which to insert
// (following lines are moved down one)
// data: the char data to insert, if NULL, insert blank line
void InsertLine (int index, char * data, int length)
{
    line_t * line;
    
    textbuffer = realloc(textbuffer, sizeof(line_t) * (numlines + 1));
    memmove(&textbuffer[index + 1], &textbuffer[index], sizeof(line_t) * (numlines - index));
    
    line = &textbuffer[index];
    if (data) {
        line->length = length;
        line->buffer = malloc(length);
        memcpy(line->buffer, data, length);
    } else {
        line->length = 0;
        line->size = LINELEN;
        line->buffer = malloc(LINELEN);
    }
    numlines++;
    cx = 0;
    MoveCursorVert(1);
}

void DeleteLine (int index)
{
    if (index < numlines - 1) {
        memmove(&textbuffer[index],
                &textbuffer[index + 1],
                sizeof(line_t) * numlines - cy+yoffs);
    } else {
        yoffs--;
    }
    textbuffer = realloc(textbuffer, sizeof(line_t) * numlines - 1);
    numlines--;
}

void Open (const char * path)
{
    FILE *      stream;
    char *      buf;
    ssize_t     len;
    size_t      cap;
    
    strncpy(filepath, path, PATH_MAX);
    
    stream = fopen(filepath, "r");
    if (!stream) {
        Error("Open: could not open file");
    }
    
    if (textbuffer) {
        free(textbuffer);
        textbuffer = NULL;
    }
    
    // load textbuffer
    numlines = 0;
    cap = 0;
    buf = NULL;
    while ( (len = getline(&buf, &cap, stream)) != -1 )
    {
        while (len >= 0 && (buf[len-1] == '\n' ||
                           buf[len-1] == '\r' ||
                           buf[len-1] == ' '))
        {
            len--; // strip \n, \r, and space chars
        }
        AppendLine(buf, (int)len);
    }
    free(buf);
    fclose(stream);
}

void Close (void)
{
    
}

void HL_Number (int * x, int y)
{
    char * c;
    line_t * l;

    attron(COLOR_PAIR(CP_NUMBER));
    
    l = &textbuffer[y+yoffs];
    do
    {
        c = &l->buffer[*x];
        if (!strchr(numchars, *c)) {
            break;
        }
        mvaddch(y, *x, *c);
        if (++(*x) == l->length) {
            attroff(COLOR_PAIR(CP_NUMBER));
            return;
        }
    } while (1);
    attroff(COLOR_PAIR(CP_NUMBER));
    mvaddch(y, *x, *c);
}

void HL_AltNumber ()
{
    
}


void Display (void)
{
    int y, x;
    int startx;
    char * c;
    char * prev = NULL;
    line_t * l;
    enum { st_none, st_inword, st_inchar, st_innum } state;

    state = st_none;
    curs_set(0);
    clear();
    for (y = 0, l = &textbuffer[y+yoffs]; y <= maxy; y++, l++)
    {
        startx = -1;
        for (x = 0, c = &l->buffer[x] ; x < l->length; x++, c++)
        {
            bool sep = prev && isseparator(*prev);
            // number HL
#if 1
            if ((isdigit(*c) || (sep && *c == '.')) && state != st_innum) {
                state = st_innum;
                startx = x;
            }
            if (state == st_innum && (!strchr(numchars, *c)||x==l->length-1)) {
                state = st_none;
                attron(COLOR_PAIR(CP_NUMBER));
                mvaddnstr(y, startx, &l->buffer[startx], x - startx);
                attroff(COLOR_PAIR(CP_NUMBER));
            }
            if (state != st_inword) {
                if ( isalpha(*c) && (sep || x == 0) ) {
                    state = st_inword;
                    startx = x;
                }
            }
#endif
            // keyword HL
#if 1
            else if (state == st_inword && (isseparator(*c)||x==l->length-1)) { // check if done
                state = st_none;
                if (startx == -1)  Error("wordx not inited???");
                for (int i=0; i<NUM_KEYWORDS; i++)
                {
                    int checklen = isseparator(*c) ? x - startx : x - startx + 1;
                    int keylen = (int)strlen(keywords[i]);
                    if (checklen == keylen && !strncmp(keywords[i], &l->buffer[startx], keylen))
                    {
                        attron(COLOR_PAIR(CP_KEYWORD));
                        mvaddnstr(y, startx, keywords[i], x-startx);
                        attroff(COLOR_PAIR(CP_KEYWORD));
                    }
                }
            }
#endif
            mvaddch(y, x, *c);
            
            prev = c;
        }
        state = st_none;
        prev = NULL;
    }
    refresh();
    curs_set(1);
}

void ScrollScreen (int step)
{
    if (yoffs + step < 0)
        yoffs = 0;
    else if (yoffs + step >= numlines - maxy)
        yoffs = numlines - maxy - 1;
    else
        yoffs += step;
}


void MoveCursorVert (int step)
{
    if (cy + step < 0 || cy + step > maxy) {
        ScrollScreen(step);
    } else {
        cy += step;
    }
    if (cx > textbuffer[cy+yoffs].length)
        cx = textbuffer[cy+yoffs].length;
}

void MoveCursorHoriz (int step)
{
    if (cx + step < 0) {
        MoveCursorVert(-1);
        cx = textbuffer[cy+yoffs].length;
    } else if (cx + step > textbuffer[cy+yoffs].length) {
        MoveCursorVert(1);
        cx = 0;
    } else {
        cx += step;
    }
}


void InsertChar (int c)
{
    line_t * line;

    line = &textbuffer[cy+yoffs];
    
    // make room if needed
    if (line->length + 1 > LINELEN) {
        line->size += LINELEN;
        line->buffer = realloc(line->buffer, line->size);
    }
    
    if (cx == line->length) { // just append
        line->buffer[cx] = c;
    } else { // insert
        memmove(&line->buffer[cx+1], &line->buffer[cx], line->length-cx);
        line->buffer[cx] = c;
    }
    line->length++;
    cx++;
}

void DeleteChar ()
{
    if (cx == 0 && cy+yoffs > 0)
    {
        
    }
}


int main (int argc, const char * argv[])
{
    int c;
    
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    if (!has_colors())
        Error("Your terminal does not support color! Please join us in the 21st Century.");
    start_color();
    use_default_colors();
    init_pair(CP_KEYWORD, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CP_NUMBER, COLOR_BLUE, COLOR_BLACK);
    init_pair(CP_TEXT, COLOR_WHITE, COLOR_BLACK);

    // init editor
    getmaxyx(stdscr, maxy, maxx);
    maxy--;
    maxx--;
    numlines = 0;

    if (argc > 1) {
        Open(argv[1]);
    }
    
    while (1)
    {
        Display();
        
        move(cy, cx); // update cursor
        
        c = getch();
        switch (c)
        {
            case ctrl('q'):
                Quit();
                break;
                
            case ctrl(KEY_BACKSPACE):
                DeleteLine(cy+yoffs);
                break;
                
            case KEY_UP:
                MoveCursorVert(-1);
                break;
                
            case KEY_DOWN:
                MoveCursorVert(1);
                break;
                
            case KEY_LEFT:
                MoveCursorHoriz(-1);
                break;
                
            case KEY_RIGHT:
                MoveCursorHoriz(1);
                break;
                
            case KEY_RESIZE:
                getmaxyx(stdscr, maxy, maxx);
                maxy--;
                maxx--;
                break;
                
            case 10:
            case KEY_ENTER:
                if (cx == textbuffer[cy+yoffs].length)
                    InsertLine(cy+yoffs+1, NULL, 0);
                else {
                    int x = cx;
                    line_t *line = &textbuffer[cy+yoffs];
                    InsertLine(cy+yoffs+1, &line->buffer[x], line->length-x);
                    line->length = x;
                }
                break;
            case KEY_BACKSPACE:
                DeleteChar();
                break;
                

            default:
                if (c >= 32 && c < 127) {
                    InsertChar(c);
                }
                break;
        }
    }
}
