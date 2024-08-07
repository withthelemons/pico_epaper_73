cmake_minimum_required(VERSION 3.13)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
add_definitions(-DPICO_FLOAT_SUPPORT_ROM_V1=0 -DPICO_DOUBLE_SUPPORT_ROM_V1=0 -DPICO_NO_FPGA_CHECK=1)  # This board never shipped with V1 ROM (as far as I know)
include(pico_sdk_import.cmake)
if (DEFINED USE_DORMANT_SLEEP)
    include(pico_extras_import.cmake)
endif()
project(Pico_ePaper_73)
pico_sdk_init()

# generate an executable file
add_executable(epd main.c main.h)


# add a compilation subdirectory
add_subdirectory(lib/Config)
add_subdirectory(lib/e-Paper)
add_subdirectory(lib/RTC)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(lib/led)
add_subdirectory(lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)
add_subdirectory(examples)

# add a header directory
include_directories(examples)
include_directories(./lib/e-Paper)
include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/RTC)
include_directories(./lib/led)

# enable usb output, disable uart output
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    pico_enable_stdio_usb(epd 1)
    pico_enable_stdio_uart(epd 0)
else()
    pico_enable_stdio_usb(epd 1)
    pico_enable_stdio_uart(epd 0)
endif()

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(epd)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
target_link_libraries(epd FatFs_SPI pico_stdlib hardware_spi hardware_adc hardware_i2c)
if (DEFINED USE_DORMANT_SLEEP)
    target_link_libraries(hardware_rosc hardware_sleep)
endif()