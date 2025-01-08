set(CMAKE_CXX_STANDARD 14)

set(PRJ_COMMON_CXXFLAGS -Wall -Wextra -Wno-cast-function-type)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("Arduino-API Build Mode: Debug")
    add_compile_options(${PRJ_COMMON_CXXFLAGS})
    # Add debug-specific settings or configurations here
else()
    message("Arduino-API Build Mode: Release")
    add_compile_options(${PRJ_COMMON_CXXFLAGS} -O2)
endif()


add_compile_definitions(HOST)


set(SRC_DIR1 ${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api)
set(SRC_DIR2 ${CMAKE_CURRENT_SOURCE_DIR}/cores/ioig)


file(GLOB SRCS_1 "${SRC_DIR1}/*.cpp") #all cpp from original API
file(GLOB SRCS_2 "${SRC_DIR2}/*.cpp") #all local implementation files

if(WIN32) 
set(WIN_COMPAT_HEADER "${ARDUINO_CORE_DIR}/cores/ioig/Win_Compat.h" CACHE INTERNAL "Windows compatibility header")
# Find MSYS include directory (with multiple search paths and error handling)
find_path(MSYS_INCLUDE_DIR
    NAMES string.h
    PATHS
        "C:/msys64/ucrt64/include"
        "C:/msys32/ucrt64/include" # Add more potential paths
        "/usr/include" # Linux/WSL path
        "/usr/local/include"
        "C:/Program Files/mingw64/include" # MinGW path
        "C:/Program Files (x86)/mingw64/include" # MinGW path
    PATH_SUFFIXES # Search in subdirectories as well
        "include"
        "mingw32/include"
        "mingw64/include"
)

if(MSYS_INCLUDE_DIR)
    message(STATUS "Found MSYS includes at: ${MSYS_INCLUDE_DIR}")    
    set(MSYS_INCLUDE_DIR ${MSYS_INCLUDE_DIR} CACHE PATH "Path to MSYS includes")
    # Normalize MSYS_INCLUDE_DIR to Windows format
    file(TO_NATIVE_PATH "${MSYS_INCLUDE_DIR}" MSYS_INCLUDE_DIR_WINDOWS)
    add_compile_definitions(-DMSYS_INCLUDE_DIR="${MSYS_INCLUDE_DIR_WINDOWS}")
    set(MSYS_INCLUDE_DIR ${MSYS_INCLUDE_DIR_WINDOWS} CACHE PATH "Path to MSYS includes") #updates
else()
    message(FATAL_ERROR "Could not find MSYS includes. Please install MSYS or set MSYS_INCLUDE_DIR manually.")
    # Or, if you want a warning instead of a fatal error:
    # message(WARNING "Could not find MSYS includes. Some functionality might be unavailable.")
endif()
endif()

set(SOURCE_FILES "${SRCS_1}" "${SRCS_2}")

include_directories(${SRC_DIR1})
include_directories(${SRC_DIR2})


set(ARDUINO_API_LIB ArduinoCore)
add_library(${ARDUINO_API_LIB} STATIC ${SOURCE_FILES})
target_link_libraries(${ARDUINO_API_LIB} PRIVATE)
target_precompile_headers(${ARDUINO_API_LIB} PRIVATE ${WIN_COMPAT_HEADER})

install(TARGETS ${ARDUINO_API_LIB} DESTINATION lib)
