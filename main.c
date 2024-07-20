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
    puts("  --config [path]      Load config file specified by 'path'.");
}

static void Error(const char * message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    printf("\n");
    PrintUsage();
    exit(EXIT_FAILURE);
}

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

    char * number_arg = GetOption("--line", "-l");
    if ( number_arg ) {
        int line_number = atoi(number_arg);
        // TODO: Scroll()
    }

    const char * document_path = argv[argc - 1];

    FILE * file = fopen(document_path, "r");
    if ( file == NULL ) {
        if ( create ) {
            file = fopen(document_path, "w");
            if ( file == NULL ) {
                printf(PROGRAM_NAME": failed to create '%s'", document_path);
                return EXIT_FAILURE;
            }
            AppendLine(NewLine());
        } else {
            printf(PROGRAM_NAME": Could not open '%s'. Does it exist?\n",
                   document_path);
            return EXIT_FAILURE;
        }
    } else {
        LoadBuffer(file);
    }

    fclose(file);
    return Edit(document_path);
}
