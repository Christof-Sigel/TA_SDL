@echo off
python glimports.py

REM TODO(Christof): enable warning 4505 when we are using the unit stuff again so we don't miss unreferenced functions
REM maybe also 4710 (function not inlined)
set CommonCompilerFlags=-Od -D_CRT_SECURE_NO_WARNINGS -MTd -nologo -fp:fast -fp:except- -EHsc -Gm- -GR- -EHa- -Zo -Oi  -Wall -wd4201 -wd4068 -wd5030 -wd4649 -wd4505 -wd4710  -FC -Z7 -I../w64/include


set CommonLinkerFlags= -opt:ref -incremental:no  user32.lib gdi32.lib winmm.lib

mkdir build
pushd build
REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags%  ..\ta_sdl_game.cpp ..\Logging.cpp -Fmta_sdl_game.map -LD -Feta_sdl.dll /link -incremental:no -opt:ref -PDB:ta_sdl_dll_%random%.pdb -EXPORT:GameSetup -EXPORT:CheckResources -EXPORT:GameTeardown -EXPORT:GameUpdateAndRender -libpath:../w64/lib OpenGL32.lib Glu32.lib libz.a SDL2.lib
del lock.tmp
cl %CommonCompilerFlags% -wd4514 ..\ta_sdl_platform.cpp ..\Logging.cpp -Fmta_sdl_platform.map -DDLL_NAME=ta_sdl.dll -DDLL_TEMP_NAME=ta_sdl_temp.dll -Feta_sdl.exe  /link -libpath:../w64/lib OpenGL32.lib Glu32.lib SDL2.lib %CommonLinkerFlags% -subsystem:console

popd
