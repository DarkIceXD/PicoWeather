cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

set(PICO_BOARD "pico_w")

include(pico-sdk/pico_sdk_init.cmake)

project(weather)

pico_sdk_init()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(weather weather.c temperature_sensor/temperature_sensor.c drivers/ili9341.c drivers/ccs811.c)

pico_enable_stdio_uart(weather 0)
pico_enable_stdio_usb(weather 1)

target_link_libraries(weather pico_stdlib hardware_adc hardware_spi hardware_i2c lvgl::lvgl)

pico_add_extra_outputs(weather)

add_subdirectory(lvgl)
