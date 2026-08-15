#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstddef>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using Tube = std::vector<int>;
using State = std::vector<Tube>;

struct Move {
    int source = -1;
    int destination = -1;
};

struct Node {
    State state;
    std::size_t parent = 0;
    Move move;
    bool hasParent = false;
};

constexpr int ID_INPUT = 101;
constexpr int ID_OUTPUT = 102;
constexpr int ID_SOLVE = 103;
constexpr int ID_EXAMPLE = 104;
constexpr int ID_CLEAR = 105;
constexpr int ID_PREVIOUS = 106;
constexpr int ID_NEXT = 107;
constexpr int ID_FINAL = 108;
constexpr int ID_GUIDE = 109;

constexpr COLORREF C_BG = RGB(8, 15, 22);
constexpr COLORREF C_PANEL = RGB(17, 27, 36);
constexpr COLORREF C_PANEL_2 = RGB(12, 21, 29);
constexpr COLORREF C_BORDER = RGB(38, 56, 70);
constexpr COLORREF C_TEXT = RGB(242, 246, 249);
constexpr COLORREF C_MUTED = RGB(142, 166, 186);
constexpr COLORREF C_BLUE = RGB(51, 101, 181);
constexpr COLORREF C_BLUE_LIGHT = RGB(83, 151, 255);
constexpr COLORREF C_GREEN = RGB(25, 113, 74);
constexpr COLORREF C_GREEN_LIGHT = RGB(70, 211, 154);
constexpr COLORREF C_RED_DARK = RGB(19, 14, 18);
constexpr COLORREF C_RED = RGB(235, 75, 104);

HWND gInput = nullptr;
HWND gOutput = nullptr;
HWND gSolve = nullptr;
HWND gExample = nullptr;
HWND gClear = nullptr;
HWND gPrevious = nullptr;
HWND gNext = nullptr;
HWND gFinal = nullptr;
HWND gGuide = nullptr;

HFONT gTitleFont = nullptr;
HFONT gEyebrowFont = nullptr;
HFONT gHeadingFont = nullptr;
HFONT gBodyFont = nullptr;
HFONT gSmallFont = nullptr;
HFONT gMonoFont = nullptr;
HBRUSH gBackgroundBrush = nullptr;
HBRUSH gPanelBrush = nullptr;
HBRUSH gPanel2Brush = nullptr;

std::vector<State> gHistory;
std::vector<Move> gMoves;
std::size_t gStep = 0;
int gCapacity = 0;
std::string gStatus = "ready";

struct Layout {
    int left = 0;
    int right = 0;
    int width = 0;
    int headerBottom = 78;
    RECT solveCard{};
    RECT exampleCard{};
    RECT clearCard{};
    RECT guideCard{};
    RECT inputPanel{};
    RECT outputPanel{};
    RECT explorerPanel{};
};

Layout calculateLayout(const RECT& client) {
    Layout layout;
    const int clientWidth = static_cast<int>(client.right);
    const int clientHeight = static_cast<int>(client.bottom);
    layout.width = std::min(1160, clientWidth - 36);
    layout.left = (clientWidth - layout.width) / 2;
    layout.right = layout.left + layout.width;

    const int gap = 16;
    const int half = (layout.width - gap) / 2;
    layout.solveCard = {layout.left, 102, layout.left + half, 194};
    layout.exampleCard = {layout.left + half + gap, 102, layout.right, 194};
    layout.clearCard = {layout.left, 210, layout.left + half, 296};
    layout.guideCard = {layout.left + half + gap, 210, layout.right, 296};

    const int editorTop = 314;
    const int editorBottom = std::min(590, clientHeight - 248);
    layout.inputPanel = {layout.left, editorTop, layout.left + half, editorBottom};
    layout.outputPanel = {layout.left + half + gap, editorTop, layout.right,
                          editorBottom};
    layout.explorerPanel = {layout.left, editorBottom + 16, layout.right,
                            clientHeight - 18};
    return layout;
}

std::string encodeState(const State& state) {
    std::ostringstream output;
    for (const Tube& tube : state) {
        output << '|';
        for (int color : tube) {
            output << color << ',';
        }
    }
    return output.str();
}

bool isSolved(const State& state, int /*capacity*/) {
    std::unordered_set<int> consolidatedColors;
    for (const Tube& tube : state) {
        if (tube.empty()) {
            continue;
        }
        const int color = tube.front();
        if (!std::all_of(tube.begin(), tube.end(),
                         [&](int layer) { return layer == color; }) ||
            !consolidatedColors.insert(color).second) {
            return false;
        }
    }
    return true;
}


