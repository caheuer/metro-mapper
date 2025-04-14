#include "Map.hpp"
#include "../constants.hpp"
#include <iostream>
#include <unordered_set>

Map::Map() {
    // Initialize the root station
    int id = addStation(0, 0);
    stations[id].name = "Main Station";
    stations[id].isInterchange = true;
    stationIdCounter = 1;
}

int Map::addStation(float x, float y) {
    std::string name = "Station " + std::to_string(stationIdCounter);
    stations[stationIdCounter] = {stationIdCounter, name, x, y};
    return stationIdCounter++;
}

int Map::addStationConnection(int fromStationId, int toStationId, Direction direction, int lineId) {
    std::unordered_map<int, Station> prevStations = stations;
    
    Station& fromStation = stations[fromStationId];
    Station& toStation = stations[toStationId];

    bool connectionExists = false;
    for (StationConnection& connection : fromStation.connections) {
        if (connection.to == toStationId && connection.dir == direction) {
            // Connection already exists, just add the line ID to the existing connection
            connection.lineIds.push_back(lineId);
            // We use std::lower_bound to keep lineIds sorted
            // auto it = std::lower_bound(connection.lineIds.begin(), connection.lineIds.end(), lineId);
            // connection.lineIds.insert(it, lineId);
            connectionExists = true;
            break;
        }
    }
    if (!connectionExists) {
        fromStation.connections.push_back({toStationId, direction, {lineId}});
    }

    connectionExists = false;
    for (StationConnection& connection : toStation.connections) {
        if (connection.to == fromStationId && connection.dir == opposite(direction)) {
            // Connection already exists, just add the line ID to the existing connection
            connection.lineIds.push_back(lineId);
            // We use std::lower_bound to keep lineIds sorted
            // auto it = std::lower_bound(connection.lineIds.begin(), connection.lineIds.end(), lineId);
            // connection.lineIds.insert(it, lineId);
            connectionExists = true;
            break;
        }
    }
    if (!connectionExists) {
        toStation.connections.push_back({fromStationId, opposite(direction), {lineId}});
    }

    // Check if stations are now an interchange or terminal or connector
    checkITC(fromStationId);
    checkITC(toStationId);

    // Calculate grid positions
    // If direction is diagonal assignGridPositions might fail. In that case, try straightening the direction
    if (!assignGridPositions()) {
        if (static_cast<int>(direction) % 2 == 1) {
            stations = prevStations;
            Direction newDirection;
            if (std::abs(stations[fromStationId].x - stations[toStationId].x) > std::abs(stations[fromStationId].y - stations[toStationId].y)) {
                if ((stations[toStationId].x - stations[fromStationId].x) > 0) {
                    newDirection = Direction::East;
                } else {
                    newDirection = Direction::West;
                }
            } else {
                if ((stations[toStationId].y - stations[fromStationId].y) > 0) {
                    newDirection = Direction::North;
                } else {
                    newDirection = Direction::South;
                }
            }
            return addStationConnection(fromStationId, toStationId, newDirection, lineId);
        }
        std::cout << "Error: Failed to assign grid positions." << std::endl;
        stations = prevStations;
        return -1;
    }

    // Return the ID of the new station
    return toStationId;
}

int Map::addStationConnection(int fromStationId, Direction direction, int lineId) {
    Station& fromStation = stations[fromStationId];
    auto [dx, dy] = offset(direction);
    int x = fromStation.x + dx * MIN_DIST;
    int y = fromStation.y + dy * MIN_DIST;

    int toStationId = addStation(x, y);
    return addStationConnection(fromStationId, toStationId, direction, lineId);
}

int Map::addLine(std::string name, Color color) {
    lines.push_back({name, color});
    return lines.size() - 1; // Return the index of the new line
}

