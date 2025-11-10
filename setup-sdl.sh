#!/bin/bash
echo "📥 Setting up SDL for Android..."

# ایجاد دایرکتوری‌های لازم
mkdir -p android/include
mkdir -p android/src

# دانلود فایل‌های ضروری SDL برای اندروید
echo "🔽 Downloading SDL Android files..."

# ایجاد فایل‌های ضروری SDL (ساده شده)
cat > android/include/SDL.h << 'EOF'
#ifndef SDL_h_
#define SDL_h_

#include "SDL_main.h"
#include "SDL_stdinc.h"
#include "SDL_audio.h"
#include "SDL_video.h"
#include "SDL_events.h"
#include "SDL_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SDL_INIT_TIMER          0x00000001
#define SDL_INIT_AUDIO          0x00000010
#define SDL_INIT_VIDEO          0x00000020
#define SDL_INIT_JOYSTICK       0x00000200
#define SDL_INIT_HAPTIC         0x00001000
#define SDL_INIT_GAMECONTROLLER 0x00002000
#define SDL_INIT_EVENTS         0x00004000
#define SDL_INIT_SENSOR         0x00008000
#define SDL_INIT_NOPARACHUTE    0x00100000
#define SDL_INIT_EVERYTHING     (SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_SENSOR)

int SDL_Init(Uint32 flags);
void SDL_Quit(void);
const char* SDL_GetError(void);

// Window
typedef struct SDL_Window SDL_Window;
SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window* window);

// Events
typedef union SDL_Event SDL_Event;
int SDL_PollEvent(SDL_Event* event);
void SDL_Delay(Uint32 ms);

#ifdef __cplusplus
}
#endif

#endif /* SDL_h_ */
EOF

# ایجاد فایل‌های ساده شده دیگر SDL
cat > android/include/SDL_main.h << 'EOF'
#ifndef SDL_main_h_
#define SDL_main_h_

#define main SDL_main

int main(int argc, char *argv[]);

#endif /* SDL_main_h_ */
EOF

echo "✅ SDL setup completed!"