bool pour(State& state, int source, int destination, int capacity) {
    Tube& from = state[source];
    Tube& to = state[destination];
    if (from.empty() || static_cast<int>(to.size()) == capacity) {
        return false;
    }
    const int color = from.back();
    if (!to.empty() && to.back() != color) {
        return false;
    }
    from.pop_back();
    to.push_back(color);
    return true;
}

std::vector<Move> reconstruct(const std::vector<Node>& nodes, std::size_t index) {
    std::vector<Move> result;
    while (nodes[index].hasParent) {
        result.push_back(nodes[index].move);
        index = nodes[index].parent;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

bool solveShortest(const State& initial, int capacity, std::vector<Move>& moves,
                   State& finalState) {
    std::vector<Node> nodes;
    std::queue<std::size_t> queue;
    std::unordered_map<std::string, std::size_t> visited;
    nodes.push_back({initial, 0, {}, false});
    queue.push(0);
    visited.emplace(encodeState(initial), 0);

    while (!queue.empty()) {
        const std::size_t index = queue.front();
        queue.pop();
        const State current = nodes[index].state;
        if (isSolved(current, capacity)) {
            moves = reconstruct(nodes, index);
            finalState = current;
            return true;
        }

        for (int source = 0; source < static_cast<int>(current.size()); ++source) {
            for (int destination = 0;
                 destination < static_cast<int>(current.size()); ++destination) {
                if (source == destination) {
                    continue;
                }
                State next = current;
                if (!pour(next, source, destination, capacity)) {
                    continue;
                }
                std::string key = encodeState(next);
                if (visited.find(key) != visited.end()) {
                    continue;
                }
                const std::size_t nextIndex = nodes.size();
                visited.emplace(std::move(key), nextIndex);
                nodes.push_back(
                    {std::move(next), index, {source, destination}, true});
                queue.push(nextIndex);
            }
        }
    }
    return false;
}

bool parsePuzzle(const std::string& text, State& state, int& capacity,
                 std::string& error) {
    std::istringstream inputLines(text);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputLines, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) {
            lines.push_back(line);
        }
    }

    if (lines.size() < 2) {
        error = "Enter N on the first line and C on the second line.";
        return false;
    }

    int count = 0;
    std::istringstream countLine(lines[0]);
    std::istringstream capacityLine(lines[1]);
    std::string trailing;
    if (!(countLine >> count) || countLine >> trailing ||
        !(capacityLine >> capacity) || capacityLine >> trailing || count <= 0 ||
        capacity <= 0) {
        error = "N and C must be positive integers.";
        return false;
    }
    if (count > 18 || capacity > 12) {
        error = "The dashboard supports up to 18 tubes and capacity 12.";
        return false;
    }

    if (lines.size() > static_cast<std::size_t>(count + 2)) {
        error = "Expected " + std::to_string(count) +
                " tube lines after N and C, but found " +
                std::to_string(lines.size() - 2) + ".";
        return false;
    }

    state.assign(count, {});
    for (int tube = 0; tube < count; ++tube) {
        if (tube + 2 >= static_cast<int>(lines.size())) {
            continue;
        }
        std::istringstream tubeLine(lines[tube + 2]);
        std::vector<int> values;
        int color = 0;
        while (tubeLine >> color) {
            values.push_back(color);
        }
        if (!tubeLine.eof()) {
            error = "Tube " + std::to_string(tube + 1) +
                    " contains a value that is not an integer.";
            return false;
        }
        if (values.size() > static_cast<std::size_t>(capacity)) {
            error = "Tube " + std::to_string(tube + 1) + " has " +
                    std::to_string(values.size()) + " values, but capacity C is " +
                    std::to_string(capacity) + ".";
            return false;
        }

        bool sawZero = false;
        for (int value : values) {
            color = value;
            if (color < 0) {
                error = "Tube " + std::to_string(tube + 1) +
                        " contains a negative color code.";
                return false;
            }
            if (color == 0) {
                sawZero = true;
            } else {
                if (sawZero) {
                    error = "Tube " + std::to_string(tube + 1) +
                            " has a color after an empty position (0).";
                    return false;
                }
                state[tube].push_back(color);
            }
        }
    }
    return true;
}

