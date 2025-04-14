#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "classes/Keyboard.hpp"
#include "classes/Mouse.hpp"
#include "classes/Map.hpp"

enum Mode {
    SELECT,
    LINE,
    DELETE,
};

bool initMap(SDL_Renderer* renderer);
void renderMap(SDL_Renderer* renderer, int width, int height, Mouse* mouse, Keyboard* keyboard, TTF_Font* font);