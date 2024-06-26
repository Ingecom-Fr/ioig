set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(PRJ_COMMON_CXXFLAGS -Wall -Wextra -Wno-cast-function-type)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("IoIg Host API Build Mode: Debug")
    add_compile_options(${PRJ_COMMON_CXXFLAGS})
    # Add debug-specific settings or configurations here
else()
    message("IoIg Host API Build Mode: Release")
    add_compile_options(${PRJ_COMMON_CXXFLAGS} -O2)
endif()


include(ExternalProject)
include(FetchContent)

add_compile_definitions(IOIG_HOST)

#these variables are not used here, cache them
set(BUILD_IOIG_FW CACHE INTERNAL "")
set(PICO_SDK_PATH CACHE INTERNAL "")
set(FW_DIR CACHE INTERNAL "")


#include(${CMAKE_SOURCE_DIR}/../cmake/find_libusb-1.0.cmake)
find_package(PkgConfig REQUIRED)
    pkg_check_modules(libusb REQUIRED libusb-1.0)

link_directories(${CMAKE_INSTALL_PREFIX})

set(SYS_LIBS usb-1.0 pthread atomic) #should go in the end, after all other libs


if(NOT EXISTS "${HOST_DIR}/APIs/arduino/ArduinoCore-API/api")
    #Download submodules
    message(STATUS "Downloading submodules...")
    execute_process(
      COMMAND git submodule update --init --recursive
      RESULT_VARIABLE result
      )
    
    if(result)
      message(FATAL_ERROR "Failed to update submodules")
    endif()
endif()


#==========================================================
# ioig Host Lib 
#==========================================================

set(IOIG_HOST_LIB ioig)
set(SRC_DIR ${HOST_DIR})

include_directories("${PRJ_ROOT_DIR}")
include_directories("${SRC_DIR}")
include_directories("${SRC_DIR}/APIs/native")


set(HOST_SRCS "${SRC_DIR}/ioig_private.cpp" 
              "${SRC_DIR}/ioig_usb.cpp"  
              "${SRC_DIR}/APIs/native/analog.cpp"  
              "${SRC_DIR}/APIs/native/gpio.cpp"  
              "${SRC_DIR}/APIs/native/i2c.cpp"  
              "${SRC_DIR}/APIs/native/serial.cpp"  
              "${SRC_DIR}/APIs/native/spi.cpp"
    )

set(HOST_INCS "${SRC_DIR}/APIs/native/analog.h"  
              "${SRC_DIR}/APIs/native/gpio.h"  
              "${SRC_DIR}/APIs/native/i2c.h"  
              "${SRC_DIR}/APIs/native/ioig_periph.h"
              "${SRC_DIR}/APIs/native/ioig.h"
              "${SRC_DIR}/APIs/native/serial.h"  
              "${SRC_DIR}/APIs/native/spi.h"
)

    

add_library(${IOIG_HOST_LIB} STATIC ${HOST_SRCS})
install(TARGETS ${IOIG_HOST_LIB} DESTINATION lib)
install(FILES ${HOST_INCS} DESTINATION include)

add_custom_command(
    TARGET ${IOIG_HOST_LIB} 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build . --target install
    COMMENT "Running IOIG install after build"
)


#==========================================================
# ArduinoCore API 
#==========================================================

if(NOT WIN32) #TODO: fix windows build for arduino code

add_subdirectory(${HOST_DIR}/APIs/arduino)

endif()

#==========================================================
# Build tests dir
#==========================================================

if(BUILD_IOIG_TESTS)
    #===========================================
    #Google Test Framework
    #===========================================
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    #===========================================

    set(TESTS_DIR ${PRJ_ROOT_DIR}/tests)
    file(GLOB TEST_SRCS ${TESTS_DIR}/*.cpp)
    
    # Iterate over each source file and create an executable
    foreach(SRC_FILE ${TEST_SRCS})
        # Get the filename without extension
        get_filename_component(BIN_NAME ${SRC_FILE} NAME_WE)
    
        # Create the executable
        add_executable(${BIN_NAME} ${SRC_FILE})
        target_link_libraries(${BIN_NAME} ${IOIG_HOST_LIB} ${ARDUINO_API_LIB} ${SYS_LIBS} gtest_main)
    
        # Set the output directory for the executable
        set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests)    
    endforeach()
endif()

#==========================================================
# Build examples dir
#==========================================================

set(EXAMPLES_DIR ${PRJ_ROOT_DIR}/examples)
file(GLOB EXAMPLE_SRCS ${EXAMPLES_DIR}/*.cpp)

# Iterate over each source file and create an executable
foreach(SRC_FILE ${EXAMPLE_SRCS})
    # Get the filename without extension
    get_filename_component(BIN_NAME ${SRC_FILE} NAME_WE)

    # Create the executable
    add_executable(${BIN_NAME} ${SRC_FILE})
    target_link_libraries(${BIN_NAME} ${IOIG_HOST_LIB} ${ARDUINO_API_LIB} ${SYS_LIBS})  

    # Set the output directory for the executable
    set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY examples)    
endforeach()