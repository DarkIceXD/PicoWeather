cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

set(PICO_BOARD "pico_w")

include(pico-sdk/pico_sdk_init.cmake)

project(weather)

pico_sdk_init()

add_executable(weather weather.c storage/storage.c https_client/https_client.c jsmn/json.c min_max/min_max.c drivers/ili9341.c drivers/ft6x36.c drivers/bme280.c drivers/ccs811.c ui/ui.c)

target_compile_options(weather PRIVATE -Wall -Wextra -Wpedantic)

pico_enable_stdio_uart(weather 0)
pico_enable_stdio_usb(weather 1)

target_include_directories(weather PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(weather pico_stdlib pico_cyw43_arch_lwip_poll pico_lwip_mbedtls pico_mbedtls hardware_rtc hardware_adc hardware_spi hardware_i2c lvgl::lvgl)

pico_add_extra_outputs(weather)

add_subdirectory(lvgl)
