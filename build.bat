@echo off

set CommonCompilerFlags=-Od -D_CRT_SECURE_NO_WARNINGS -MTd -nologo -fp:fast -fp:except- -EHsc -Gm- -GR- -EHa- -Zo -Oi  -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4505 -FC -Z7 -I./w64/include 


set CommonLinkerFlags= -opt:ref -incremental:no  user32.lib gdi32.lib winmm.lib

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags%  ta_sdl_game.cpp Logging.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameSetup -EXPORT:CheckResources -EXPORT:GameTeardown -EXPORT:GameUpdateAndRender -libpath:w64/lib OpenGL32.lib Glu32.lib libz.a glew32.lib
del lock.tmp
cl %CommonCompilerFlags% ta_sdl_platform.cpp Logging.cpp -Fmwin32_handmade.map -DDLL_NAME=ta_sdl_game.dll -DDLL_TEMP_NAME=ta_sdl_game_temp.dll  /link -libpath:w64/lib OpenGL32.lib Glu32.lib glew32.lib SDL2.lib %CommonLinkerFlags% -subsystem:console

