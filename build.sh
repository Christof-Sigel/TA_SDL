#!/bin/bash
IGNORE_WARNINGS=-Wno-c++11-extensions
clang++ -lSDL2 -ggdb -Wall -lGLEW -lGLU -lGL ta_sdl.cpp $IGNORE_WARNINGS -o ta_sdl


