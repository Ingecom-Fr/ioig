cmake_minimum_required(VERSION 3.13)

project(ioig)

set(PRJ_COMMON_CXXFLAGS -Wall -Wextra -Wno-cast-function-type)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("IoIg Project Build Mode: Debug")
    add_compile_options(${PRJ_COMMON_CXXFLAGS})
    # Add debug-specific settings or configurations here
else()
    message("IoIg Project Build Mode: Release")
    add_compile_options(${PRJ_COMMON_CXXFLAGS} -O2)
endif()



set(PRJ_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Project root")
set(HOST_DIR "${PRJ_ROOT_DIR}/host" CACHE STRING "Host Lib Directory")
set(FW_DIR "${PRJ_ROOT_DIR}/fw" CACHE STRING "Firmware Directory")

#set the default installation directory 
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)


# ---------------------------------------------------------
# Firmware
# ---------------------------------------------------------
if(BUILD_IOIG_FW)
    set(PICO_BOARD ${PICO_BOARD} CACHE STRING "Pico board type")
    
    if(NOT DEFINED PICO_BOARD OR PICO_BOARD STREQUAL "")
        message("PICO board defaults to pico")
        set(PICO_BOARD pico)
    else()
        message("PICO board ${PICO_BOARD}")
    endif()
    
    include(ExternalProject)
    
    
    # We use an external project to get distinct build environment for the FW
    ExternalProject_Add(    
            ioig_fw        
            BUILD_ALWAYS TRUE
            SOURCE_DIR ${FW_DIR}
            BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/fw
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}  # Set the directory where the external project will be built
            INSTALL_COMMAND ""    
            DOWNLOAD_COMMAND ""
            GIT_REPOSITORY "" 
            GIT_TAG ""
            CMAKE_ARGS -DPICO_BOARD=${PICO_BOARD} -DPRJ_ROOT_DIR=${PRJ_ROOT_DIR} -DHOST_DIR=${HOST_DIR}
    )
endif()



# ---------------------------------------------------------
# Host Library
# ---------------------------------------------------------
add_subdirectory(${HOST_DIR}) #check CMakeLists.txt on host directory


