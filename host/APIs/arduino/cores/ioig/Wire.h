#pragma once

#include "Arduino.h"
#include "HardwareI2C.h"
#include "Print.h"
#include <memory>

typedef void (*voidFuncPtrParamInt)(int);

namespace arduino
{

    class IoIgI2CImpl;

    class IoIgI2C : public HardwareI2C
    {
        friend class IoIgI2CImpl;

    public:
        IoIgI2C(int sda, int scl);        
        ~IoIgI2C();

        virtual void begin();
#ifndef DEVICE_I2CSLAVE
        virtual void __attribute__((error("I2C Slave mode is not supported"))) begin(uint8_t address);
#else
        virtual void begin(uint8_t address);
#endif
        virtual void end();

        virtual void setClock(uint32_t freq);

        virtual void beginTransmission(uint8_t address);
        virtual uint8_t endTransmission(bool stopBit);
        virtual uint8_t endTransmission(void);

        virtual size_t requestFrom(uint8_t address, size_t len, bool stopBit);
        virtual size_t requestFrom(uint8_t address, size_t len);

        virtual void onReceive(void (*)(int));
        virtual void onRequest(void (*)(void));

        virtual size_t write(uint8_t data);
        virtual size_t write(int data)
        {
            return write((uint8_t)data);
        };
        virtual size_t write(const uint8_t *data, int len);
        using Print::write;
        virtual int read();
        virtual int peek();
        virtual void flush();
        virtual int available();

    private:
        std::unique_ptr<IoIgI2CImpl> pimpl; ///< Pointer to implementation.
    };

}//namespace

#if I2C_INSTANCES > 0
extern arduino::IoIgI2C Wire;
#endif
#if I2C_INSTANCES > 1
extern arduino::IoIgI2C Wire1;
#endif
#if I2C_INSTANCES > 2
extern arduino::IoIgI2C Wire2;
#endif

typedef arduino::IoIgI2C TwoWire;