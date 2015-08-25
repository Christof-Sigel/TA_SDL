#!/bin/bash
python glimports.py glXGetProcAddress "const GLubyte *"
IGNORE_WARNINGS="-Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-cast-align -Wno-unknown-pragmas -Wno-unused-function"
#TODO(Christof): re-enable -Wunused-function when we start using all the unit stuff again
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
#clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl_linux.cpp ta_sdl.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl
clang++ -std=c++11 -lSDL2 -ggdb -Weverything -lGLU -lGL -lz -lpng -shared -Wl,-soname,ta_sdl_game.so.1 -fPIC ta_sdl_game.cpp Logging.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -o ta_sdl_game.so.new
mv -f ta_sdl_game.so.new ta_sdl_game.so
clang++ -std=c++11 -lSDL2 -ggdb -Weverything -lGLU -lGL -lz -lpng ta_sdl_platform.cpp Logging.cpp $IGNORE_WARNINGS -D__CSGL_DEBUG__ -DDLL_NAME=./ta_sdl_game.so -o ta_sdl.new
mv -f ta_sdl.new ta_sdl
