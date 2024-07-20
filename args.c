#include "args.h"
#include <string.h>

static int     _argc = -1;
static char ** _argv;

void InitArgs(int argc, char ** argv)
{
    _argc = argc;
    _argv = argv;
}

int GetArg(const char * arg) 
{
    for ( int i = 0; i < _argc; i++ ) {
        if ( strcmp(_argv[i], arg) == 0 ) {
            return i;
        }
    }

    return 0;
}

char * GetOption(const char * long_name, const char * short_name)
{
    for ( int i = 0; i < _argc; i++ ) {
        if (   (long_name  && strcmp(_argv[i], long_name) == 0)
            || (short_name && strcmp(_argv[i], short_name) == 0))
        {
            if ( i + 1 >= _argc ) {
                return NULL;
            } else {
                return _argv[i + 1];
            }
        }
    }

    return NULL;
}
