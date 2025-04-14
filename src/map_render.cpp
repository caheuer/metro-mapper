#include "map_render.hpp"
#include "constants.hpp"
#include "classes/Map.hpp"
#include "classes/TextCapture.hpp"
#include "direction.hpp"
#include "render_utility.hpp"
#include "filesystem.hpp"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream> 
#include <array>

SDL_Texture* menuTexture = nullptr;
SDL_Texture* interchangeTexture = nullptr;

TextCapture textCapture;

Map map;
int offsetX = WINDOW_WIDTH / 2;
int offsetY = WINDOW_HEIGHT / 2;
bool moving = false;
float zoom = 1.0f;

Mode mode = SELECT;
int selectedStationId = -1;
int selectedLineId = 0;
bool ignoreClick = false;

int getHoveredStationId(Mouse* mouse) {
    std::pair<int, int> mousePos = mouse->getPosition();
    SDL_Point mousePoint = {mousePos.first, mousePos.second};
    std::vector<Station> stations = map.getStations();
    for (Station& station : stations) {
        int width = 1;
        int height = 1;
        for (StationConnection& connection : station.connections) {
            int lineCount = connection.lineIds.size();
            auto [dx, dy] = offset(connection.dir);
            if (dx != 0 && lineCount > height) {
                height = lineCount;
            }
            if (dy != 0 && lineCount > width) {
                width = lineCount;
            }
        }
        width++;
        height++;

        int x = station.x * CELL_WIDTH * zoom + offsetX;
        int y = station.y * CELL_HEIGHT * zoom + offsetY;
        SDL_Rect rect = {
            static_cast<int>(x - width * LINE_THICKNESS * zoom / 2),
            static_cast<int>(y - height * LINE_THICKNESS * zoom / 2),
            static_cast<int>(width * LINE_THICKNESS * zoom),
            static_cast<int>(height * LINE_THICKNESS * zoom)
        };
        if (SDL_PointInRect(&mousePoint, &rect)) {
            return station.id;
        }
    }
    return -1; // No station hovered
}

