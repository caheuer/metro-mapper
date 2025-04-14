#include "Mouse.hpp"
#include <SDL.h>

void Mouse::update() {
    // Update the mouse state, to be called every frame before event handling
    deltaX = 0;
    deltaY = 0;
    scroll = 0;

    leftButtonPressed = false;
    leftButtonReleased = false;

    middleButtonPressed = false;
    middleButtonReleased = false;

    rightButtonPressed = false;
    rightButtonReleased = false;
}

void Mouse::mouseButtonDown(int button) {
    switch (button) {
        case SDL_BUTTON_LEFT:
            leftButtonDown = true;
            leftButtonPressed = true;
            break;
        case SDL_BUTTON_MIDDLE:
            middleButtonDown = true;
            middleButtonPressed = true;
            break;
        case SDL_BUTTON_RIGHT:
            rightButtonDown = true;
            rightButtonPressed = true;
            break;
    }
}

void Mouse::mouseButtonUp(int button) {
    switch (button) {
        case SDL_BUTTON_LEFT:
            leftButtonDown = false;
            leftButtonReleased = true;
            break;
        case SDL_BUTTON_MIDDLE:
            middleButtonDown = false;
            middleButtonReleased = true;
            break;
        case SDL_BUTTON_RIGHT:
            rightButtonDown = false;
            rightButtonReleased = true;
            break;
    }
}

void Mouse::mouseMove(int x, int y) {
    deltaX = x - this->x;
    deltaY = y - this->y;
    this->x = x;
    this->y = y;
}

void Mouse::mouseWheel(int y) {
    scroll = y;
}