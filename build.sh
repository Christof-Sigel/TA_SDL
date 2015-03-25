#!/bin/bash
IGNORE_WARNINGS=-Wno-c++11-extensions
clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL -lz -lpng ta_sdl.cpp $IGNORE_WARNINGS -o ta_sdl


