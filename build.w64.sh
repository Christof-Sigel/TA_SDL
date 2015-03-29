#!/bin/bash

x86_64-w64-mingw32-g++ -fdiagnostics-color -std=c++11 ta_sdl.cpp -mwindows -mconsole -lmingw32 -lSDL2main -lglew32 -lglu32 -lopengl32 -lSDL2 -lpng -lz -ggdb -D__CSGL_DEBUG__  -o ta_sdl_64.exe
