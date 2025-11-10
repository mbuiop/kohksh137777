#include "SDL.h"

// پیاده‌سازی ساده توابع SDL برای اندروید

int SDL_Init(Uint32 flags) {
    return 0;
}

void SDL_Quit(void) {
}

const char* SDL_GetError(void) {
    return "No error";
}

SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags) {
    return (SDL_Window*)1; // مقدار dummy
}

void SDL_DestroyWindow(SDL_Window* window) {
}

int SDL_PollEvent(SDL_Event* event) {
    return 0;
}

void SDL_Delay(Uint32 ms) {
}
