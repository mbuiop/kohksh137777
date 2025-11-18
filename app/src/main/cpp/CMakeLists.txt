cmake_minimum_required(VERSION 3.18.1)

project("kohksh")

set(CMAKE_CXX_STANDARD 17)

find_library(log-lib log)
find_library(android-lib android)

# اضافه کردن SDL2
add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../../../jni/SDL2/libs/${ANDROID_ABI}/libSDL2.so)

add_library(kohksh SHARED
    main.cpp
)

target_include_directories(kohksh PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../../jni/SDL2/include
)

target_link_libraries(kohksh
    SDL2
    ${log-lib}
    ${android-lib}
)