bool Map::canRemoveStationConnection(int fromStationId, int toStationId, int lineId, bool* isOnlyLine) {
    Station& fromStation = stations[fromStationId];
    Station& toStation = stations[toStationId];

    // Check if the connection exists
    bool connectionExists = false;
    *isOnlyLine = false;
    for (StationConnection& connection : fromStation.connections) {
        if (connection.to != toStationId) {
            continue;
        }
        if (connection.lineIds.size() == 1 && connection.lineIds[0] == lineId) {
            *isOnlyLine = true;
        }
        for (int id : connection.lineIds) {
            if (id == lineId || lineId == -1) {
                connectionExists = true;
                break;
            }
        }
    }
    if (!connectionExists) {
        return false;
    }
    if (!*isOnlyLine) {
        // Can remove without problem as a different line will still support the connection
        return true;
    }

    // Check if the graph would split into multiple components after removing the connection
    std::unordered_set<int> visited;
    std::queue<int> queue;
    queue.push(fromStationId);
    visited.insert(fromStationId);

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        for (const StationConnection& connection : stations[current].connections) {
            if (current == fromStationId && connection.to == toStationId) {
                continue;
            }
            if (current == toStationId && connection.to == fromStationId) {
                continue;
            }
            if (visited.count(connection.to) == 0) {
                visited.insert(connection.to);
                queue.push(connection.to);
            }
        }
    }

    return visited.size() == stations.size(); // If we can still visit all stations, we can remove this connection
}

bool Map::canRemoveStationConnection(int fromStationId, int toStationId, int lineId) {
    bool isOnlyLine = false;
    return canRemoveStationConnection(fromStationId, toStationId, lineId, &isOnlyLine);
}

bool Map::removeStationConnection(int fromStationId, int toStationId, int lineId) {
    bool isOnlyLine = false;
    if (!canRemoveStationConnection(fromStationId, toStationId, lineId, &isOnlyLine)) {
        return false;
    }

    Station& fromStation = stations[fromStationId];
    Station& toStation = stations[toStationId];

    if (isOnlyLine) {
        // Remove the connection from both stations
        for (int i = 0; i < fromStation.connections.size(); i++) {
            if (fromStation.connections[i].to == toStationId) {
                fromStation.connections.erase(fromStation.connections.begin() + i);
                break;
            }
        }
        for (int i = 0; i < toStation.connections.size(); i++) {
            if (toStation.connections[i].to == fromStationId) {
                toStation.connections.erase(toStation.connections.begin() + i);
                break;
            }
        }
    } else {
        // Just remove the line ID from the connection
        for (StationConnection& connection : fromStation.connections) {
            if (connection.to == toStationId) {
                for (int i = 0; i < connection.lineIds.size(); i++) {
                    if (connection.lineIds[i] == lineId) {
                        connection.lineIds.erase(connection.lineIds.begin() + i);
                        break;
                    }
                }
                break;
            }
        }
        for (StationConnection& connection : toStation.connections) {
            if (connection.to == fromStationId) {
                for (int i = 0; i < connection.lineIds.size(); i++) {
                    if (connection.lineIds[i] == lineId) {
                        connection.lineIds.erase(connection.lineIds.begin() + i);
                        break;
                    }
                }
                break;
            }
        }
    }

    // Check if the stations are now an interchange, terminal or connector
    checkITC(fromStationId);
    checkITC(toStationId);

    // Reassign grid positions
    std::unordered_map prevStations = stations;
    if(!assignGridPositions()) {
        stations = prevStations;
    }

    return true;
}

bool Map::canRemoveStation(int id) {
    // Check if the graph would split into multiple components after removing the station
    int startId = rootId;
    if (id == startId) {
        if (stations[startId].connections.size() == 0) {
            //Root station is lone station on map
            return false;
        }
        startId = stations[rootId].connections[0].to;
    }

    std::unordered_set<int> visited;
    std::queue<int> queue;
    queue.push(startId);
    visited.insert(startId);

    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        for (const StationConnection& connection : stations[current].connections) {
            if (connection.to == id) {
                continue; // Skip the station we want to remove
            }
            if (visited.count(connection.to) == 0) {
                visited.insert(connection.to);
                queue.push(connection.to);
            }
        }
    }

    return visited.size() == stations.size() - 1; // If we can visit all other stations, we can remove this one
}

bool Map::removeStation(int id) {
    if (!canRemoveStation(id)) {
        return false;
    }

    std::vector<int> previousNeighborIds;

    // Remove the station from all connections
    for (auto& [i, station] : stations) {
        for (int j = 0; j < station.connections.size(); j++) {
            if (station.connections[j].to == id) {
                previousNeighborIds.push_back(i);
                station.connections.erase(station.connections.begin() + j);
                break;
            }
        }
    }

    // Remove the station itself
    for (auto [i, station] : stations) {
        if (station.id == id) {
            stations.erase(i);
            break;
        }
    }

    // Update root ID if necessary
    if (id == rootId) {
        rootId = stations.begin()->first;
    }

    // Check if previous neighbors are now interchange, terminal or connector
    for (int id : previousNeighborIds) {
        checkITC(id);
    }

    // Reassign grid positions
    std::unordered_map prevStations = stations;
    if(!assignGridPositions()) {
        stations = prevStations;
    }

    return true;
}

