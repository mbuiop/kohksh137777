#include <jni.h>
#include <string>
#include <android/log.h>

// شامل کردن SDL ساده شده
#include "SDL.h"

#define LOG_TAG "KohkshAndroid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// تابع اصلی برنامه
void kohksh_main() {
    LOGI("🚀 Kohksh Android Application Started!");
    
    // مقداردهی اولیه SDL
    if (SDL_Init(0) < 0) {
        LOGE("SDL init failed: %s", SDL_GetError());
        return;
    }
    
    LOGI("✅ SDL initialized successfully");
    
    // ایجاد پنجره
    SDL_Window* window = SDL_CreateWindow("Kohksh", 0, 0, 800, 600, 0);
    if (!window) {
        LOGE("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    
    LOGI("✅ Window created successfully");
    
    // حلقه اصلی برنامه
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == 0x100) { // SDL_QUIT
                running = false;
            }
        }
        
        // اینجا کد اصلی برنامه شما قرار می‌گیرد
        LOGI("Kohksh is running...");
        
        SDL_Delay(1000); // 1 ثانیه تأخیر
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    LOGI("👋 Kohksh Android Application Exited");
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_kohksh_MainActivity_startKohksh(JNIEnv *env, jobject thiz) {
    LOGI("🎯 Starting Kohksh from Java...");
    kohksh_main();
}

JNIEXPORT jstring JNICALL
Java_com_kohksh_MainActivity_getVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF("Kohksh Android v1.0.0 - Built with SDL");
}

JNIEXPORT void JNICALL
Java_com_kohksh_MainActivity_nativeInit(JNIEnv *env, jobject thiz) {
    LOGI("🔧 Native initialization called");
}

}
