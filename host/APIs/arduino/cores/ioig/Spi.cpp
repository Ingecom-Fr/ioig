#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "Spi.h"
#include "ioig_private.h"

#if SPI_INSTANCIES > 0
arduino::IoIgSpi SPI(GP4, GP3, GP2, -1, 4000000, 0);
#endif
#if SPI_INSTANCIES > 1
arduino::IoIgSpi SPI1(GP12, GP11, GP10, -1, 4000000, 1);
#endif

using namespace arduino;

class arduino::IoIgSpiImpl
{
public:
    IoIgSpiImpl(IoIgSpi &parent) : _parent(parent) {}
    ~IoIgSpiImpl() {};

    IoIgSpi &_parent;
    ioig::Spi *spiDev = nullptr;
    int miso;
    int mosi;
    int sck;
    SPISettings settings = SPISettings(0, MSBFIRST, SPI_MODE0);
};

IoIgSpi::IoIgSpi(int miso, int mosi, int sck, int cs, unsigned long freq_hz, unsigned hw_instance)
{
    pimpl = std::make_unique<IoIgSpiImpl>(*this);
    pimpl->spiDev = new ioig::Spi(sck, mosi, miso, cs, freq_hz, hw_instance);
    pimpl->miso = miso;
    pimpl->mosi = mosi;
    pimpl->sck = sck;
}

IoIgSpi::~IoIgSpi()
{
    end();
}

uint8_t IoIgSpi::transfer(uint8_t data)
{
    uint8_t ret;
    ret = pimpl->spiDev->write((const uint8_t *)&data, 1);
    return ret;
}

uint16_t IoIgSpi::transfer16(uint16_t data)
{
    union
    {
        uint16_t val;
        struct
        {
            uint8_t lsb;
            uint8_t msb;
        };
    } t;
    t.val = data;

    if (settings.getBitOrder() == LSBFIRST)
    {
        t.lsb = transfer(t.lsb);
        t.msb = transfer(t.msb);
    }
    else
    {
        t.msb = transfer(t.msb);
        t.lsb = transfer(t.lsb);
    }
    return t.val;
}

void IoIgSpi::transfer(void *buf, size_t count)
{
    pimpl->spiDev->write((const uint8_t *)buf, count);
}

void IoIgSpi::beginTransaction(SPISettings settings)
{
    if (settings != pimpl->settings)
    {
        switch (settings.getDataMode())
        {
        case SPI_MODE0:
            pimpl->spiDev->format(8, 0, 0, settings.getBitOrder());
            break;
        case SPI_MODE1:
            pimpl->spiDev->format(8, 0, 1, settings.getBitOrder());
            break;
        case SPI_MODE2:
            pimpl->spiDev->format(8, 1, 0, settings.getBitOrder());
            break;
        case SPI_MODE3:
            pimpl->spiDev->format(8, 1, 1, settings.getBitOrder());
            break;
        default:
            pimpl->spiDev->format(8, 0, 0, settings.getBitOrder());
            break;
        }
        pimpl->spiDev->set_freq(settings.getClockFreq());
        pimpl->settings = settings;
    }
}

void IoIgSpi::endTransaction(void)
{
    // all operations are blocking, don't need to wait here
}

void IoIgSpi::usingInterrupt(int interruptNumber)
{
    (void)interruptNumber;
    // https://reference.arduino.cc/reference/en/language/functions/communication/spi/usinginterrupt/
}

void IoIgSpi::notUsingInterrupt(int interruptNumber)
{
    (void)interruptNumber;
}

void IoIgSpi::attachInterrupt()
{
}

void IoIgSpi::detachInterrupt()
{
}

void IoIgSpi::begin()
{

    if (pimpl->spiDev == nullptr)
    {
        pimpl->spiDev = new ioig::Spi(pimpl->sck, pimpl->mosi, pimpl->miso);
    }
}

void IoIgSpi::end()
{
    if (pimpl->spiDev != nullptr)
    {
        delete pimpl->spiDev;
        pimpl->spiDev = nullptr;
    }
}
