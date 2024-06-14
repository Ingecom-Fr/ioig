cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(${PRJ_ROOT_DIR}/cmake/find_pico_sdk.cmake)
include(${PRJ_ROOT_DIR}/cmake/pico_sdk_import.cmake)

set(IOIG_FW ioig_fw)

add_compile_definitions(IOIG_FIRMWARE)

project(${IOIG_FW} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -O2
        )
 
pico_sdk_init() #pico_sdk_init() should be called after the project is created (and languages added)

include_directories(${PRJ_ROOT_DIR})
include_directories(${HOST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR})


set(IOIG_FW_CXX_SRCS "tasks/analog.cpp"  
                     "tasks/gpio.cpp"  
                     "tasks/i2c.cpp"  
                     "tasks/serial.cpp"
                     "tasks/spi.cpp"
                     "main.cpp"
    )

set(IOIG_FW_C_SRCS "usb_descriptors.c")


add_executable(${IOIG_FW} ${IOIG_FW_C_SRCS} ${IOIG_FW_CXX_SRCS})

target_include_directories(${IOIG_FW} PRIVATE ${CMAKE_CURRENT_LIST_DIR})


#uart output
#NOTE: Activate only for debug purpose
pico_enable_stdio_uart(${IOIG_FW} 1) 
    
#NOTE: Don't activate. it's tinyUSB device : check stdio_usb.h on pico-sdk
pico_enable_stdio_usb(${IOIG_FW} 0)   

target_link_libraries(${IOIG_FW} PRIVATE
    pico_stdlib
    pico_unique_id
    pico_multicore
    tinyusb_board
    tinyusb_device
    hardware_rtc
    hardware_adc
    hardware_gpio
    hardware_i2c
    hardware_spi
    hardware_pwm
    hardware_adc
    hardware_dma
    hardware_irq
    hardware_pio
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${IOIG_FW})


install(TARGETS ${IOIG_FW} DESTINATION fw)

