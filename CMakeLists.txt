cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(
    xmas-house
    LANGUAGES C CXX ASM
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


include(TM1637-pico/PicoTM1637.cmake)

add_executable(
    xmas-house
    xmas.c
)

target_link_libraries(xmas-house
    PicoTM1637
    pico_stdlib
    hardware_pwm
)

pico_add_extra_outputs(xmas-house)
