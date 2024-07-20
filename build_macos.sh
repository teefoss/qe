#!/bin/bash
clang *.c macos/*.c -o qe -Imacos/include -Lmacos/lib -lSDL2 -lSDL2_ttf