void renderMainMap(SDL_Renderer* renderer, TTF_Font* font) {
    // Draw connections
    for (Station& station : map.getStations()) {
        int x1 = station.x * CELL_WIDTH * zoom + offsetX;
        int y1 = station.y * CELL_HEIGHT * zoom + offsetY;
        for (StationConnection& connection : station.connections) {
            if (static_cast<int>(connection.dir) > 3) {
                continue; // Skip connections in the opposite direction
            }
            Station& other = map.getStation(connection.to);
            int x2 = other.x * CELL_WIDTH * zoom + offsetX;
            int y2 = other.y * CELL_HEIGHT * zoom + offsetY;
            int lineCount = connection.lineIds.size();
            for (int i = 0; i < lineCount; i++) {
                Line& line = map.getLine(connection.lineIds[i]);
                auto [nx, ny] = offsetNormed(rotateClockwise(connection.dir, 2));
                int dx = static_cast<int>(nx + nx * (i - (lineCount - 1) * 0.5f) * LINE_THICKNESS * zoom);
                int dy = static_cast<int>(ny + ny * (i - (lineCount - 1) * 0.5f) * LINE_THICKNESS * zoom);
                thickLineRGBA(renderer,
                    x1 + dx, y1 + dy, x2 + dx, y2 + dy, LINE_THICKNESS * zoom,
                    line.color.r, line.color.g, line.color.b, 255);
            }
        }
    }

    // Draw stations
    for (Station& station : map.getStations()) {
        int x = station.x * CELL_WIDTH * zoom + offsetX;
        int y = station.y * CELL_HEIGHT * zoom + offsetY;
        auto [dx, dy] = offset(station.labelDir);
        if (station.isInterchange || station.connections.size() == 0) {
            int TILE_SIZE = 128;
            int width = 1;
            int height = 1;
            for (StationConnection& connection : station.connections) {
                int lineCount = connection.lineIds.size();
                auto [dx, dy] = offset(connection.dir);
                if (dx != 0 && lineCount > width) {
                    height = lineCount;
                }
                if (dy != 0 && lineCount > height) {
                    width = lineCount;
                }
            }
            width++;
            height++;

            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    double angle = 0;
                    //Centers
                    SDL_Rect srcRect = {TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE};
                    if (i == 0 && j == 0) {
                        //Top left corner
                        srcRect.x = 0;
                        angle = 0;
                    } else if (i == width - 1 && j == height - 1) {
                        //Bottom right corner
                        srcRect.x = 0;
                        angle = 180;
                    } else if (i == width - 1 && j == 0) {
                        //Top right corner
                        srcRect.x = 0;
                        angle = 90;
                    } else if (i == 0 && j == height - 1) {
                        //Bottom left corner
                        srcRect.x = 0;
                        angle = 270;
                    } else if (i == 0) {
                        //Left edge
                        srcRect.x = TILE_SIZE;
                        angle = 270;
                    } else if (i == width - 1) {
                        //Right edge
                        srcRect.x = TILE_SIZE;
                        angle = 90;
                    } else if (j == 0) {
                        //Top edge
                        srcRect.x = TILE_SIZE;
                        angle = 0;
                    } else if (j == height - 1) {
                        //Bottom edge
                        srcRect.x = TILE_SIZE;
                        angle = 180;
                    }
                    SDL_Rect dstRect = {
                        static_cast<int>(x + i * LINE_THICKNESS * zoom - LINE_THICKNESS * width * zoom / 2),
                        static_cast<int>(y + j * LINE_THICKNESS * zoom - LINE_THICKNESS * height * zoom / 2),
                        // +1 due to rounding errors
                        static_cast<int>(LINE_THICKNESS * zoom + 1),
                        static_cast<int>(LINE_THICKNESS * zoom + 1)
                    };
                    SDL_RenderCopyEx(renderer, interchangeTexture, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
                }
            }
        } else if (station.isTerminal) {
            Direction dir = rotateClockwise(station.connections[0].dir, 2);
            auto [dx, dy] = offset(dir);
            int x1 = x - dx * 10 * zoom;
            int y1 = y - dy * 10 * zoom;
            int x2 = x + dx * 10 * zoom;
            int y2 = y + dy * 10 * zoom;
            Line& line = map.getLine(station.connections[0].lineIds[0]);
            thickLineRGBA(renderer,
                x1, y1, x2, y2, LINE_THICKNESS * 0.6f * zoom,
                line.color.r, line.color.g, line.color.b, 255);
        } else {
            Line& line = map.getLine(station.connections[0].lineIds[0]);
            int x2 = x + dx * 10 * zoom;
            int y2 = y + dy * 10 * zoom;
            thickLineRGBA(renderer,
                x, y, x2, y2, LINE_THICKNESS * 0.6f * zoom,
                line.color.r, line.color.g, line.color.b, 255);
        }

        // Draw station name
        std::string name = station.name;
        if (name == "") {
            name = "Unnamed Station";
        }
        SDL_Color color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, name.c_str(), color, 1.5 * CELL_WIDTH * FONT_SIZE / 12);
        if (!surface) {
            SDL_Log("Text render error: %s", TTF_GetError());
            return;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_Rect dstRect = {
            static_cast<int>(x - surface->w / 2 * zoom * 12 / FONT_SIZE + dx * (15 * zoom + surface->w / 2 * zoom * 12 / FONT_SIZE)),
            static_cast<int>(y - surface->h / 2 * zoom * 12 / FONT_SIZE + dy * (15 * zoom + surface->h / 2 * zoom * 12 / FONT_SIZE)),
            static_cast<int>(surface->w * zoom * 12 / FONT_SIZE),
            static_cast<int>(surface->h * zoom * 12 / FONT_SIZE)
        };
        SDL_FreeSurface(surface);
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
}

