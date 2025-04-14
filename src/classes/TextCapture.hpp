#pragma once
#include <string>
#include "Keyboard.hpp"

class TextCapture {
public:
    TextCapture() = default;
    void update(Keyboard* keyboard); // Update the text capture state, to be called every frame after event handling
    std::string text;
    bool allowLineBreak = false;
};