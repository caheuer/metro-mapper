#include "direction.hpp"

Direction opposite(Direction d) {
    switch (d) {
        case Direction::North: return Direction::South;
        case Direction::NorthEast: return Direction::SouthWest;
        case Direction::East: return Direction::West;
        case Direction::SouthEast: return Direction::NorthWest;
        case Direction::South: return Direction::North;
        case Direction::SouthWest: return Direction::NorthEast;
        case Direction::West: return Direction::East;
        case Direction::NorthWest: return Direction::SouthEast;
        default: return d;
    }
}

Direction rotateClockwise(Direction d, int steps) {
    int dir = (static_cast<int>(d) + 8 + steps % 8) % 8;
    return static_cast<Direction>(dir);
}

std::pair<int, int> offset(Direction dir) {
    switch (dir) {
        case Direction::North: return {0, -1};
        case Direction::NorthEast: return {1, -1};
        case Direction::East:  return {1, 0};
        case Direction::SouthEast: return {1, 1};
        case Direction::South: return {0, 1};
        case Direction::SouthWest: return {-1, 1};
        case Direction::West:  return {-1, 0};
        case Direction::NorthWest: return {-1, -1};
        default: return {0, 0};
    }
}

std::pair<float, float> offsetNormed(Direction dir) {
    switch (dir) {
        case Direction::North: return {0, -1};
        case Direction::NorthEast: return {0.7071, -0.7071};
        case Direction::East:  return {1, 0};
        case Direction::SouthEast: return {0.7071, 0.7071};
        case Direction::South: return {0, 1};
        case Direction::SouthWest: return {-0.7071, 0.7071};
        case Direction::West:  return {-1, 0};
        case Direction::NorthWest: return {-0.7071, -0.7071};
        default: return {0, 0};
    }
}