void renderNameInput(SDL_Renderer* renderer, TTF_Font* font, TextCapture& textCapture, int height) {
    int padding = 10;

    SDL_Color color = {0, 0, 0, 255};

    std::string text = "Type to change name: " + textCapture.text;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        SDL_Log("Text render error: %s", TTF_GetError());
        return;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_Rect dstRect = {
        padding,
        height - surface->h * 32 / FONT_SIZE - padding,
        surface->w * 32 / FONT_SIZE,
        surface->h * 32 / FONT_SIZE
    }; //bottom left corner

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

std::pair<int, int> renderLineSelection(SDL_Renderer* renderer, TTF_Font* font, int height, SDL_Rect& selectableRect) {
    int padding = 10;

    // Label
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, "Select line:", color);
    if (!surface) {
        SDL_Log("Text render error: %s", TTF_GetError());
        return {0, 0};
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_Rect dstRect = {
        padding,
        height - surface->h * 32 / FONT_SIZE - padding,
        surface->w * 32 / FONT_SIZE,
        surface->h * 32 / FONT_SIZE
    }; //bottom left corner
    int labelWidth = surface->w * 32 / FONT_SIZE;

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);

    // Lines
    std::vector<Line>& lines = map.getLines();
    for (int i = 0; i < lines.size(); i++) {
        Line& line = lines[i];
        SDL_Rect dstRect = {
            2 * padding + labelWidth + i * (50 + padding),
            height - 50 - padding,
            50,
            50
        };
        SDL_SetRenderDrawColor(renderer, line.color.r, line.color.g, line.color.b, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        // Draw black outline if selected
        if (i == selectedLineId) {
            thickRectangle(renderer, dstRect.x, dstRect.y, dstRect.w, dstRect.h, 5, 0, 0, 0, 255);
        }
    }

    // Set selectableRect to the area of the button row
    selectableRect = {2 * padding + labelWidth, height - 50 - padding, static_cast<int>(lines.size()) * (50 + padding), 50 + 2 * padding};

    // Return the rect of selectable button row
    return {labelWidth + 2 * padding + lines.size() * (50 + padding), 50 + 2 * padding};
}

std::pair<int, int> renderMenu(SDL_Renderer* renderer, TTF_Font* font) {
    int padding = 10;
    int w = 0, h = 0;
    SDL_QueryTexture(menuTexture, nullptr, nullptr, &w, &h);
    SDL_Rect dstRect = {padding, padding, w, h};

    std::array<std::string, 6> menuOptions = {"Select", "Lines", "Delete", "Save", "Load", "Export"};
    int textH = 0;
    for(int i = 0; i < menuOptions.size(); i++) {
        SDL_Color color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, menuOptions[i].c_str(), color);
        if (!surface) {
            SDL_Log("Text render error: %s", TTF_GetError());
            return {0, 0};
        }

        // Create texture from surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_Rect textRect = {
            padding + i * w / static_cast<int>(menuOptions.size()) + (w / static_cast<int>(menuOptions.size()) - surface->w * 12 / FONT_SIZE) / 2,
            padding + 5 + h,
            surface->w * 12 / FONT_SIZE,
            surface->h * 12 / FONT_SIZE
        };
        if (textH < surface->h * 12 / FONT_SIZE) {
            textH = surface->h * 12 / FONT_SIZE;
        }
        SDL_FreeSurface(surface);

        // Draw a background rectangle for better visibility
        SDL_SetRenderDrawColor(renderer, 223, 223, 223, 191);
        SDL_Rect dstRect = {
            padding + i * w / static_cast<int>(menuOptions.size()),
            padding,
            w / static_cast<int>(menuOptions.size()),
            h + 5 + textH + 5
        };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &dstRect);


        // Draw a rectangle around the selected option
        if (i == static_cast<int>(mode)) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &dstRect);
        }

        // Draw the text
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_DestroyTexture(texture);
    }

    // Draw the menu texture
    SDL_RenderCopy(renderer, menuTexture, nullptr, &dstRect);

    // Return the width and height of rendered menu
    return {w + padding * 2, h + 5 + textH + 5 + padding * 2};
}

