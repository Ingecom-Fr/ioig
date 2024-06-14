#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>

#include "ioig.h"


// Using a vector instead of std::map
// We've few GPIOs, let's get best performance with a std::vector
static std::vector<ioig::Gpio *> gpioVec;


static ioig::Gpio * getGpioObj(pin_size_t pinNumber) 
{
    if (gpioVec.empty()) 
    {   
        //init vector
        for (size_t i = 0; i < TARGET_PINS_COUNT; i++)        
        {
            gpioVec.push_back(nullptr);
        }
    }

    if (pinNumber >= TARGET_PINS_COUNT) 
    {
        return nullptr;
    }

    return gpioVec[pinNumber];
}


void pinMode(pin_size_t pinNumber, PinMode pinMode)
{

    auto gpio = getGpioObj(pinNumber);

    if ( gpio != nullptr ) 
    {
       delete gpio; //delete previous obj
    }

    switch (pinMode)
    {
    case PinMode::INPUT:
    case PinMode::OUTPUT_OPENDRAIN:
        gpio = new ioig::Gpio(pinNumber, ioig::Input);
        break;
    case PinMode::INPUT_PULLUP:
        gpio = new ioig::Gpio(pinNumber, ioig::Input, ioig::PullUp);
        break;
    case PinMode::INPUT_PULLDOWN:
        gpio = new ioig::Gpio(pinNumber, ioig::Input, ioig::PullDown);
        break;
    case PinMode::OUTPUT:
        gpio = new ioig::Gpio(pinNumber, ioig::Output);
        break;
    default:
        break;
    }

    gpioVec[pinNumber] = gpio;    
}

void digitalWrite(pin_size_t pinNumber, PinStatus status)
{
    auto gpio = getGpioObj(pinNumber);
    if ( gpio == nullptr ) 
    {
        gpio = new ioig::Gpio(pinNumber, ioig::Output);
        gpioVec[pinNumber] = gpio;
    }
    gpio->write(status);
}

PinStatus digitalRead(pin_size_t pinNumber)
{
    auto gpio = getGpioObj(pinNumber);
    if ( gpio == nullptr ) 
    {
        gpio = new ioig::Gpio(pinNumber, ioig::Input);
        gpioVec[pinNumber] = gpio;        
    }
    return static_cast<PinStatus>(gpio->read());
}


void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode)
{
    //TODO:
    (void)interruptNumber;
    (void)callback;
    (void)mode;
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param)
{
    //TODO:
    (void)interruptNumber;
    (void)callback;
    (void)mode;
    (void)param;

}

void detachInterrupt(pin_size_t interruptNumber)
{
    //TODO:
    (void)interruptNumber;
}