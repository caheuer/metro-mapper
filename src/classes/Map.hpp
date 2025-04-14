#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <limits>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "json.hpp"
#include "../direction.hpp"

struct Color {
    int r, g, b; // RGB values (0-255)
};

struct Line {
    std::string name;
    Color color; // Hex color code, e.g., "#FF5733"
};

struct StationConnection {
    int to;
    Direction dir;
    std::vector<int> lineIds; 
};

struct Station {
    int id;
    std::string name;
    float x = 0, y = 0;
    Direction labelDir = Direction::North;
    std::vector<StationConnection> connections;
    bool isInterchange = false; // true if the station has multiple lines passing through it
    bool isTerminal = false; // true if the station is a terminal (has only one connection to another station)
    bool isConnector = false; // true if the station is a connector (is just connected to two other stations in a straight line, no bends, regardless of the number of lines)
};

class Map {
public:
    Map();
    int addStationConnection(int fromStationId, int toStationId, Direction direction, int lineId);
    int addStationConnection(int fromStationId, Direction direction, int lineId);
    int addLine(std::string name, Color color);
    
    bool canRemoveStationConnection(int fromStationId, int toStationId, int lineId);
    bool removeStationConnection(int fromStationId, int toStationId, int lineId);
    bool canRemoveStation(int id);
    bool removeStation(int id);

    std::vector<Station> getStations();
    Station& getStation(int id) { return stations[id]; }
    std::vector<Line>& getLines() { return lines; }
    Line& getLine(int id) { return lines[id]; }
    int getRootId() { return rootId; }

    float getMinX() { return minX; }
    float getMinY() { return minY; }
    float getMaxX() { return maxX; }
    float getMaxY() { return maxY; }

    std::string toJson();
    static Map fromJson(std::string json);

private:
    int addStation(float x, float y);
    bool canRemoveStationConnection(int fromStationId, int toStationId, int lineId, bool* isOnlyLine);
    int stationIdCounter = 0;
    std::unordered_map<int, Station> stations;
    std::vector<Line> lines;
    bool assignGridPositions();
    void checkITC(int id); //Check if the station is an interchange, terminal or connector
    int rootId = 0;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
};
