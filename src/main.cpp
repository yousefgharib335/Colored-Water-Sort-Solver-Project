#include <algorithm>
#include <cstddef>
#include <iostream>
#include <queue>
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

struct SearchNode {
    State state;
    std::size_t parent = 0;
    Move move;
    bool hasParent = false;
};

std::string encodeState(const State& state) {
    std::ostringstream encoded;
    for (const Tube& tube : state) {
        encoded << '|';
        for (int color : tube) {
            encoded << color << ',';
        }
    }
    return encoded.str();
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

std::vector<Move> reconstructMoves(const std::vector<SearchNode>& nodes,
                                   std::size_t solutionIndex) {
    std::vector<Move> moves;
    while (nodes[solutionIndex].hasParent) {
        moves.push_back(nodes[solutionIndex].move);
        solutionIndex = nodes[solutionIndex].parent;
    }
    std::reverse(moves.begin(), moves.end());
    return moves;
}

bool findShortestSolution(const State& initialState, int capacity,
                          std::vector<Move>& solution, State& finalState) {
    std::vector<SearchNode> nodes;
    std::queue<std::size_t> frontier;
    std::unordered_map<std::string, std::size_t> visited;

    nodes.push_back({initialState, 0, {}, false});
    frontier.push(0);
    visited.emplace(encodeState(initialState), 0);

    while (!frontier.empty()) {
        const std::size_t currentIndex = frontier.front();
        frontier.pop();
        const State currentState = nodes[currentIndex].state;

        if (isSolved(currentState, capacity)) {
            solution = reconstructMoves(nodes, currentIndex);
            finalState = currentState;
            return true;
        }

        for (int source = 0; source < static_cast<int>(currentState.size());
             ++source) {
            for (int destination = 0;
                 destination < static_cast<int>(currentState.size());
                 ++destination) {
                if (source == destination) {
                    continue;
                }

                State nextState = currentState;
                if (!pour(nextState, source, destination, capacity)) {
                    continue;
                }

                std::string key = encodeState(nextState);
                if (visited.find(key) != visited.end()) {
                    continue;
                }

                const std::size_t nextIndex = nodes.size();
                visited.emplace(std::move(key), nextIndex);
                nodes.push_back(
                    {std::move(nextState), currentIndex, {source, destination}, true});
                frontier.push(nextIndex);
            }
        }
    }
    return false;
}

void printState(const State& state, int capacity) {
    for (std::size_t index = 0; index < state.size(); ++index) {
        std::cout << "Tube " << index + 1 << ": [";
        for (int layer = 0; layer < capacity; ++layer) {
            if (layer > 0) {
                std::cout << ',';
            }
            std::cout << (layer < static_cast<int>(state[index].size())
                              ? state[index][layer]
                              : 0);
        }
        std::cout << "]\n";
    }
}

bool readInput(State& state, int& capacity) {
    int tubeCount = 0;
    if (!(std::cin >> tubeCount) || !(std::cin >> capacity)) {
        std::cerr << "Invalid input: expected the number of tubes and capacity.\n";
        return false;
    }
    if (tubeCount <= 0 || capacity <= 0) {
        std::cerr << "Invalid input: tube count and capacity must be positive.\n";
        return false;
    }

    state.assign(tubeCount, {});
    for (int tube = 0; tube < tubeCount; ++tube) {
        bool sawEmptyPosition = false;
        for (int layer = 0; layer < capacity; ++layer) {
            int color = 0;
            if (!(std::cin >> color) || color < 0) {
                std::cerr << "Invalid input: each layer must be a nonnegative integer.\n";
                return false;
            }
            if (color == 0) {
                sawEmptyPosition = true;
            } else {
                if (sawEmptyPosition) {
                    std::cerr << "Invalid input: zeros must be at the end of each "
                                 "tube line.\n";
                    return false;
                }
                state[tube].push_back(color);
            }
        }
    }
    return true;
}

int main() {
    State initialState;
    int capacity = 0;
    if (!readInput(initialState, capacity)) {
        return 1;
    }

    std::vector<Move> solution;
    State finalState;
    if (!findShortestSolution(initialState, capacity, solution, finalState)) {
        std::cout << "No solution exists.\n";
        return 0;
    }

    std::cout << "Solution found.\n";
    std::cout << "Minimum number of moves: " << solution.size() << "\n";
    for (std::size_t index = 0; index < solution.size(); ++index) {
        std::cout << index + 1 << ". Tube " << solution[index].source + 1
                  << " -> Tube " << solution[index].destination + 1 << "\n";
    }
    std::cout << "Final state:\n";
    printState(finalState, capacity);
    return 0;
}
