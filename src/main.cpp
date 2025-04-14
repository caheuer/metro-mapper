#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "classes/Map.hpp"
#include "classes/Mouse.hpp"
#include "classes/Keyboard.hpp"
#include "classes/TextCapture.hpp"
#include "constants.hpp"
#include "map_render.hpp"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
Mouse mouse;
Keyboard keyboard;
int width = WINDOW_WIDTH;
int height = WINDOW_HEIGHT;
int framesUntilStandby = 5;
bool running = true;
SDL_Event event;

void mainLoop() {
    mouse.update();
    keyboard.update();

    while (SDL_PollEvent(&event)) {
        // Reset standby frames on any event
        framesUntilStandby = 5;

        switch(event.type) {
            case SDL_QUIT:
                running = false;
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse.mouseButtonDown(event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                mouse.mouseButtonUp(event.button.button);
                break;
            case SDL_MOUSEMOTION:
                mouse.mouseMove(event.button.x, event.button.y);
                break;
            case SDL_MOUSEWHEEL:
                mouse.mouseWheel(event.wheel.y);
                break;
            case SDL_KEYDOWN:
                keyboard.keyDown(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                keyboard.keyUp(event.key.keysym.sym);
                break;
            case SDL_TEXTINPUT:
                keyboard.textInput(event.text.text);
                break;
        }
    }

    // Only render if there have been events
    // When there are no events the rendered frame will not change anyways, so we can save resources
    if (framesUntilStandby > 0) {
        framesUntilStandby--;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        renderMap(renderer, width, height, &mouse, &keyboard, font);

        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char* argv[]) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        Module.preinitializedWebGLContextAttributes = {
            preserveDrawingBuffer: true
        };
    });
#endif

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Metro Mapper",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    TTF_Init();
    font = TTF_OpenFont("assets/fonts/opensans.ttf", FONT_SIZE);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }

    IMG_Init(IMG_INIT_PNG);
    
    if (!initMap(renderer)) {
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (running) {
        mainLoop();
    }
#endif

    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
