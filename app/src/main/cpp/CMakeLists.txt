cmake_minimum_required(VERSION 3.22.1)
project(nexcore)

add_library(nexcore SHARED
    jni_bridge.cc
    nexcore/src/core.cc
    nexcore/src/audit.cc
    nexcore/src/filesystem.cc
    nexcore/src/hash.cc
    nexcore/src/json.cc
)

target_include_directories(nexcore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/nexcore/include
)

find_library(log-lib log)
target_link_libraries(nexcore ${log-lib})
