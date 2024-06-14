#include <iostream>
#include <memory.h>
#include <csignal>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <chrono>
#include <string>
#include <atomic>
#include <random>
#include <functional>
#include <list>

#include "ioig.h"

#define USE_THREADS 1

using namespace ioig;
using namespace std::chrono_literals;

static int brightness = 0;  // how bright the LED is
static int fadeAmount = 5;  // how many points to fade the LED by
static int ledPin     = 21;
static int sensorPin  = A0;


int main()
{
    puts("IoIg Analog Test");
    puts("   INFO: Pin 21 Analog Write (LED Fade)");
    puts("   INFO: Pin 26 Analog Read  (Sersor Read)");
    
#if USE_THREADS
    
    auto analogWriteTherad = std::thread([&]() { 
        puts("Analog Out Therad");
        //ioig::Gpio led(ledPin, Output);
        ioig::AnalogOut analogOut(ledPin);        
        while (1) 
        {
            analogOut.write_u16(brightness);
            // change the brightness for next time through the loop:
            brightness = brightness + fadeAmount;
    
            // reverse the direction of the fading at the ends of the fade:
            if (brightness <= 0 || brightness >= 255) {
              fadeAmount = -fadeAmount;
            }        
                         
            std::this_thread::sleep_for(30ms);            
        }
    });

    
    auto analogReadTherad = std::thread([&]() { 
        puts("Analog In Therad");
        ioig::AnalogIn analogIn(sensorPin);            
        while (1) 
        {
            auto val = analogIn.read_u16();
            printf("Read sensor value=%d\n", val);
            
            std::this_thread::sleep_for(500ms);            
        }
    });

    analogWriteTherad.join();    
    analogReadTherad.join();

#else
    ioig::Gpio led(ledPin, ioig::PinDirection::Output);
    ioig::AnalogOut analogOut(ledPin);
    ioig::AnalogIn analogIn(sensorPin);       

    int cnt=0;

    while (1)
    {
        analogOut.write_u16(brightness);
        // change the brightness for next time through the loop:
        brightness = brightness + fadeAmount;

        // reverse the direction of the fading at the ends of the fade:
        if (brightness <= 0 || brightness >= 255)
        {
            fadeAmount = -fadeAmount;
        }

        if (cnt++ % 20 == 0)
        {
            auto val = analogIn.read_u16();
            printf("Read sensor value=%d\n", val);
        }
        std::this_thread::sleep_for(30ms);
    }
#endif

    return 0;
}


