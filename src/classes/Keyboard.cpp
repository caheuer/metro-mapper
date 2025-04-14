#include "Keyboard.hpp"
#include <SDL.h>

void Keyboard::update() {
    // Update the keyboard state, to be called every frame before event handling
    keysPressed.clear();
    keysReleased.clear();
}

void Keyboard::keyDown(int key) {
    keysDown.insert(key);
    keysPressed.insert(key);
}

void Keyboard::keyUp(int key) {
    keysDown.erase(key);
    keysReleased.insert(key);
}

void Keyboard::textInput(const char* text) {
    textBuffer += text;
}

std::string Keyboard::getText() {
    std::string text = textBuffer;
    textBuffer.clear(); // Clear the buffer after getting the text
    return text;
}