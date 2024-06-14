if(UNIX)
    set(USER_HOME "$ENV{HOME}")
elseif(WIN32)
    set(USER_HOME "$ENV{USERPROFILE}")
endif()

find_path(PICO_SDK_PATH
    NAMES src/rp2_common/hardware_gpio/gpio.c
    PATHS
        "$ENV{PICO_SDK_PATH}" 
        ${PRJ_ROOT_DIR}/fw/pico-sdk
        ${USER_HOME}/SDKs/pico-sdk
        ${USER_HOME}/pico-sdk
        ${USER_HOME}/workspace/pico-sdk
        ${USER_HOME}/develzone/SDKs/pico-sdk
        ${USER_HOME}/devel/pico-sdk
        /opt/pico-sdk
        /z/SDKs/pico-sdk
        "Z:\\SDKs\\pico-sdk"
        "C:\\pico-sdk"
        "C:\\Program Files\\pico-sdk"
        "C:\\Program Files\\pico-sdk"
)

if(PICO_SDK_PATH)
    message("Found PICO SDK at: ${PICO_SDK_PATH}")
else()
    message("PICO_SDK_PATH not found on search paths, attempting to download the SDK...")

    execute_process(
        COMMAND git clone https://github.com/raspberrypi/pico-sdk.git ${PRJ_ROOT_DIR}/fw/pico-sdk
        WORKING_DIRECTORY ${PRJ_ROOT_DIR}/fw
        RESULT_VARIABLE result
    )

    if(result)
        message(FATAL_ERROR "Failed to clone the pico-sdk repository")
    endif()

    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${PRJ_ROOT_DIR}/fw/pico-sdk
        RESULT_VARIABLE result
    )

    if(result)
        message(FATAL_ERROR "Failed to initialize submodules")
    endif()

    execute_process(
        COMMAND git checkout -b 0.16.0 0.16.0
        WORKING_DIRECTORY ${PRJ_ROOT_DIR}/fw/pico-sdk/lib/tinyusb
        RESULT_VARIABLE result
    )

    if(result)
        message(FATAL_ERROR "Failed to checkout tinyusb submodule")
    endif()

    execute_process(
        COMMAND git clean -df
        WORKING_DIRECTORY ${PRJ_ROOT_DIR}/fw/pico-sdk/lib/tinyusb
        RESULT_VARIABLE result
    )

    if(result)
        message(FATAL_ERROR "Failed to clean tinyusb submodule")
    endif()

    set(PICO_SDK_PATH "${PRJ_ROOT_DIR}/fw/pico-sdk")
    set(ENV{PICO_SDK_PATH} "${PICO_SDK_PATH}")

    message("Downloaded and set PICO_SDK_PATH to: ${PICO_SDK_PATH}")
endif()