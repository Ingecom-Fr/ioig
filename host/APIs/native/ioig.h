#pragma once

#include <cstdint>

#include "fw/device.h"
#include "fw/targets/rp2040/hw_defs.h"

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__linux__) || defined(__APPLE__)
#ifndef IOIG_HOST
#define IOIG_HOST 1
#endif
#endif

static constexpr int MAX_USB_DEVICES = 8;


#include "ioig_periph.h"
#include "analog.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "serial.h"




