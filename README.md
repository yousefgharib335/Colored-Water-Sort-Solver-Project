# Colored Water Sort Solver

A C++17 console program that finds a minimum-move solution to a Colored Water
Sort puzzle using breadth-first search (BFS).

## Puzzle interpretation

- Tube values are entered from bottom to top.
- `0` represents unused space and may appear only at the end of a tube line.
- Every move transfers exactly one top layer from the source tube to the
  destination tube.
- A destination must be empty or have the same top color as the poured block.
- A state is solved when every nonempty tube contains only one color and each
  color is consolidated into a single tube. A solved tube may be partially
  filled when the number of layers of that color is below the tube capacity.

## Build

```sh
cmake -S . -B build
cmake --build build
```

With GCC directly:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o water_sort_solver
```

## Run

On Linux or macOS:

```sh
./build/water_sort_solver < samples/solvable.txt
```

On Windows with a CMake multi-configuration generator:

```powershell
Get-Content samples/solvable.txt | .\build\Debug\water_sort_solver.exe
```

## Algorithm and complexity

BFS explores states in increasing move count, so the first solved state reached
uses the minimum number of moves. An unordered map prevents repeated exploration.
Each state records its parent and incoming move so the solution can be rebuilt.

If `V` distinct states are reachable, memory use is `O(V * N * C)`. Each state
tries at most `N * (N - 1)` pours, giving `O(V * N^2 * C)` time with direct state
copying and encoding.

## Dashboard GUI

Run `build\water_sort_gui.exe` on Windows. The dashboard includes a shortest-path
solution explorer, a Guide & Rules card, and a Random example card. Each random
example is checked by the BFS solver before it is displayed, so generated puzzles
are unsolved and guaranteed to have a solution.