set(CMAKE_CXX_STANDARD 14)
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
set(IOIG_FW CACHE INTERNAL "")
set(PICO_SDK_PATH CACHE INTERNAL "")
set(FW_DIR CACHE INTERNAL "")


#include(${CMAKE_SOURCE_DIR}/../cmake/find_libusb-1.0.cmake)
find_package(PkgConfig REQUIRED)
    pkg_check_modules(libusb REQUIRED libusb-1.0)

link_directories(${CMAKE_INSTALL_PREFIX})

set(SYS_LIBS usb-1.0 pthread atomic) #should go in the end, after all other libs


set(ARDUINO_CORE_DIR "${HOST_DIR}/APIs/arduino")
set(ARDUINO_CORE_API_DIR "${ARDUINO_CORE_DIR}/ArduinoCore-API/api")
if(NOT EXISTS ${ARDUINO_CORE_API_DIR})
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

add_subdirectory(${HOST_DIR}/APIs/arduino)


#==========================================================
# Build tests dir
#==========================================================

if(IOIG_TESTS)
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
    file(GLOB_RECURSE TEST_SRCS ${TESTS_DIR}/*.cpp) # Find all source files recursively
    
    # Iterate over each source file and create an executable
    foreach(SRC_FILE ${TEST_SRCS})
        # Get the filename without extension
        get_filename_component(BIN_NAME ${SRC_FILE} NAME_WE)

    
        # Create the executable
        add_executable(${BIN_NAME} ${SRC_FILE})
        target_link_libraries(${BIN_NAME} ${IOIG_HOST_LIB} ${SYS_LIBS} gtest_main)
    
        # Set the output directory for the executable
        set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)    
    endforeach()
endif()

#==========================================================
# Build examples IoIg dir
#==========================================================

set(EXAMPLES_DIR_IOIG ${PRJ_ROOT_DIR}/examples/ioig)
file(GLOB EXAMPLE_SRCS_IOIG ${EXAMPLES_DIR_IOIG}/*.cpp)

# Iterate over each source file and create an executable
foreach(SRC_FILE ${EXAMPLE_SRCS_IOIG})
    # Get the filename without extension
    get_filename_component(BIN_NAME ${SRC_FILE} NAME_WE)

    # Create the executable
    add_executable(${BIN_NAME} ${SRC_FILE})
    target_link_libraries(${BIN_NAME} ${IOIG_HOST_LIB} ${SYS_LIBS})  

    # Set the output directory for the executable
    set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/ioig)    
endforeach()

#==========================================================
# Build examples Arduino dir
#==========================================================

set(EXAMPLES_DIR_ARDUINO ${PRJ_ROOT_DIR}/examples/arduino)
file(GLOB_RECURSE EXAMPLE_SRCS_ARDUINO ${EXAMPLES_DIR_ARDUINO}/*.cpp)

set(ARDUINO_CORE_API_HEADER "${ARDUINO_CORE_DIR}/cores/ioig/Arduino.h")

# Iterate over each source file and create an executable
foreach(SRC_FILE ${EXAMPLE_SRCS_ARDUINO})
    # Get the filename without extension
    get_filename_component(BIN_NAME ${SRC_FILE} NAME_WE)


    # Create the executable
    add_executable(${BIN_NAME} ${SRC_FILE})

    target_include_directories(${BIN_NAME} PRIVATE  ${EXAMPLES_DIR_ARDUINO}                                                                                                        
                                                    "${ARDUINO_CORE_DIR}/cores/ioig" 
                                                    ${ARDUINO_CORE_API_DIR})    

    # Add precompiled header support
    target_precompile_headers(${BIN_NAME} PRIVATE  ${WIN_COMPAT_HEADER} ${ARDUINO_CORE_API_HEADER})

    target_link_libraries(${BIN_NAME} ArduinoCore  ${IOIG_HOST_LIB} ${SYS_LIBS}) 

    # Set the output directory for the executable
    set_target_properties(${BIN_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/arduino)    
endforeach()
