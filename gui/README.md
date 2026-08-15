# Windows GUI

This optional bonus interface is written in C++17 using the native Windows API,
so it does not require Qt or another GUI framework.

## Build with CMake

From the project root:

```powershell
cmake -S gui -B gui/build
cmake --build gui/build
```

## Build directly with MinGW GCC

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -mwindows src/gui_main.cpp -o water_sort_gui.exe -lgdi32 -luser32
```

The GUI accepts the same input format as the console program. It displays the
minimum move sequence and draws the final tubes using different colors.

