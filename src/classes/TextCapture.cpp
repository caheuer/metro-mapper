#include "TextCapture.hpp"
#include <SDL.h>

void TextCapture::update(Keyboard* keyboard) {
    // Update the text capture state, to be called every frame after event handling
    if (keyboard->isKeyPressed(SDLK_RETURN) && allowLineBreak) {
        text += '\n';
    } else if (keyboard->isKeyPressed(SDLK_BACKSPACE)) {
        if (!text.empty()) {
            text.pop_back();
        }
    } else {
        text += keyboard->getText();
    }
}