bool initMap(SDL_Renderer* renderer) {
    menuTexture = IMG_LoadTexture(renderer, "assets/images/menu.png");
    if (!menuTexture) {
        std::cerr << "Failed to load menu texture: " << IMG_GetError() << std::endl;
        return false;
    }

    interchangeTexture = IMG_LoadTexture(renderer, "assets/images/interchange.png");
    if (!interchangeTexture) {
        std::cerr << "Failed to load interchange texture: " << IMG_GetError() << std::endl;
        return false;
    }

    //Predefined lines
    int l1 = map.addLine("Red Line", {255, 0, 0});
    int l2 = map.addLine("Green Line", {0, 255, 0});
    int l3 = map.addLine("Blue Line", {0, 0, 255});
    map.addLine("Yellow Line", {255, 255, 0});
    map.addLine("Purple Line", {255, 0, 255});
    map.addLine("Cyan Line", {0, 255, 255});
    map.addLine("Orange Line", {255, 165, 0});
    map.addLine("Pink Line", {255, 192, 203});
    map.addLine("Brown Line", {165, 42, 42});
    map.addLine("Gray Line", {128, 128, 128});

    return true;
}

void renderMap(SDL_Renderer* renderer, int width, int height, Mouse* mouse, Keyboard* keyboard, TTF_Font* font) {
    // Draw main map
    renderMainMap(renderer, font);

    ignoreClick = false;

    // Move map
    if (mouse->isLeftButtonDown() || mouse->isMiddleButtonDown()) {
        std::pair<int, int> delta = mouse->getDelta();
        if (std::abs(delta.first) + std::abs(delta.second) > 1) {
            moving = true;
        }
        if (moving) {
            offsetX += delta.first;
            offsetY += delta.second;
        }
    } else if (mouse->isLeftButtonReleased() && moving) {
        moving = false;
        ignoreClick = true;
    }

    // Zoom map in/out relative to mouse position
    if (mouse->getScroll() != 0) {
        auto [mx, my] = mouse->getPosition();

        // Convert mouse position to world coordinates
        float worldX = (mx - offsetX) / zoom;
        float worldY = (my - offsetY) / zoom;

        float zoomFactor = (mouse->getScroll() > 0) ? ZOOM_STEP : (1.0f / ZOOM_STEP);
        float newZoom = zoom * zoomFactor;
        newZoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, newZoom));

        // Adjust offset to zoom centered on mouse
        offsetX -= (worldX * newZoom - worldX * zoom);
        offsetY -= (worldY * newZoom - worldY * zoom);
        zoom = newZoom;
    }

    // Unselect
    if (keyboard->isKeyPressed(SDLK_ESCAPE) || mouse->isRightButtonReleased()) {
        selectedStationId = -1;
        textCapture.text = "";
    }

    // Draw menu
    auto [menuWidth, menuHeight] = renderMenu(renderer, font);
    
    // Handle menu selection
    if (mouse->isLeftButtonReleased() && mouse->getPosition().first < menuWidth && mouse->getPosition().second < menuHeight && !ignoreClick) {
        ignoreClick = true;
        int option = mouse->getPosition().first / 64;
        if (option >= 0 && option < 3) {
            mode = static_cast<Mode>(option);
        } else if (option == 3) {
            saveToFile("map.metromap", map.toJson());
        } else if (option == 4) {
            std::string data;
            bool success = loadFromFile(".metromap", &data);
            if (success) {
                map = Map::fromJson(data);
            }
        } else if (option == 5) {
            int prevOffsetX = offsetX;
            int prevOffsetY = offsetY;
            int prevZoom = zoom;

            zoom = 1.0f;
            offsetX = -static_cast<int>(map.getMinX() * CELL_WIDTH) + EXPORT_PADDING;
            offsetY = -static_cast<int>(map.getMinY() * CELL_HEIGHT) + EXPORT_PADDING;
            int width = static_cast<int>((map.getMaxX() - map.getMinX()) * CELL_WIDTH) + 2 * EXPORT_PADDING;
            int height = static_cast<int>((map.getMaxY() - map.getMinY()) * CELL_HEIGHT) + 2 * EXPORT_PADDING;

            std::vector<uint8_t> image = renderToPNG(renderer, width, height, [&]() {
                renderMainMap(renderer, font);
            });
            saveToFile("map.png", image);

            offsetX = prevOffsetX;
            offsetY = prevOffsetY;
            zoom = prevZoom;
        }
    }

    // SELECT mode
    if (mode == SELECT && selectedStationId >= 0) {
        // Selection marker
        Station& selectedStation = map.getStation(selectedStationId);
        int x = selectedStation.x * CELL_WIDTH * zoom + offsetX;
        int y = selectedStation.y * CELL_HEIGHT * zoom + offsetY;
        filledCircleRGBA(renderer, x, y, 15 * zoom, 255, 165, 0, 127); // Orange circle
        
        // Name input
        textCapture.update(keyboard);
        map.getStation(selectedStationId).name = textCapture.text;
        renderNameInput(renderer, font, textCapture, height);
    }

    // LINE mode
    if (mode == LINE) {
        // Draw line selection
        SDL_Rect selectableRect;
        renderLineSelection(renderer, font, height, selectableRect);

        // Handle line selection
        SDL_Point mousePoint = {mouse->getPosition().first, mouse->getPosition().second};
        if (mouse->isLeftButtonReleased() && SDL_PointInRect(&mousePoint, &selectableRect) && !ignoreClick) {
            ignoreClick = true;
            int lineIndex = (mouse->getPosition().first - selectableRect.x) / (50 + 10);
            if (lineIndex >= 0 && lineIndex < map.getLines().size()) {
                selectedLineId = lineIndex;
            }
        }

        // Line extension preview
        if (selectedStationId >= 0 && selectedLineId >= 0) {
            Station& selectedStation = map.getStation(selectedStationId);
            int x1 = selectedStation.x * CELL_WIDTH * zoom + offsetX;
            int y1 = selectedStation.y * CELL_HEIGHT * zoom + offsetY;
            auto [x2, y2] = mouse->getPosition();
            int hoveredStationId = getHoveredStationId(mouse);
            bool isConnectionLegal = true;
            bool isConnectionExisting = false;
            
            //Check if hovering a station
            if (hoveredStationId >= 0) {
                //Hovering a station, so we draw a line to it
                Station& hoveredStation = map.getStation(hoveredStationId);
                x2 = hoveredStation.x * CELL_WIDTH * zoom + offsetX;
                y2 = hoveredStation.y * CELL_HEIGHT * zoom + offsetY;

                //Cannot connect to itself
                if (hoveredStationId == selectedStationId) {
                    isConnectionLegal = false;
                }
            }

            //Calculate which direction is closest to the mouse/selected station
            Direction dir = Direction::North;
            float closestDistanceSquared = std::numeric_limits<float>::max();
            for (int i = 0; i < 8; i++) {
                // If shift is pressed, ignore diagonal directions
                if (keyboard->isKeyDown(SDLK_LSHIFT) || keyboard->isKeyDown(SDLK_RSHIFT)) {
                    if (i % 2 != 0) {
                        continue;
                    }
                }

                Direction d = static_cast<Direction>(i);
                auto [dx, dy] = offset(d);
                dx = x1 + dx * CELL_WIDTH * zoom;
                dy = y1 + dy * CELL_HEIGHT * zoom;
                float distanceSquared = std::pow(x2 - dx, 2) + std::pow(y2 - dy, 2);
                if (distanceSquared < closestDistanceSquared) {
                    closestDistanceSquared = distanceSquared;
                    dir = d;
                }
            }
            
            //Set x2, y2 to correct position if not hovering a station
            if (hoveredStationId == -1) {
                x2 = x1 + offset(dir).first * CELL_WIDTH * zoom;
                y2 = y1 + offset(dir).second * CELL_HEIGHT * zoom;
            }

            //Check if at new location is already a station
            for (Station& station : map.getStations()) {
                if (station.id == selectedStationId) {
                    continue;
                }
                int x = station.x * CELL_WIDTH * zoom + offsetX;
                int y = station.y * CELL_HEIGHT * zoom + offsetY;
                SDL_Rect rect = {
                    static_cast<int>(x - CELL_WIDTH * 0.2 * zoom),
                    static_cast<int>(y - CELL_HEIGHT * 0.2 * zoom),
                    static_cast<int>(CELL_WIDTH * 0.4 * zoom),
                    static_cast<int>(CELL_HEIGHT * 0.4 * zoom)
                };
                SDL_Point point = {x2, y2};
                if (SDL_PointInRect(&point, &rect)) {
                    hoveredStationId = station.id;
                    x2 = x;
                    y2 = y;
                    break;
                }
            }

            //Check if connection with selected line in direction is already present
            for (StationConnection& connection : selectedStation.connections) {
                if (connection.dir == dir) {
                    for (int lineId : connection.lineIds) {
                        if (lineId == selectedLineId) {
                            isConnectionExisting = true; // Connection already exists
                            isConnectionLegal = selectedStation.isTerminal || map.canRemoveStationConnection(selectedStationId, connection.to, selectedLineId);
                            break;
                        }
                    }
                }
            }

            //Draw line preview
            Line& line = map.getLine(selectedLineId);
            thickLineRGBA(renderer,
                x1, y1, x2, y2, LINE_THICKNESS * zoom,
                line.color.r, line.color.g, line.color.b, 127);
            filledCircleRGBA(renderer, x2, y2, LINE_THICKNESS * zoom, line.color.r, line.color.g, line.color.b, 127);

            //Draw a tooltip
            std::string text = "";
            if (isConnectionExisting && !isConnectionLegal) {
                text = "Cannot delete connection";
            } else if (isConnectionExisting) {
                text = "Delete connection";
            } else if (!isConnectionLegal) {
                text = "Invalid connection";
            } else if (hoveredStationId >= 0) {
                text = "Connect to " + map.getStation(hoveredStationId).name;
            } else {
                text = "Create new station";
            }
            drawTooltip(renderer, mouse, font, text);

            //Handle click
            if (mouse->isLeftButtonReleased() && isConnectionLegal && !ignoreClick) {
                ignoreClick = true;
                if (isConnectionExisting) {
                    if (selectedStation.isTerminal && !map.canRemoveStationConnection(selectedStationId, hoveredStationId, selectedLineId)) {
                        map.removeStation(selectedStationId);
                    } else {
                        map.removeStationConnection(selectedStationId, hoveredStationId, selectedLineId);
                    }
                    selectedStationId = hoveredStationId;
                } else if (hoveredStationId >= 0) {
                    selectedStationId = map.addStationConnection(selectedStationId, hoveredStationId, dir, selectedLineId);
                } else {
                    selectedStationId = map.addStationConnection(selectedStationId, dir, selectedLineId);
                }
            }
        }
    }

    // DELETE mode
    if (mode == DELETE) {
        int hoveredStationId = getHoveredStationId(mouse);
        bool canDelete = map.canRemoveStation(hoveredStationId);

        if (hoveredStationId >= 0) {
            //Draw a tooltip
            std::string text = "";
            if (!canDelete) {
                text = "Cannot delete station " + map.getStation(hoveredStationId).name;
            } else {
                text = "Delete station " + map.getStation(hoveredStationId).name;
            }
            drawTooltip(renderer, mouse, font, text);

            //Handle click
            if (mouse->isLeftButtonReleased() && canDelete && !ignoreClick) {
                ignoreClick = true;
                map.removeStation(hoveredStationId);
                selectedStationId = -1;
                textCapture.text = "";
            }
        }
    }

    // Station selection
    if (mouse->isLeftButtonReleased() && !ignoreClick) {
        int hoveredStationId = getHoveredStationId(mouse);
        if (hoveredStationId == selectedStationId || hoveredStationId == -1) {
            //Deselect the station if it is already selected or clciked into white space
            selectedStationId = -1;
            textCapture.text = "";
        } else {
            selectedStationId = hoveredStationId;
            textCapture.text = map.getStation(selectedStationId).name;
        }
    }
}