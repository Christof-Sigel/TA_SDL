LDWIN = -mwindows -mconsole -lmingw32 -lSDL2main -lglew32 -lglu32 -lopengl32
LDSDL = -lSDL2
LDFLAGS = $(LDSDL) -ggdb -D__CSGL_DEBUG_
LDLIN = -lGLEW -lGLU -lGL


W32XX = i686-w64-mingw32-g++
W64XX = x86_64-w64-mingw32-g++
CXX=clang++

SOURCES=main.cpp lib/Shader.cpp lib/TriangleMesh.cpp lib/Object.cpp lib/Matrix.cpp file_format/HPIFile.cpp file_format/3DOFile.cpp file_format/Gaf.cpp

OBJECTS = $(SOURCES:.cpp=.o)
W32OBJECTS = $(SOURCES:.cpp=.w32o)
W64OBJECTS = $(SOURCES:.cpp=.w64o)
EXECUTABLES = ta_sdl_32.exe ta_sdl_64.exe ta_sdl
CFLAGS=-Wall -std=c++11 -ggdb -Ilib/ -O0

all: $(EXECUTABLES)

w32/ta_sdl.exe: $(W32OBJECTS)
	$(W32XX) $(W32OBJECTS) $(LDWIN) $(LDFLAGS) -o $@

w64/ta_sdl_64.exe: $(W64OBJECTS)
	$(W64XX) $(W64OBJECTS) $(LDWIN) $(LDFLAGS) -o $@

ta_sdl: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDLIN) $(LDFLAGS) -o $@

%.w64o: %.cpp Makefile
	$(W64XX) $(CFLAGS) -c $< -o $@

%.w32o: %.cpp Makefile
	$(W32XX) $(CFLAGS) -c $< -o $@

%.o: %.cpp Makefile
	$(CXX) -Xclang -fcolor-diagnostics $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(W32OBJECTS) $(W64OBJECTS) $(EXECUTABLES)

rebuild:clean all

git-hook: clean
	./build.w32.sh 2>&1 | ansi2html > /srv/http/TA_SDL/win32.html
	./build.w64.sh 2>&1 | ansi2html > /srv/http/TA_SDL/win64.html
	./build.sh 2>&1 | ansi2html > /srv/http/TA_SDL/linux.html
	PYTHONIOENCODING=utf-8 gitinspector -HTlr -F html > /srv/http/TA_SDL/git.html
	cp ta_sdl_32.exe ta_sdl_64.exe ta_sdl /srv/http/TA_SDL
