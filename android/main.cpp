#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_activity.h>
#include <android/asset_manager.h>

// شامل کردن SDL
#include "SDL.h"
#include "SDL_main.h"

#define LOG_TAG "KohkshSDL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" {

// تابع اصلی SDL
int main(int argc, char *argv[]) {
    LOGI("Kohksh SDL Application Starting...");
    
    // مقداردهی اولیه SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOGE("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return -1;
    }
    
    LOGI("SDL initialized successfully!");
    
    // ایجاد پنجره
    SDL_Window* window = SDL_CreateWindow(
        "Kohksh Android",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    
    if (!window) {
        LOGE("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    LOGI("Window created successfully!");
    
    // حلقه اصلی برنامه
    bool quit = false;
    SDL_Event e;
    
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        // اینجا کد رندر و منطق بازی شما قرار می‌گیرد
        SDL_Delay(16); // ~60 FPS
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    LOGI("Kohksh SDL Application Exited");
    return 0;
}

JNIEXPORT void JNICALL
Java_com_kohksh_MainActivity_startKohksh(JNIEnv *env, jobject thiz) {
    LOGI("Starting Kohksh SDL from Java...");
    
    // اجرای تابع اصلی در یک thread جدید
    std::thread main_thread([]() {
        main(0, nullptr);
    });
    main_thread.detach();
}

JNIEXPORT jstring JNICALL
Java_com_kohksh_MainActivity_getVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF("Kohksh SDL Android v1.0.0");
}

}
