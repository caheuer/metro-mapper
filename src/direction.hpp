#pragma once
#include <utility>

enum class Direction {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest,
    None = -1
};

Direction opposite(Direction d);
Direction rotateClockwise(Direction d, int steps = 1);
std::pair<int, int> offset(Direction dir);
std::pair<float, float> offsetNormed(Direction dir);