cmake_minimum_required(VERSION 3.16)
project(cc_scout VERSION 0.1.0 LANGUAGES C)

find_package(ALSA REQUIRED)

add_library(cc_scout STATIC src/cc_scout.c)
add_library(CCScout::core ALIAS cc_scout)
target_include_directories(cc_scout PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(cc_scout PUBLIC c_std_11)
target_link_libraries(cc_scout PRIVATE ALSA::ALSA)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_executable(cc-scout src/main.c)
    target_link_libraries(cc-scout PRIVATE CCScout::core)
endif()
