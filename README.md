# 3D_Mapper

Visualizer is located in branch visualizer

Usage :

- Connect robot to 9V power source

- Connect arduino to power

- Robot will execute ~12 360 turns

- Save data from serial and use script to convert it to a C array

- Compile visualizer with `g++ srcs/*.cpp -l SDL2 -lm -I incl/` *

- run ./a.out

*Note : If you have SDL3, compile with -lSDL3 instead. Tested only on Linux. Seems to require sdl2_compat package
