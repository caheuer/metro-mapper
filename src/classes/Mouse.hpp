#pragma once
#include <utility>

class Mouse {
public:
    Mouse() = default;
    void update(); // Update the mouse state, to be called every frame before event handling
    void mouseButtonDown(int button);
    void mouseButtonUp(int button);
    void mouseMove(int x, int y);
    void mouseWheel(int y);

    std::pair<int, int> getPosition() const { return {x, y}; }
    std::pair<int, int> getDelta() const { return {deltaX, deltaY}; }
    int getScroll() const { return scroll; }

    bool isLeftButtonDown() const { return leftButtonDown; }
    bool isLeftButtonPressed() const { return leftButtonPressed; }
    bool isLeftButtonReleased() const { return leftButtonReleased; }

    bool isMiddleButtonDown() const { return middleButtonDown; }
    bool isMiddleButtonPressed() const { return middleButtonPressed; }
    bool isMiddleButtonReleased() const { return middleButtonReleased; }

    bool isRightButtonDown() const { return rightButtonDown; }
    bool isRightButtonPressed() const { return rightButtonPressed; }
    bool isRightButtonReleased() const { return rightButtonReleased; }

private:
    int x = 0, y = 0; // Current mouse position
    int deltaX = 0, deltaY = 0; // Change in mouse position
    int scroll = 0; // Scroll amount

    bool leftButtonDown = false;
    bool leftButtonPressed = false;
    bool leftButtonReleased = false;

    bool middleButtonDown = false;
    bool middleButtonPressed = false;
    bool middleButtonReleased = false;

    bool rightButtonDown = false;
    bool rightButtonPressed = false;
    bool rightButtonReleased = false;
};