#!/bin/bash
IGNORE_WARNINGS=-Wno-c++11-extensions
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl_linux.cpp ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl_platform.cpp ta_sdl_game.cpp Logging.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl


