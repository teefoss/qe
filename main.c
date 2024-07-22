//
//  main.c
//  qe
//
//  Created by Thomas Foster on 7/17/24.
//

#include "buffer.h"
#include "config.h"
#include "window.h"
#include "args.h"

#include <stdlib.h>
#include <stdarg.h>
#include <SDL_ttf.h>

#define PROGRAM_NAME "qe"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

static void PrintUsage(void)
{
    puts("Usage: " PROGRAM_NAME " (options) [file]");
    puts("Options:");
    puts("  -l, --line <number>  Select line <number> after opening file.");
    puts("  -c, --create         Create the file if it does not exist.");
    puts("  -v, --version        Display version information.");
    puts("  -h, --help           Show this information.");
    puts("  --config [path]      Load config file at 'path'.");
}

#if 0 // TODO: Might need this elsewhere.
static void Error(const char * message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    printf("\n");
    PrintUsage();
    exit(EXIT_FAILURE);
}
#endif

int main(int argc, char ** argv)
{
    InitArgs(argc, argv);

    if ( argc == 1 ) {
        PrintUsage();
        return EXIT_FAILURE;
    }

    if ( GetOption("--help", "-h") ) {
        PrintUsage();
        return EXIT_SUCCESS;
    }

    if ( GetOption("--version", "-v") ) {
        printf("qe Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
        printf("Copyright (C) Thomas Foster 2024\n");
        return EXIT_SUCCESS;
    }

    char * config_file = GetOption("--config", NULL);
    if ( config_file ) {
        printf("%s\n", config_file); // TODO: load
    }

    bool create = GetOption("--create", "-c");

    int line_number = 0;
    char * line_number_arg = GetOption("--line", "-l");
    if ( line_number_arg ) {
        line_number = atoi(line_number_arg) - 1;
    }

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if ( TTF_Init() != 0 ) {
        fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    InitWindow();
    LoadDocument(argv[argc - 1], create, line_number);

    return ProgramLoop();
}