std::string stateText(const State& state, int capacity) {
    std::ostringstream output;
    for (std::size_t tube = 0; tube < state.size(); ++tube) {
        output << "Tube " << tube + 1 << ": [";
        for (int layer = 0; layer < capacity; ++layer) {
            if (layer) {
                output << ',';
            }
            output << (layer < static_cast<int>(state[tube].size())
                           ? state[tube][layer]
                           : 0);
        }
        output << "]\r\n";
    }
    return output.str();
}

void buildHistory(const State& initial) {
    gHistory.clear();
    gHistory.push_back(initial);
    State current = initial;
    for (const Move& move : gMoves) {
        pour(current, move.source, move.destination, gCapacity);
        gHistory.push_back(current);
    }
    gStep = 0;
}

void setControlFont(HWND control, HFONT font) {
    SendMessage(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

void updateNavigation(HWND window) {
    EnableWindow(gPrevious, !gHistory.empty() && gStep > 0);
    EnableWindow(gNext, !gHistory.empty() && gStep + 1 < gHistory.size());
    EnableWindow(gFinal, !gHistory.empty() && gStep + 1 < gHistory.size());
    InvalidateRect(gPrevious, nullptr, TRUE);
    InvalidateRect(gNext, nullptr, TRUE);
    InvalidateRect(gFinal, nullptr, TRUE);
    InvalidateRect(window, nullptr, TRUE);
}

void clearSolution(HWND window, bool clearInput) {
    if (clearInput) {
        SetWindowTextA(gInput, "");
    }
    SetWindowTextA(gOutput, "Waiting for a puzzle.\r\n\r\nUse the sample or enter your own values.");
    gHistory.clear();
    gMoves.clear();
    gStep = 0;
    gCapacity = 0;
    gStatus = "ready";
    updateNavigation(window);
}

std::string puzzleInputText(const State& state, int capacity) {
    std::ostringstream output;
    output << state.size() << "\r\n" << capacity << "\r\n";
    for (const Tube& tube : state) {
        for (int layer = 0; layer < capacity; ++layer) {
            if (layer > 0) {
                output << ' ';
            }
            output << (layer < static_cast<int>(tube.size()) ? tube[layer] : 0);
        }
        output << "\r\n";
    }
    return output.str();
}

bool generateRandomExample(State& initial, int& capacity, std::string& text) {
    static std::mt19937 generator(std::random_device{}());
    static std::string previousExample;
    std::uniform_int_distribution<int> sizeChoice(2, 3);

    for (int attempt = 0; attempt < 30; ++attempt) {
        capacity = sizeChoice(generator);
        const int colorCount = sizeChoice(generator);
        const int tubeCount = colorCount + 2;

        std::vector<int> layers;
        for (int color = 1; color <= colorCount; ++color) {
            for (int layer = 0; layer < capacity; ++layer) {
                layers.push_back(color);
            }
        }
        std::shuffle(layers.begin(), layers.end(), generator);

        State candidate(tubeCount);
        std::size_t position = 0;
        for (int tube = 0; tube < colorCount; ++tube) {
            for (int layer = 0; layer < capacity; ++layer) {
                candidate[tube].push_back(layers[position++]);
            }
        }
        if (isSolved(candidate, capacity)) {
            continue;
        }

        std::vector<Move> solution;
        State finalState;
        if (!solveShortest(candidate, capacity, solution, finalState) ||
            solution.empty()) {
            continue;
        }

        const std::string candidateText = puzzleInputText(candidate, capacity);
        if (candidateText == previousExample) {
            continue;
        }
        initial = std::move(candidate);
        text = candidateText;
        previousExample = candidateText;
        return true;
    }

    capacity = 2;
    initial = {{1, 2}, {2, 1}, {}, {}};
    text = puzzleInputText(initial, capacity);
    previousExample = text;
    return true;
}

void loadExample(HWND window) {
    State initial;
    std::string example;
    int capacity = 0;
    generateRandomExample(initial, capacity, example);
    SetWindowTextA(gInput, example.c_str());

    gCapacity = capacity;
    gMoves.clear();
    gHistory.assign(1, initial);
    gStep = 0;
    gStatus = "random example";
    SetWindowTextA(
        gOutput,
        "New random solvable puzzle loaded.\r\n\r\nThe unsolved tubes are shown below.\r\nSelect Random example again for another puzzle, or Solve puzzle to continue.");
    updateNavigation(window);
}

void showGuide(HWND window) {
    gStatus = "guide";
    SetWindowTextA(
        gOutput,
        "QUICK GUIDE\r\n\r\n"
        "INPUT FORMAT\r\n"
        "1. First line: number of tubes (N).\r\n"
        "2. Second line: tube capacity (C).\r\n"
        "3. Next lines: colors from bottom to top.\r\n"
        "4. Use 0 for empty space. Missing rows or values are also empty.\r\n\r\n"
        "PUZZLE RULES\r\n"
        "- Pour only into an empty tube or onto the same top color.\r\n"
        "- Each move transfers exactly one top layer.\r\n"
        "- The goal is one tube per color, with no mixed tubes.\r\n\r\n"
        "CONTROLS\r\n"
        "- Solve puzzle finds a shortest solution using BFS.\r\n"
        "- Random example generates a new verified solvable puzzle.\r\n"
        "- Previous, Next, and Show final explore the solution states.\r\n\r\n"
        "LIMITS & PERFORMANCE\r\n"
        "- Hard GUI limit: 18 tubes and capacity 12.\r\n"
        "- Recommended range: 4-10 tubes and capacity 2-4.\r\n"
        "- Random examples use 4-5 tubes and capacity 2-3.\r\n"
        "- Larger puzzles may be slow or use substantial memory because BFS stores every visited state.");
    InvalidateRect(window, nullptr, TRUE);
}

void solveFromInput(HWND window) {
    const int length = GetWindowTextLengthA(gInput);
    std::vector<char> buffer(static_cast<std::size_t>(length) + 1);
    GetWindowTextA(gInput, buffer.data(), static_cast<int>(buffer.size()));

    State initial;
    std::string error;
    int capacity = 0;
    if (!parsePuzzle(buffer.data(), initial, capacity, error)) {
        gStatus = "input error";
        gHistory.clear();
        gMoves.clear();
        gStep = 0;
        gCapacity = 0;
        const std::string details =
            "Input error\r\n\r\n" + error +
            "\r\n\r\nTip: missing values and missing tube rows are treated as empty spaces.";
        SetWindowTextA(gOutput, details.c_str());
        updateNavigation(window);
        MessageBoxA(window, error.c_str(), "Invalid puzzle", MB_OK | MB_ICONERROR);
        return;
    }

    gStatus = "searching";
    EnableWindow(gSolve, FALSE);
    SetWindowTextA(gOutput, "Searching all reachable states...");
    InvalidateRect(window, nullptr, TRUE);
    UpdateWindow(window);
    SetCursor(LoadCursor(nullptr, IDC_WAIT));

    std::vector<Move> moves;
    State finalState;
    const bool found = solveShortest(initial, capacity, moves, finalState);
    SetCursor(LoadCursor(nullptr, IDC_ARROW));
    EnableWindow(gSolve, TRUE);
    gCapacity = capacity;
    gMoves = moves;

    std::ostringstream result;
    if (!found) {
        gHistory.assign(1, initial);
        gStep = 0;
        gMoves.clear();
        gStatus = "no solution";
        result << "No solution exists.\r\n\r\nEvery reachable state was checked.";
    } else {
        buildHistory(initial);
        gStatus = "solution ready";
        result << "Solution found\r\nMinimum moves: " << gMoves.size()
               << "\r\n\r\n";
        for (std::size_t i = 0; i < gMoves.size(); ++i) {
            result << i + 1 << ". Tube " << gMoves[i].source + 1 << " -> Tube "
                   << gMoves[i].destination + 1 << "\r\n";
        }
        result << "\r\nFinal state\r\n" << stateText(finalState, capacity);
    }
    SetWindowTextA(gOutput, result.str().c_str());
    updateNavigation(window);
}

void layoutChildren(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    const Layout l = calculateLayout(client);

    MoveWindow(gSolve, l.solveCard.left, l.solveCard.top,
               l.solveCard.right - l.solveCard.left,
               l.solveCard.bottom - l.solveCard.top, TRUE);
    MoveWindow(gExample, l.exampleCard.left, l.exampleCard.top,
               l.exampleCard.right - l.exampleCard.left,
               l.exampleCard.bottom - l.exampleCard.top, TRUE);
    MoveWindow(gClear, l.clearCard.left, l.clearCard.top,
               l.clearCard.right - l.clearCard.left,
               l.clearCard.bottom - l.clearCard.top, TRUE);
    MoveWindow(gGuide, l.guideCard.left, l.guideCard.top,
               l.guideCard.right - l.guideCard.left,
               l.guideCard.bottom - l.guideCard.top, TRUE);

    MoveWindow(gInput, l.inputPanel.left + 20, l.inputPanel.top + 54,
               l.inputPanel.right - l.inputPanel.left - 40,
               l.inputPanel.bottom - l.inputPanel.top - 72, TRUE);
    MoveWindow(gOutput, l.outputPanel.left + 20, l.outputPanel.top + 54,
               l.outputPanel.right - l.outputPanel.left - 40,
               l.outputPanel.bottom - l.outputPanel.top - 72, TRUE);

    const int navTop = l.explorerPanel.top + 14;
    MoveWindow(gPrevious, l.explorerPanel.right - 338, navTop, 96, 34, TRUE);
    MoveWindow(gNext, l.explorerPanel.right - 232, navTop, 96, 34, TRUE);
    MoveWindow(gFinal, l.explorerPanel.right - 126, navTop, 106, 34, TRUE);
}

void fillRounded(HDC dc, const RECT& rect, COLORREF color, int radius,
                 COLORREF border = C_BORDER) {
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void drawTextLine(HDC dc, const std::string& text, int x, int y, HFONT font,
                  COLORREF color, UINT alignment = TA_LEFT | TA_TOP) {
    SetBkMode(dc, TRANSPARENT);
    SetTextAlign(dc, alignment);
    SetTextColor(dc, color);
    SelectObject(dc, font);
    TextOutA(dc, x, y, text.c_str(), static_cast<int>(text.size()));
}

void drawHeader(HDC dc, const Layout& l) {
    drawTextLine(dc, "WATER SORT PROJECT", l.left + 10, 12, gEyebrowFont,
                 C_BLUE_LIGHT);
    drawTextLine(dc, "Colored Water Dashboard", l.left + 10, 32, gTitleFont,
                 C_TEXT);

    HBRUSH dot = CreateSolidBrush(C_GREEN_LIGHT);
    HGDIOBJ old = SelectObject(dc, dot);
    Ellipse(dc, l.right - 126, 29, l.right - 114, 41);
    SelectObject(dc, old);
    DeleteObject(dot);
    drawTextLine(dc, gStatus, l.right - 105, 27, gSmallFont, C_MUTED);

    HPEN divider = CreatePen(PS_SOLID, 1, C_BORDER);
    HGDIOBJ oldPen = SelectObject(dc, divider);
    MoveToEx(dc, 0, l.headerBottom, nullptr);
    LineTo(dc, l.right + l.left, l.headerBottom);
    SelectObject(dc, oldPen);
    DeleteObject(divider);
}

void drawPanelTitles(HDC dc, const Layout& l) {
    fillRounded(dc, l.inputPanel, C_PANEL, 12);
    fillRounded(dc, l.outputPanel, C_PANEL, 12);
    fillRounded(dc, l.explorerPanel, C_PANEL, 12);

    drawTextLine(dc, "PUZZLE INPUT", l.inputPanel.left + 20,
                 l.inputPanel.top + 18, gEyebrowFont, C_MUTED);
    drawTextLine(dc, "missing rows / values = empty", l.inputPanel.right - 20,
                 l.inputPanel.top + 18, gSmallFont, C_TEXT, TA_RIGHT | TA_TOP);

    drawTextLine(dc, "MINIMUM-MOVE SOLUTION", l.outputPanel.left + 20,
                 l.outputPanel.top + 18, gEyebrowFont, C_MUTED);
    const std::string moveCount = gHistory.empty()
                                      ? "-- moves"
                                      : std::to_string(gMoves.size()) + " moves";
    drawTextLine(dc, moveCount, l.outputPanel.right - 20,
                 l.outputPanel.top + 18, gSmallFont, C_TEXT,
                 TA_RIGHT | TA_TOP);

    drawTextLine(dc, "SOLUTION EXPLORER", l.explorerPanel.left + 20,
                 l.explorerPanel.top + 20, gEyebrowFont, C_MUTED);
    std::string stepText = "Solve a puzzle to begin";
    if (!gHistory.empty()) {
        if (gStep == 0) {
            stepText = "INITIAL STATE  /  STEP 0 OF " +
                       std::to_string(gMoves.size());
        } else {
            stepText = "STEP " + std::to_string(gStep) + " OF " +
                       std::to_string(gMoves.size()) + "  /  TUBE " +
                       std::to_string(gMoves[gStep - 1].source + 1) + " -> TUBE " +
                       std::to_string(gMoves[gStep - 1].destination + 1);
        }
    }
    drawTextLine(dc, stepText, l.explorerPanel.left + 20,
                 l.explorerPanel.top + 46, gSmallFont, C_TEXT);
}

COLORREF liquidColor(int code) {
    static const COLORREF colors[] = {
        RGB(255, 85, 111), RGB(69, 139, 255), RGB(59, 204, 133),
        RGB(255, 183, 77), RGB(171, 108, 255), RGB(39, 201, 210),
        RGB(255, 120, 73), RGB(153, 127, 111), RGB(245, 91, 181),
        RGB(48, 190, 169), RGB(159, 208, 74), RGB(112, 112, 255)};
    return colors[(code - 1) % (sizeof(colors) / sizeof(colors[0]))];
}

void drawTubes(HDC dc, const Layout& l) {
    if (gHistory.empty() || gCapacity == 0) {
        const int centerX = (l.explorerPanel.left + l.explorerPanel.right) / 2;
        const int centerY = (l.explorerPanel.top + l.explorerPanel.bottom) / 2;
        drawTextLine(dc, "No state to display", centerX, centerY - 8,
                     gHeadingFont, C_MUTED, TA_CENTER | TA_TOP);
        return;
    }

    const State& state = gHistory[gStep];
    const int count = static_cast<int>(state.size());
    const int areaLeft = l.explorerPanel.left + 22;
    const int areaRight = l.explorerPanel.right - 22;
    const int areaTop = l.explorerPanel.top + 82;
    const int areaBottom = l.explorerPanel.bottom - 18;
    const int slot = std::max(42, (areaRight - areaLeft) / std::max(1, count));
    const int tubeWidth = std::max(28, std::min(64, slot - 16));
    const int cellHeight = std::max(
        15, std::min(36, (areaBottom - areaTop - 30) / std::max(1, gCapacity)));
    const int tubeHeight = cellHeight * gCapacity;

    for (int tube = 0; tube < count; ++tube) {
        const int centerX = areaLeft + tube * slot + slot / 2;
        const int left = centerX - tubeWidth / 2;
        const int bottom = areaTop + tubeHeight;

        for (int layer = 0; layer < gCapacity; ++layer) {
            const int yBottom = bottom - layer * cellHeight;
            RECT fill{left + 4, yBottom - cellHeight, left + tubeWidth - 4,
                      yBottom};
            const bool occupied = layer < static_cast<int>(state[tube].size());
            HBRUSH brush = CreateSolidBrush(
                occupied ? liquidColor(state[tube][layer]) : C_PANEL_2);
            FillRect(dc, &fill, brush);
            DeleteObject(brush);
            if (occupied && tubeWidth >= 40 && cellHeight >= 20) {
                drawTextLine(dc, std::to_string(state[tube][layer]), centerX,
                             yBottom - cellHeight / 2 - 7, gSmallFont, C_TEXT,
                             TA_CENTER | TA_TOP);
            }
        }

        HPEN tubePen = CreatePen(PS_SOLID, 2, RGB(78, 100, 117));
        HGDIOBJ oldPen = SelectObject(dc, tubePen);
        MoveToEx(dc, left, areaTop, nullptr);
        LineTo(dc, left, bottom);
        LineTo(dc, left + tubeWidth, bottom);
        LineTo(dc, left + tubeWidth, areaTop);
        SelectObject(dc, oldPen);
        DeleteObject(tubePen);
        drawTextLine(dc, "T" + std::to_string(tube + 1), centerX, bottom + 10,
                     gSmallFont, C_MUTED, TA_CENTER | TA_TOP);
    }
}

void drawActionButton(const DRAWITEMSTRUCT& item) {
    COLORREF fill = C_PANEL_2;
    COLORREF border = C_BORDER;
    const char* badge = "";
    const char* title = "";
    const char* subtitle = "";

    if (item.CtlID == ID_SOLVE) {
        fill = C_GREEN;
        border = RGB(38, 135, 91);
        badge = "GO";
        title = "Solve puzzle";
        subtitle = "Find the minimum move sequence";
    } else if (item.CtlID == ID_EXAMPLE) {
        fill = C_BLUE;
        border = RGB(70, 125, 207);
        badge = "RND";
        title = "Random example";
        subtitle = "Generate a new solvable puzzle";
    } else if (item.CtlID == ID_GUIDE) {
        fill = RGB(24, 43, 58);
        border = RGB(48, 78, 99);
        badge = "?";
        title = "Guide & rules";
        subtitle = "Input format and solver explanation";
    } else {
        fill = C_RED_DARK;
        border = RGB(36, 29, 35);
        badge = "X";
        title = "Clear workspace";
        subtitle = "Remove the current input and result";
    }
    if (item.itemState & ODS_SELECTED) {
        fill = RGB(GetRValue(fill) * 4 / 5, GetGValue(fill) * 4 / 5,
                   GetBValue(fill) * 4 / 5);
    }

    FillRect(item.hDC, &item.rcItem, gBackgroundBrush);
    fillRounded(item.hDC, item.rcItem, fill, 14, border);
    const int iconSize = 48;
    const int iconLeft = item.rcItem.left + 26;
    const int iconTop = item.rcItem.top +
                        (item.rcItem.bottom - item.rcItem.top - iconSize) / 2;
    const int textLeft = iconLeft + iconSize + 22;
    HBRUSH iconBrush = CreateSolidBrush(fill);
    HPEN iconPen = CreatePen(PS_SOLID, 1, RGB(188, 210, 224));
    HGDIOBJ oldBrush = SelectObject(item.hDC, iconBrush);
    HGDIOBJ oldPen = SelectObject(item.hDC, iconPen);
    Ellipse(item.hDC, iconLeft, iconTop, iconLeft + iconSize,
            iconTop + iconSize);
    SelectObject(item.hDC, oldPen);
    SelectObject(item.hDC, oldBrush);
    DeleteObject(iconPen);
    DeleteObject(iconBrush);

    drawTextLine(item.hDC, badge, iconLeft + iconSize / 2, iconTop + 15,
                 gEyebrowFont,
                 C_TEXT, TA_CENTER | TA_TOP);
    drawTextLine(item.hDC, title, textLeft, item.rcItem.top + 24,
                 gHeadingFont, C_TEXT);
    drawTextLine(item.hDC, subtitle, textLeft, item.rcItem.top + 54,
                 gSmallFont, RGB(222, 232, 239));
}

void drawNavigationButton(const DRAWITEMSTRUCT& item) {
    const bool disabled = item.itemState & ODS_DISABLED;
    COLORREF fill = disabled ? RGB(14, 23, 31) : RGB(31, 45, 57);
    if (item.itemState & ODS_SELECTED) {
        fill = RGB(44, 61, 75);
    }
    FillRect(item.hDC, &item.rcItem, gPanelBrush);
    fillRounded(item.hDC, item.rcItem, fill, 9,
                disabled ? RGB(29, 42, 53) : C_BORDER);

    char label[64]{};
    GetWindowTextA(item.hwndItem, label, sizeof(label));
    RECT textRect = item.rcItem;
    SetBkMode(item.hDC, TRANSPARENT);
    SetTextColor(item.hDC, disabled ? RGB(75, 94, 108) : C_TEXT);
    SelectObject(item.hDC, gSmallFont);
    DrawTextA(item.hDC, label, -1, &textRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        gTitleFont = CreateFontA(-27, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        gEyebrowFont = CreateFontA(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
                                   FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        gHeadingFont = CreateFontA(-20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE,
                                   FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        gBodyFont = CreateFontA(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        gSmallFont = CreateFontA(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        gMonoFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                FIXED_PITCH | FF_MODERN, "Cascadia Mono");

        gSolve = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                                  BS_OWNERDRAW,
                               0, 0, 1, 1, window,
                               reinterpret_cast<HMENU>(ID_SOLVE), nullptr, nullptr);
        gExample = CreateWindowA("BUTTON", "",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                                 0, 0, 1, 1, window,
                                 reinterpret_cast<HMENU>(ID_EXAMPLE), nullptr,
                                 nullptr);
        gClear = CreateWindowA("BUTTON", "",
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, 0,
                               0, 1, 1, window,
                               reinterpret_cast<HMENU>(ID_CLEAR), nullptr, nullptr);
        gGuide = CreateWindowA("BUTTON", "",
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, 0,
                               0, 1, 1, window,
                               reinterpret_cast<HMENU>(ID_GUIDE), nullptr, nullptr);

        gInput = CreateWindowExA(
            0, "EDIT", "4\r\n2\r\n1 2\r\n2 1\r\n0 0\r\n0 0",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL |
                ES_WANTRETURN | WS_VSCROLL,
            0, 0, 1, 1, window, reinterpret_cast<HMENU>(ID_INPUT), nullptr,
            nullptr);
        gOutput = CreateWindowExA(
            0, "EDIT", "Example A loaded.\r\n\r\nSelect Solve puzzle to begin.",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY
                ,
            0, 0, 1, 1, window, reinterpret_cast<HMENU>(ID_OUTPUT), nullptr,
            nullptr);
        setControlFont(gInput, gMonoFont);
        setControlFont(gOutput, gMonoFont);

        const DWORD navStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
        gPrevious = CreateWindowA("BUTTON", "< Previous", navStyle, 0, 0, 1, 1,
                                  window, reinterpret_cast<HMENU>(ID_PREVIOUS),
                                  nullptr, nullptr);
        gNext = CreateWindowA("BUTTON", "Next >", navStyle, 0, 0, 1, 1, window,
                              reinterpret_cast<HMENU>(ID_NEXT), nullptr, nullptr);
        gFinal = CreateWindowA("BUTTON", "Show final", navStyle, 0, 0, 1, 1,
                               window, reinterpret_cast<HMENU>(ID_FINAL), nullptr,
                               nullptr);

        layoutChildren(window);
        updateNavigation(window);
        return 0;
    }
    case WM_GETMINMAXINFO: {
        auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
        info->ptMinTrackSize.x = 980;
        info->ptMinTrackSize.y = 760;
        return 0;
    }
    case WM_SIZE:
        if (gInput) {
            layoutChildren(window);
        }
        InvalidateRect(window, nullptr, TRUE);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_SOLVE:
            solveFromInput(window);
            return 0;
        case ID_EXAMPLE:
            loadExample(window);
            return 0;
        case ID_GUIDE:
            showGuide(window);
            return 0;
        case ID_CLEAR:
            clearSolution(window, true);
            SetFocus(gInput);
            return 0;
        case ID_PREVIOUS:
            if (gStep > 0) {
                --gStep;
                updateNavigation(window);
            }
            return 0;
        case ID_NEXT:
            if (gStep + 1 < gHistory.size()) {
                ++gStep;
                updateNavigation(window);
            }
            return 0;
        case ID_FINAL:
            if (!gHistory.empty()) {
                gStep = gHistory.size() - 1;
                updateNavigation(window);
            }
            return 0;
        default:
            break;
        }
        break;
    case WM_DRAWITEM: {
        const auto& item = *reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (item.CtlID == ID_SOLVE || item.CtlID == ID_EXAMPLE ||
            item.CtlID == ID_GUIDE ||
            item.CtlID == ID_CLEAR) {
            drawActionButton(item);
            return TRUE;
        }
        drawNavigationButton(item);
        return TRUE;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, C_TEXT);
        SetBkColor(dc, C_PANEL_2);
        return reinterpret_cast<LRESULT>(gPanel2Brush);
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, C_TEXT);
        SetBkColor(dc, C_PANEL_2);
        return reinterpret_cast<LRESULT>(gPanel2Brush);
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        FillRect(dc, &client, gBackgroundBrush);
        const Layout layout = calculateLayout(client);
        drawHeader(dc, layout);
        drawPanelTitles(dc, layout);
        drawTubes(dc, layout);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_DESTROY:
        DeleteObject(gTitleFont);
        DeleteObject(gEyebrowFont);
        DeleteObject(gHeadingFont);
        DeleteObject(gBodyFont);
        DeleteObject(gSmallFont);
        DeleteObject(gMonoFont);
        DeleteObject(gBackgroundBrush);
        DeleteObject(gPanelBrush);
        DeleteObject(gPanel2Brush);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    gBackgroundBrush = CreateSolidBrush(C_BG);
    gPanelBrush = CreateSolidBrush(C_PANEL);
    gPanel2Brush = CreateSolidBrush(C_PANEL_2);

    WNDCLASSA windowClass{};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "WaterSortDashboardWindow";
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.hbrBackground = gBackgroundBrush;
    if (!RegisterClassA(&windowClass)) {
        return 1;
    }

    HWND window = CreateWindowExA(
        0, windowClass.lpszClassName, "Colored Water Dashboard",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1220, 880, nullptr,
        nullptr, instance, nullptr);
    if (!window) {
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return static_cast<int>(message.wParam);
}
