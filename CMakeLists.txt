cmake_minimum_required(VERSION 3.12)
project(kohksh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# پیدا کردن وابستگی‌ها
find_package(PkgConfig REQUIRED)

# پیدا کردن SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

# پیدا کردن OpenGL
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

# پیدا کردن OpenAL
pkg_check_modules(OPENAL REQUIRED openal)
pkg_check_modules(VORBIS REQUIRED vorbis vorbisfile)
pkg_check_modules(FLAC REQUIRED flac)

# فایل‌های منبع
file(GLOB_RECURSE SOURCE_FILES
    "*.cpp"
    "*.c"
    "src/*.cpp"
    "src/*.c"
    "Engine/*.cpp"
    "Engine/*.c"
)

if(NOT SOURCE_FILES)
    message(WARNING "No source files found, creating minimal example")
    set(SOURCE_FILES minimal_example.cpp)
    file(WRITE minimal_example.cpp "
#include <iostream>
#include <SDL2/SDL.h>

int main() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << \"SDL init failed: \" << SDL_GetError() << std::endl;
        return -1;
    }
    
    SDL_Window* window = SDL_CreateWindow(\"Kohksh\", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    
    if(!window) {
        std::cerr << \"Window creation failed: \" << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    
    std::cout << \"Kohksh started successfully!\" << std::endl;
    
    SDL_Delay(2000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
")
endif()

# ایجاد executable
add_executable(kohksh ${SOURCE_FILES})

# لینک کتابخانه‌ها
target_link_libraries(kohksh 
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENAL_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${FLAC_LIBRARIES}
    pthread
    dl
)

# اضافه کردن include directories
target_include_directories(kohksh PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
)