std::vector<Station> Map::getStations() {
    std::vector<Station> res;
    for (auto [id, station] : stations) {
        res.push_back(station);
    }
    return res;
}

bool Map::assignGridPositions() {
    int maxIterations = MAX_ITERATIONS * stations.size();

    int i = 0;
    std::queue<int> queue;
    std::unordered_set<int> inQueue;
    std::unordered_set<int> visited;

    queue.push(rootId);
    inQueue.insert(rootId);

    while (!queue.empty() && i++ < maxIterations) {
        int current = queue.front();

        Station& currentStation = stations[current];

        for (const StationConnection& connection : currentStation.connections) {
            Station& neighbor = stations[connection.to];

            auto [dx, dy] = offset(connection.dir);

            int stepX = dx * MIN_DIST;
            int stepY = dy * MIN_DIST;

            if (static_cast<int>(connection.dir) % 2 == 1) { // diagonal
                int dist = std::max(abs(stepX), abs(stepY));
                stepX = (stepX > 0 ? 1 : -1) * dist;
                stepY = (stepY > 0 ? 1 : -1) * dist;
            }

            int requiredX = currentStation.x + stepX;
            int requiredY = currentStation.y + stepY;

            // If neighbor is not in right direction or not far enough in the right direction
            bool changed = false;
            if ((dx == 0 && neighbor.x != requiredX) || (dx > 0 && neighbor.x < requiredX) || (dx < 0 && neighbor.x > requiredX)) {
                neighbor.x = requiredX;
                changed = true;
            }
            if ((dy == 0 && neighbor.y != requiredY) || (dy > 0 && neighbor.y < requiredY) || (dy < 0 && neighbor.y > requiredY)) {
                neighbor.y = requiredY;
                changed = true;
            }
            if (static_cast<int>(connection.dir) % 2 == 1) {
                //Connection is diagonal
                if (visited.count(connection.to) == 0 || visited.count(current) == 0) {
                    neighbor.x = requiredX;
                    neighbor.y = requiredY;
                    changed = true;
                }
            }

            if ((changed || visited.count(connection.to) == 0) && inQueue.count(connection.to) == 0) {
                queue.push(connection.to);
                inQueue.insert(connection.to);
            }
        }

        queue.pop();
        inQueue.erase(current);
        visited.insert(current);
    }

    if (i >= maxIterations) {
        return false;
    }

    // Next, we want to adjust the positions of the stations such that the distances between those in one section are equal
    i = 0;
    float cumulativeChange = 0.0f;
    do {
        cumulativeChange = 0.0f;
        for (auto [i, station] : stations) {
            for (const StationConnection& connection : station.connections) {
                Station& neighbor = stations[connection.to];

                // Neighbor is a connector if it has exactly two connections and both are in opposite directions
                // In this case, we "slide" the neighbor to the center of the current station and the other station
                if (neighbor.isConnector) {
                    Station& other = stations[neighbor.connections[0].to == i ? neighbor.connections[1].to : neighbor.connections[0].to];
                    float nx = (station.x + other.x) / 2;
                    float ny = (station.y + other.y) / 2;
                    if (std::abs(neighbor.x - nx) > EPSILON || std::abs(neighbor.y - ny) > EPSILON) {
                        neighbor.x = nx;
                        neighbor.y = ny;
                        cumulativeChange += std::abs(neighbor.x - nx) + std::abs(neighbor.y - ny);
                    }
                }
            }
        }
    } while (cumulativeChange > EPSILON && ++i < maxIterations);

    if (i >= maxIterations) {
        return false;
    }

    //Next, we assign directions for the station labels
    queue = std::queue<int>();
    inQueue.clear();
    visited.clear();
    std::unordered_map<int, Direction> preferredDirections;

    queue.push(rootId);
    inQueue.insert(rootId);
    while(!queue.empty()) {
        int current = queue.front();
        queue.pop();
        inQueue.erase(current);
        visited.insert(current);

        std::unordered_set<int> xDirs;
        std::unordered_set<int> yDirs;
        std::unordered_set<Direction> dirs;
        bool directionFound = false;
        Station& currentStation = stations[current];
        Direction preferredDir = Direction::None;
        if (currentStation.isConnector) {
            preferredDir = preferredDirections[current];
        }
        if (currentStation.isTerminal) {
            preferredDir = opposite(currentStation.connections[0].dir);
        }

        // Check interference with connected stations
        for (StationConnection& connection : currentStation.connections) {
            //Direct interference
            auto [dx, dy] = offset(connection.dir);
            if (dx != 0) {
                xDirs.insert(dx);
            }
            if (dy != 0) {
                yDirs.insert(dy);
            }
            dirs.insert(connection.dir);

            //Interference with labels
            if (visited.find(connection.to) == visited.end()) {
                //Station not yet visited, i.e. not yet assigned a label direction
                continue;
            }
            Station& neighbor = stations[connection.to];
            auto [dx2, dy2] = offset(neighbor.labelDir);
            for (int i = 0; i < 8; i++) {
                Direction d = static_cast<Direction>(i);
                auto [dx3, dy3] = offset(d);
                if (dx + dx2 == dx3 && dy + dy2 == dy3) {
                    dirs.insert(d);
                }
            }
        }

        // Check interference with other stations
        for (auto [i, station] : stations) {
            if (station.id == current) {
                continue;
            }
            for (int i = 0; i < 8; i+=2) {
                Direction d = static_cast<Direction>(i);
                auto [dx, dy] = offset(d);
                if (dx != 0) {
                    if (std::abs(currentStation.x + dx * MIN_DIST - station.x) < MIN_DIST && std::abs(station.y - currentStation.y) < MIN_DIST) {
                        xDirs.insert(dx);
                    }
                }
                if (dy != 0) {
                    if (std::abs(currentStation.y + dy * MIN_DIST - station.y) < MIN_DIST && std::abs(station.x - currentStation.x) < MIN_DIST) {
                        yDirs.insert(dy);
                    }
                }
            }
        }

        if (yDirs.find(-1) == yDirs.end()) {
            currentStation.labelDir = Direction::North;
            directionFound = true;
        }
        if (xDirs.find(-1) == xDirs.end() && (!directionFound || preferredDir == Direction::West)) {
            currentStation.labelDir = Direction::West;
            directionFound = true;
        }
        if (yDirs.find(1) == yDirs.end() && (!directionFound || preferredDir == Direction::South)) {
            currentStation.labelDir = Direction::South;
            directionFound = true;
        }
        if (xDirs.find(1) == xDirs.end() && (!directionFound || preferredDir == Direction::East)) {
            currentStation.labelDir = Direction::East;
            directionFound = true;
        }
        if (dirs.find(Direction::NorthEast) == dirs.end() && (!directionFound || preferredDir == Direction::NorthEast)) {
            currentStation.labelDir = Direction::NorthEast;
            directionFound = true;
        }
        if (dirs.find(Direction::NorthWest) == dirs.end() && (!directionFound || preferredDir == Direction::NorthWest)) {
            currentStation.labelDir = Direction::NorthWest;
            directionFound = true;
        }
        if (dirs.find(Direction::SouthEast) == dirs.end() && (!directionFound || preferredDir == Direction::SouthEast)) {
            currentStation.labelDir = Direction::SouthEast;
            directionFound = true;
        }
        if (dirs.find(Direction::SouthWest) == dirs.end() && (!directionFound || preferredDir == Direction::SouthWest)) {
            currentStation.labelDir = Direction::SouthWest;
            directionFound = true;
        }
        if (!directionFound) {
            currentStation.labelDir = Direction::North;
        }

        for (StationConnection& connection : currentStation.connections) {
            Station& neighbor = stations[connection.to];
            if (visited.count(connection.to) == 0 && inQueue.count(connection.to) == 0) {
                queue.push(connection.to);
                inQueue.insert(connection.to);
            }
            if (preferredDirections.find(neighbor.id) == preferredDirections.end()
                && static_cast<int>(currentStation.labelDir) % 2 == static_cast<int>(connection.dir) % 2) {
                preferredDirections[neighbor.id] = opposite(currentStation.labelDir);
            }
        }
    }

    //Next, we determine min and max x and y values
    minX = std::numeric_limits<float>::max();
    minY = std::numeric_limits<float>::max();
    maxX = std::numeric_limits<float>::min();
    maxY = std::numeric_limits<float>::min();
    for (auto [i, station] : stations) {
        if (station.x < minX) {
            minX = station.x;
        }
        if (station.y < minY) {
            minY = station.y;
        }
        if (station.x > maxX) {
            maxX = station.x;
        }
        if (station.y > maxY) {
            maxY = station.y;
        }
    }

    return true;
}

