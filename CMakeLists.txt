cmake_minimum_required(VERSION 3.16)
project(cc_scout VERSION 0.1.0 LANGUAGES C)

find_package(ALSA REQUIRED)

add_executable(cc-scout src/main.c)
target_compile_features(cc-scout PRIVATE c_std_11)
target_link_libraries(cc-scout PRIVATE ALSA::ALSA)
