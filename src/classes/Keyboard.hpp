#pragma once
#include <unordered_set>
#include <string>

class Keyboard {
public:
    Keyboard() = default;
    void update(); // Update the keyboard state, to be called every frame before event handling
    void keyDown(int key);
    void keyUp(int key);
    void textInput(const char* text);

    bool isKeyDown(int key) const { return keysDown.find(key) != keysDown.end(); }
    bool isKeyPressed(int key) const { return keysPressed.find(key) != keysPressed.end(); }
    bool isKeyReleased(int key) const { return keysReleased.find(key) != keysReleased.end(); }
    std::unordered_set<int> getKeysDown() const { return keysDown; }
    std::string getText();

private:
    std::unordered_set<int> keysDown; // All keys currently down
    std::unordered_set<int> keysPressed; // All keys pressed this frame
    std::unordered_set<int> keysReleased; // All keys released this frame
    std::string textBuffer; // Text input buffer
};