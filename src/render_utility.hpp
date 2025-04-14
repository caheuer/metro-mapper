#pragma once
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include "classes/Mouse.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void thickRectangle(SDL_Renderer* renderer, int x, int y, int w, int h, int thickness, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (int i = 0; i < thickness; i++) {
        rectangleRGBA(renderer, x + i, y + i, x + w - i, y + h - i, r, g, b, a);
    }
}

void drawTooltip(SDL_Renderer* renderer, Mouse* mouse, TTF_Font* font, std::string text) {
    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        SDL_Log("Text render error: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_Rect dstRect = {
        static_cast<int>(mouse->getPosition().first + 10),
        static_cast<int>(mouse->getPosition().second + 10),
        static_cast<int>(surface->w * 12 / FONT_SIZE),
        static_cast<int>(surface->h * 12 / FONT_SIZE)
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 127);
    SDL_RenderFillRect(renderer, &dstRect); // Fill the background for better visibility
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

std::vector<uint8_t> renderToPNG(SDL_Renderer* renderer, int width, int height, std::function<void()> renderFunc) {
    std::vector<uint8_t> output;

    SDL_Texture* target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, width, height);
    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, target);

    // Clear the screen to avoid artifacts (workaround for Emscripten)
#ifdef __EMSCRIPTEN__
    SDL_SetRenderTarget(renderer, oldTarget);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, target);
#endif

    // Render
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    renderFunc();
    SDL_RenderPresent(renderer);

    // Read the pixels
    std::vector<uint8_t> pixels(width * height * 4);
    if (!SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ABGR8888, pixels.data(), width * 4)) {
        std::cerr << "RenderReadPixels failed: " << SDL_GetError() << std::endl;
    }

    //Encode as PNG
    stbi_write_png_to_func(
        [](void* context, void* data, int size) {
            std::vector<uint8_t>* output = static_cast<std::vector<uint8_t>*>(context);
            output->insert(output->end(), static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size);
        },
        &output,
        width,
        height,
        4,
        pixels.data(),
        width * 4
    );

    //Cleanup
    SDL_SetRenderTarget(renderer, oldTarget);
    SDL_DestroyTexture(target);

    return output;
}