void Map::checkITC(int id) {
    Station& station = stations[id];
    station.isInterchange = station.connections.size() > 2 || station.connections.size() == 0;
    if (!station.isInterchange) {
        int l = -1;
        for (StationConnection& connection : station.connections) {
            for (int lineId : connection.lineIds) {
                if (l == -1) {
                    l = lineId;
                } else if (l != lineId) {
                    station.isInterchange = true;
                    break;
                }
            }
        }
    }
    station.isTerminal = station.connections.size() == 1;
    station.isConnector = station.connections.size() == 2 && station.connections[0].dir == opposite(station.connections[1].dir);
}

std::string Map::toJson() {
    nlohmann::json j;
    j["stations"] = nlohmann::json::array();
    for (auto& [id, station] : stations) {
        nlohmann::json stationJson;
        stationJson["id"] = id;
        stationJson["name"] = station.name.c_str();
        stationJson["x"] = station.x;
        stationJson["y"] = station.y;
        stationJson["labelDir"] = static_cast<int>(station.labelDir);
        stationJson["isInterchange"] = station.isInterchange;
        stationJson["isTerminal"] = station.isTerminal;
        stationJson["isConnector"] = station.isConnector;
        for (StationConnection& connection : station.connections) {
            nlohmann::json connectionJson;
            connectionJson["to"] = connection.to;
            connectionJson["dir"] = static_cast<int>(connection.dir);
            connectionJson["lineIds"] = connection.lineIds;
            stationJson["connections"].push_back(connectionJson);
        }
        j["stations"].push_back(stationJson);
    }
    j["lines"] = nlohmann::json::array();
    for (auto& line : lines) {
        nlohmann::json lineJson;
        lineJson["name"] = line.name.c_str();
        lineJson["color"] = {line.color.r, line.color.g, line.color.b};
        j["lines"].push_back(lineJson);
    }
    j["rootId"] = rootId;
    j["stationIdCounter"] = stationIdCounter;
    j["minX"] = minX;
    j["minY"] = minY;
    j["maxX"] = maxX;
    j["maxY"] = maxY;
    return j.dump();
}

