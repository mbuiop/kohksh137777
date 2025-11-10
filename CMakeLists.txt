cmake_minimum_required(VERSION 3.10)
project(kohksh)

set(CMAKE_CXX_STANDARD 11)

# پیدا کردن تمام فایل‌های منبع
file(GLOB_RECURSE SOURCE_FILES
    "*.cpp"
    "*.c"
    "src/*.cpp"
    "src/*.c"
)

if(SOURCE_FILES)
    message("Found source files: ${SOURCE_FILES}")
    add_executable(kohksh ${SOURCE_FILES})
else()
    message("No source files found, creating dummy executable")
    add_executable(kohksh dummy.cpp)
    file(WRITE dummy.cpp "int main() { return 0; }")
endif()
