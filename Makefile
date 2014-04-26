LDWIN = -mwindows -mconsole -lmingw32 -lSDL2main -lglew32 -lglu32 -lopengl32
LDSDL = -lSDL2
LDFLAGS = $(LDSDL) 
LDLIN = -lGLEW -lGLU -lGL

W32XX = i686-w64-mingw32-g++
W64XX = x86_64-w64-mingw32-g++
CXX=clang++

SOURCES=main.cpp lib/Shader.cpp lib/TriangleMesh.cpp lib/Object.cpp lib/Matrix.cpp file_format/HPIFile.cpp

OBJECTS = $(SOURCES:.cpp=.o)
W32OBJECTS = $(SOURCES:.cpp=.w32o)
W64OBJECTS = $(SOURCES:.cpp=.w64o)
EXECUTABLES = w32/ta_sdl.exe w64/ta_sdl_64.exe ta_sdl
CFLAGS=-Wall -std=c++11

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
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(W32OBJECTS) $(W64OBJECTS) $(EXECUTABLES)

rebuild:clean all

