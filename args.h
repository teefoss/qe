#ifndef args_h
#define args_h

#include <stdbool.h>

/**  Must be called before using other arg functions. */
void InitArgs(int argc, char ** argv);

/** Returns the index of `arg` or `0` if not present. */
int GetArg(const char * arg);

/** 
 *  Returns the argument after `long_name` or `short_name` or `NULL` if not
 *  present.
 *  - parameter long_name: The long version of the option, e.g. `"--version"`
 *      or `NULL` if not needed.
 *  - parameter short_name: The short version of the option, e.g. `"-v"`.
 *      or `NULL` if not needed.
 */
char * GetOption(const char * long_name, const char * short_name);

#endif /* args_h */