Map Map::fromJson(std::string json) {
    Map map;
    nlohmann::json j = nlohmann::json::parse(json);
    map.stations.clear();
    for (auto& stationJson : j["stations"]) {
        int id = stationJson["id"];
        map.stations[id].id = id;
        map.stations[id].name = stationJson["name"].get<std::string>();
        map.stations[id].x = stationJson["x"];
        map.stations[id].y = stationJson["y"];
        map.stations[id].labelDir = static_cast<Direction>(stationJson["labelDir"]);
        map.stations[id].isInterchange = stationJson["isInterchange"];
        map.stations[id].isTerminal = stationJson["isTerminal"];
        map.stations[id].isConnector = stationJson["isConnector"];
        for (auto& connectionJson : stationJson["connections"]) {
            StationConnection connection;
            connection.to = connectionJson["to"];
            connection.dir = static_cast<Direction>(connectionJson["dir"]);
            for (auto& lineId : connectionJson["lineIds"]) {
                connection.lineIds.push_back(lineId);
            }
            map.stations[id].connections.push_back(connection);
        }
    }
    map.lines.clear();
    for (auto& lineJson : j["lines"]) {
        Line line;
        line.name = lineJson["name"].get<std::string>();
        line.color.r = lineJson["color"][0];
        line.color.g = lineJson["color"][1];
        line.color.b = lineJson["color"][2];
        map.lines.push_back(line);
    }
    map.rootId = j["rootId"];
    map.stationIdCounter = j["stationIdCounter"];
    map.minX = j["minX"];
    map.minY = j["minY"];
    map.maxX = j["maxX"];
    map.maxY = j["maxY"];
    return map;
}

