cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Example: export PICO_SDK_PATH=~/pico/pico-sdk")
endif()

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(digital_logic_pico_tester C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

function(add_ic_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} pico_stdlib)
    pico_enable_stdio_usb(${target} 1)
    pico_enable_stdio_uart(${target} 0)
    pico_add_extra_outputs(${target})
endfunction()

add_ic_test(sn74hc00_test src/sn74hc00_test.c)
add_ic_test(sn74hc02_test src/sn74hc02_test.c)
add_ic_test(sn74hc04_test src/sn74hc04_test.c)
add_ic_test(sn74hc08_test src/sn74hc08_test.c)
add_ic_test(sn74hc14_test src/sn74hc14_test.c)
add_ic_test(sn74hc32_test src/sn74hc32_test.c)
add_ic_test(sn74hc138_test src/sn74hc138_test.c)
add_ic_test(sn74hc164_test src/sn74hc164_test.c)
add_ic_test(sn74hc165_test src/sn74hc165_test.c)
add_ic_test(sn74hc595_test src/sn74hc595_test.c)
