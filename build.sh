#!/bin/bash
IGNORE_WARNINGS=
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl_linux.cpp ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
clang++ -std=c++11 -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng -shared -Wl,-soname,ta_sdl_game.so.1 -fPIC ta_sdl_game.cpp Logging.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl_game.so
clang++ -std=c++11 -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl_platform.cpp Logging.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl



