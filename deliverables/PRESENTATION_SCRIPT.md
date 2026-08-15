# Colored Water Sort Solver - Presentation and Demo Script

Target duration: 8 to 9 minutes (maximum allowed: 10 minutes)

Replace `Speaker A` and `Speaker B` with student names. If this is a one-student
team, one person presents every section. If this is a two-student team, both
students must speak.

## 0:00-0:40 - Introduction (Speaker A)

"Our project is a Colored Water Sort solver written in C++17. The program reads
tubes from the console, searches for a solution with the minimum number of
moves, prints the move sequence and final state, or reports that no solution
exists. We also created a native Windows dashboard as a bonus interface."

Show the project folder and point to `src/main.cpp`, `src/gui_main.cpp`, and the
three files in `samples/`.

## 0:40-1:40 - Rules and state model (Speaker A)

- Values in each input row are ordered from bottom to top.
- Positive integers represent colors; zero is unused space.
- Zeros must occur only at the end of a console tube row.
- One move transfers exactly one top layer.
- The destination must have free capacity and be empty or have the same top
  color.
- The puzzle is solved when every nonempty tube is uniform and each color is in
  only one tube.

Use the official sample: `[1,2] [2,1] [] []` with capacity 2.

## 1:40-3:15 - Data structures (Speaker A)

- `vector<int>` represents one tube. The vector back is the top layer, so
  `push_back` and `pop_back` implement a transfer naturally.
- `vector<Tube>` represents one complete state.
- `queue<size_t>` is the BFS frontier and preserves first-in, first-out order.
- `unordered_map<string, size_t>` records visited states and prevents cycles.
- `vector<SearchNode>` stores each state, its parent index, and the move that
  reached it.
- `unordered_set<int>` verifies that one color is not split between two solved
  tubes.

## 3:15-4:30 - BFS and minimum-move guarantee (Speaker A)

Explain the loop:

1. Enqueue the initial arrangement and mark it visited.
2. Remove the oldest state.
3. If it is solved, follow parent links backward to reconstruct the moves.
4. Otherwise, try every ordered source and destination pair.
5. Enqueue every legal state that has not been visited.
6. If the queue becomes empty, no solution exists.

"Every edge is one layer transfer with the same cost. BFS explores all states
at move depth d before depth d+1, so the first solution found is a minimum-move
solution."

## 4:30-5:20 - Complexity (Speaker B)

Let N be the tube count, C the capacity, and V the number of reachable states
examined.

- Each state tries O(N squared) ordered transfers.
- The current implementation copies and encodes O(N times C) data per candidate.
- Worst-case time is O(V times N cubed times C).
- Memory is O(V times N times C).
- V can grow very quickly, so practical BFS limits depend on the arrangement,
  memory, and search depth.

Mention that the GUI display limits are 18 tubes and capacity 12, but not every
puzzle near those limits is practical for BFS.

## 5:20-6:25 - Console demo (Speaker B)

Run:

```powershell
Get-Content samples/solvable.txt | .\build\water_sort_solver.exe
```

Point out:

- `Solution found.`
- `Minimum number of moves: 3`
- The three numbered transfers.
- The final uniform tubes.

Then run:

```powershell
Get-Content samples/no_solution.txt | .\build\water_sort_solver.exe
Get-Content samples/already_solved.txt | .\build\water_sort_solver.exe
```

Explain that they return `No solution exists.` and a zero-move solution.

## 6:25-8:10 - GUI demo (Speaker B)

1. Start `water_sort_gui.exe`.
2. Select **Random example** and show the unsolved tubes.
3. Select **Solve puzzle**.
4. Read the minimum move count and first transfer.
5. Use **Next**, **Previous**, and **Show final** to explore the state sequence.
6. Open **Guide & rules** and briefly point out the input format and limits.
7. Select **Random example** again to show that a new solvable example is
   generated.

## 8:10-8:40 - Conclusion (Both, or final speaker)

"The required console program solves valid puzzles with a shortest move
sequence, detects unreachable goals, and validates the required input format.
The BFS queue, visited hash map, and parent-linked node list provide correctness,
cycle prevention, and efficient path reconstruction. The GUI adds a visual way
to test and explain the result."

## Recording checklist

- Keep the recording below 10 minutes.
- Capture readable console text and the full GUI window.
- Use the same files committed to GitHub.
- Both students speak if the team has two members.
- Do not expose passwords, tokens, notifications, or unrelated browser tabs.
- Upload the video to a location the instructor can open.
- Test the video link in a private/incognito browser window.
- Insert the final URL into the PDF report before LMS submission.
