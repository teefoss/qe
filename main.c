//
//  main.c
//  qe
//
//  Created by Thomas Foster on 7/17/24.
//

#include "buffer.h"
#include "config.h"
#include "window.h"
//#include <unistd.h>
#include <getopt.h>

#define PROGRAM_NAME "qe"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

static void PrintUsage(void)
{
    puts("Usage: " PROGRAM_NAME " (options) [file]");
    puts("Options:");
    puts("    -c, --create       Create the file if it does not exist.");
    puts("    -v, --version      Display version information.");
    puts("    -h, --help         Show this information.");
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
    if ( argc == 1 ) {
        Error(PROGRAM_NAME": no file specified.");
    }

    const struct option options[] = {
        { "create", no_argument, NULL, 'c' },
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'v' },
        { "config", required_argument, NULL, 0 },
        { 0 },
    };

    bool create = false;
    int opt;
    while ( (opt = getopt_long(argc, argv, "cvh", options, NULL)) != -1 ) {
        switch ( opt ) {
            case 'c':
                create = true;
                break;
            case 'v':
                printf("qe Version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
                printf("Copyright (C) Thomas Foster 2024\n");
                break;
            case 'h':
                PrintUsage();
                return EXIT_SUCCESS;
            case '?':
                Error(PROGRAM_NAME": invalid option '%c'", opt);
            case ':':
                Error(PROGRAM_NAME": option '%c' requires argument", opt);
            default:
                Error(PROGRAM_NAME": bad arguments");
        }
    }

    const char * document_path = argv[optind];

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
