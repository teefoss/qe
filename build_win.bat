@echo off
setlocal

set DEF=/DPLATFORM_WINDOWS
set SRC=*.c windows/*.c
set LIBDIR=windows\lib
set INC=/Iwindows\include
set LIBS="%LIBDIR%\SDL2.lib" "%LIBDIR%\SDL2main.lib" "%LIBDIR%\SDL2_ttf.lib" "shell32.lib"

cl %DEF% %SRC% %INC% /link %LIBS% "/SUBSYSTEM:CONSOLE" /out:qe.exe
del *.obj

endlocal