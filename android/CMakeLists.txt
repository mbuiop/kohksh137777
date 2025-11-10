cmake_minimum_required(VERSION 3.18.1)

project("kohksh")

add_library(
    kohksh_android
    SHARED
    main.cpp
)

find_library(
    log-lib
    log
)

target_link_libraries(
    kohksh_android
    ${log-lib}
)
