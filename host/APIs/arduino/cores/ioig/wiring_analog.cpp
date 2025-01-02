#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <mutex>

#include "ioig.h"


#define CHECK_ANALOG_IN_PIN(pinNumber, action) if (pinNumber < 0 || pinNumber >= TARGET_PINS_COUNT || (pinNumber != A0 && pinNumber != A1 && pinNumber != A2 && pinNumber != A3) ) action;
#define CHECK_ANALOG_OUT_PIN(pinNumber, action) if (pinNumber < 0 || pinNumber >= TARGET_PINS_COUNT ) action;



namespace WiringAnalog
{
    static std::vector<ioig::AnalogIn *> analogInVec;
    static std::vector<ioig::AnalogOut *> analogOutVec;
    static int write_resolution = 8;
    static int read_resolution = 10;
    static std::mutex mutex;

};

static ioig::AnalogIn * getAnalogInObj(pin_size_t pinNumber) 
{    
    std::lock_guard<std::mutex> lock(WiringAnalog::mutex);
    if (WiringAnalog::analogInVec.empty()) 
    {   
        for (size_t i = 0; i < ADC_INSTANCES; i++)        
        {
            WiringAnalog::analogInVec.push_back(nullptr);
        }
    }

    return WiringAnalog::analogInVec[pinNumber % ADC_INSTANCES];
}


static ioig::AnalogOut * getAnalogOutObj(pin_size_t pinNumber) 
{    
    std::lock_guard<std::mutex> lock(WiringAnalog::mutex);
    if (WiringAnalog::analogOutVec.empty()) 
    {   
        for (size_t i = 0; i < TARGET_PINS_COUNT; i++)        
        {
            WiringAnalog::analogOutVec.push_back(nullptr);
        }
    }

    return WiringAnalog::analogOutVec[pinNumber];
}


void analogWrite(pin_size_t pin, int val)
{
    CHECK_ANALOG_OUT_PIN(pin, return);


    auto aOutObj = getAnalogOutObj(pin);

    if ( aOutObj == nullptr ) 
    {   
        aOutObj = new ioig::AnalogOut(pin);
        std::lock_guard<std::mutex> lock(WiringAnalog::mutex);
        WiringAnalog::analogOutVec[pin] = aOutObj;
        aOutObj->setWriteResolution(WiringAnalog::write_resolution);        
    }

    float percent = (float)val/(float)((1 << WiringAnalog::write_resolution)-1);

    if (percent >= 0) 
    {
        aOutObj->write_u16(val);        
    }else {
        delete aOutObj;
        WiringAnalog::analogOutVec[pin] = nullptr;
    }

}

void analogWriteResolution(int bits)
{
    WiringAnalog::write_resolution = bits;
}


int analogRead(pin_size_t pin)
{
    CHECK_ANALOG_IN_PIN(pin, return -1);
    auto adc = getAnalogInObj(pin);
    if (adc == nullptr) 
    {
      adc = new ioig::AnalogIn(pin);
      WiringAnalog::analogInVec[pin] = adc;
    }

   return adc->read_u16();
}

void analogReadResolution(int bits)
{
    WiringAnalog::read_resolution = bits;
}

int getAnalogReadResolution()
{
    return WiringAnalog::read_resolution;
}
