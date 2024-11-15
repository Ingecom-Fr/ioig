#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "ioig.h"


namespace WiringDigital 
{
    /* 
       Using a vector to store GPIOs
       Each vector index represents a GPIO
       We have few GPIOs, let's get best performance
     */

    static std::vector<ioig::Gpio *> gpioVec;
    static std::vector<voidFuncPtr> intrCallbackVec;
    static std::vector<voidFuncPtrParam> intrCallbackParamVec;
    static std::mutex mutex;
};

#define CHECK_PIN(pinNumber, action) if (pinNumber < 0 || pinNumber >= TARGET_PINS_COUNT) action;

static void intrHandler(int pin, const uint32_t events, void * arg)
{
    (void)events;    
    (void)arg;

    auto cbk = WiringDigital::intrCallbackVec[pin];
    if (cbk != nullptr) 
    {
        cbk();        
    }
 
}

static void intrHandlerParam(int pin, const uint32_t events, void * arg)
{
    (void)events;    
    
    auto cbkParam = WiringDigital::intrCallbackParamVec[pin];
    if (cbkParam != nullptr) 
    {
        cbkParam(arg);
    }

};


static ioig::Gpio * getGpioObj(pin_size_t pinNumber) 
{    
    std::lock_guard<std::mutex> lock(WiringDigital::mutex);
    if (WiringDigital::gpioVec.empty()) 
    {   
        //init vectors
        for (size_t i = 0; i < TARGET_PINS_COUNT; i++)        
        {
            WiringDigital::gpioVec.push_back(nullptr);
        }
    }

    return WiringDigital::gpioVec[pinNumber];
}


void pinMode(pin_size_t pinNumber, PinMode pinMode)
{     
    CHECK_PIN(pinNumber, return);

    auto gpio = getGpioObj(pinNumber);

    if ( gpio != nullptr ) 
    {
       delete gpio; //delete previous obj
    }

    switch (pinMode)
    {
    case PinMode::INPUT:
        gpio = new ioig::Gpio(pinNumber, ioig::Input, ioig::PullNone);
        break;
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

    std::lock_guard<std::mutex> lock(WiringDigital::mutex);
    WiringDigital::gpioVec[pinNumber] = gpio;    
}

void digitalWrite(pin_size_t pinNumber, PinStatus status)
{   
    CHECK_PIN(pinNumber, return);

    auto gpio = getGpioObj(pinNumber);        

    if ( gpio == nullptr ) 
    {        
        gpio = new ioig::Gpio(pinNumber, ioig::Output);
        std::lock_guard<std::mutex> lock(WiringDigital::mutex);
        WiringDigital::gpioVec[pinNumber] = gpio;
    }
    gpio->write(status);
}

PinStatus digitalRead(pin_size_t pinNumber)
{
    CHECK_PIN(pinNumber, return LOW);

    auto gpio = getGpioObj(pinNumber);    

    if ( gpio == nullptr ) 
    {
        gpio = new ioig::Gpio(pinNumber, ioig::Input);
        std::lock_guard<std::mutex> lock(WiringDigital::mutex);
        WiringDigital::gpioVec[pinNumber] = gpio;        
    }
    return static_cast<PinStatus>(gpio->read());
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode)
{
    CHECK_PIN(interruptNumber, return);

    auto gpio = getGpioObj(interruptNumber);
    
    if (gpio == nullptr)
    {
        gpio = new ioig::Gpio(interruptNumber, ioig::Input);
        std::lock_guard<std::mutex> lock(WiringDigital::mutex);
        WiringDigital::gpioVec[interruptNumber] = gpio;
    }

    std::lock_guard<std::mutex> lock(WiringDigital::mutex);
    if (WiringDigital::intrCallbackVec.empty())
    {
        // init vector
        for (size_t i = 0; i < TARGET_PINS_COUNT; i++)
        {
            WiringDigital::intrCallbackVec.push_back(nullptr);
        }
    }

    WiringDigital::intrCallbackVec[interruptNumber] = callback;

    switch (mode)
    {
    case LOW:
        gpio->setInterrupt(ioig::LevelLow, intrHandler);
        break;
    case HIGH:
        gpio->setInterrupt(ioig::LevelHigh, intrHandler);
        break;
    case CHANGE:
        gpio->setInterrupt(ioig::RiseEdge | ioig::FallEdge, intrHandler);
        break;
    case FALLING:
        gpio->setInterrupt(ioig::FallEdge, intrHandler);
        break;
    case RISING:
        gpio->setInterrupt(ioig::RiseEdge, intrHandler);
        break;
    default:
        break;
    }
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param)
{
    CHECK_PIN(interruptNumber, return);    

    auto gpio = getGpioObj(interruptNumber);
        

    if (gpio == nullptr)
    {        
        gpio = new ioig::Gpio(interruptNumber, ioig::Input);
        std::lock_guard<std::mutex> lock(WiringDigital::mutex);
        WiringDigital::gpioVec[interruptNumber] = gpio;
    }

    std::lock_guard<std::mutex> lock(WiringDigital::mutex);    
    if (WiringDigital::intrCallbackParamVec.empty())
    {
        // init vector
        for (size_t i = 0; i < TARGET_PINS_COUNT; i++)
        {
            WiringDigital::intrCallbackParamVec.push_back(nullptr);
        }
    }

    WiringDigital::intrCallbackParamVec[interruptNumber] = callback;

    switch (mode)
    {
    case LOW:
        gpio->setInterrupt(ioig::LevelLow, intrHandlerParam,param);
        break;
    case HIGH:
        gpio->setInterrupt(ioig::LevelHigh, intrHandlerParam,param);
        break;
    case CHANGE:
        gpio->setInterrupt(ioig::RiseEdge | ioig::FallEdge, intrHandler,param);
        break;
    case FALLING:
        gpio->setInterrupt(ioig::FallEdge, intrHandlerParam,param);
        break;
    case RISING:
        gpio->setInterrupt(ioig::RiseEdge, intrHandlerParam,param);
        break;
    default:
        break;
    }

}

void detachInterrupt(pin_size_t interruptNumber)
{
    CHECK_PIN(interruptNumber, return);

    std::lock_guard<std::mutex> lock(WiringDigital::mutex);

    if (!WiringDigital::intrCallbackVec.empty()) 
    {
        WiringDigital::intrCallbackVec[interruptNumber] = nullptr;
    }
}