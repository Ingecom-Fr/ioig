set(CMAKE_CXX_STANDARD 14) #ArduinoAPI has many isses on C++17

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


set(SOURCE_FILES "${SRCS_1}" "${SRCS_2}")

include_directories(${SRC_DIR1})
include_directories(${SRC_DIR2})


set(ARDUINO_API_LIB ArduinoCore)
add_library(${ARDUINO_API_LIB} STATIC ${SOURCE_FILES})
target_link_libraries(${ARDUINO_API_LIB} PRIVATE)

install(TARGETS ${ARDUINO_API_LIB} DESTINATION lib)
