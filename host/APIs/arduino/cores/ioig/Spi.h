#pragma once

#include "Arduino.h"
#include "HardwareSPI.h"

namespace arduino
{

    class IoIgSpiImpl;

    class IoIgSpi : public SPIClass
    {
    public:
        IoIgSpi(int miso, int mosi, int sck) : IoIgSpi(miso,mosi,sck,-1,1000000,0) {}
        IoIgSpi(int miso, int mosi, int sck, int cs = -1, unsigned long freq_hz = 1000000, unsigned hw_instance = 0);
        ~IoIgSpi();
        virtual uint8_t transfer(uint8_t data);
        virtual uint16_t transfer16(uint16_t data);
        virtual void transfer(void *buf, size_t count);

        // Transaction Functions
        virtual void usingInterrupt(int interruptNumber);
        virtual void notUsingInterrupt(int interruptNumber);
        virtual void beginTransaction(SPISettings settings);
        virtual void endTransaction(void);

        // SPI Configuration methods
        virtual void attachInterrupt();
        virtual void detachInterrupt();

        virtual void begin();
        virtual void end();

    private:
        SPISettings settings = SPISettings(0, MSBFIRST, SPI_MODE0);
        int _miso;
        int _mosi;
        int _sck;
        std::unique_ptr<IoIgSpiImpl> pimpl; ///< Pointer to implementation.
    };

}

#if SPI_INSTANCIES > 0
extern arduino::IoIgSPI SPI;
#endif
#if SPI_INSTANCIES > 1
#ifdef SPI1
#undef SPI1
#endif
extern arduino::IoIgSPI SPI1;
#endif
