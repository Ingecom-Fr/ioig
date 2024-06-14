#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <memory>

#include "ioig.h"
#include "ioig_protocol.h"
#include "ioig_usb.h"

#include "analog.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"


#define DBG_MSG_T(interval_s, ...)                                                       \
    {                                                                                    \
        auto nowTs = std::chrono::high_resolution_clock::now();                          \
        static auto oldTs = nowTs;                                                       \
                                                                                         \
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(nowTs - oldTs); \
        if (duration.count() >= interval_s)                                              \
        {                                                                                \
            oldTs = nowTs;                                                               \
            fprintf(stderr, __VA_ARGS__);                                                \
        }                                                                                \
    }


#define DBG_CALLBACK_T(interval_s, cbk)                                                  \
    {                                                                                    \
        auto nowTs = std::chrono::high_resolution_clock::now();                          \
        static auto oldTs = nowTs;                                                       \
                                                                                         \
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(nowTs - oldTs); \
        if (duration.count() >= interval_s)                                              \
        {                                                                                \
            oldTs = nowTs;                                                               \
            cbk                                                                          \
        }                                                                                \
    }


#define LOG_ERR(tag, ...)                             \
    std::cerr << "\033[1;31m" << tag <<  " Error : "; \
    fprintf(stderr, __VA_ARGS__);                     \
    std::cerr << "\033[0m" << std::endl;    

#define LOG_WARN(tag, ...)                              \
    std::cerr << "\033[1;33m" << tag <<  " Warning : "; \
    fprintf(stderr, __VA_ARGS__);                       \
    std::cerr << "\033[0m" << std::endl;    

#define LIBUSB_ERR(code) libusb_strerror(static_cast<libusb_error>(code))    





