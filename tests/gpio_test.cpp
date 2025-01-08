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

#include "ioig.h"
#include <gtest/gtest.h>
#include <cstdint>

using namespace ioig;
using namespace std::chrono_literals;


static void gpio_test() 
{    
    //----------------------------------------------------    
    ioig::Gpio g10(10);
    g10.output();
    
    //----------------------------------------------------
    ioig::Gpio g11(11, Input, PullNone);                
    auto evtCallback = [&](const int pin,const uint32_t events, void * arg)
    {        
        (void)arg;
        switch (events)
        {
        case RiseEdge:
            printf("RiseEdge event on pin %d\n", pin);
            break;
        case FallEdge:
            printf("FallEdge event on pin %d\n", pin);
            break;
        case LevelHigh:
            printf("LevelHigh event on pin %d\n", pin);
            break;
        case LevelLow:
            printf("LevelLow event on pin %d\n", pin);
            break;        
        default:
            break;
        }    
        fflush(stdout);    
    };
    g11.setInterrupt(RiseEdge | FallEdge , evtCallback );    
    
    //----------------------------------------------------
    Gpio g12(12);
    g12.input(); 
    g12.mode(PullNone); 

    g12.setInterrupt(FallEdge , [&](const int pin,const uint32_t events, void * arg) 
    { 
        (void)arg;
        printf("FallEdge on pin %d, ev value=%d\n", pin , events);
    });

 
    while(1)
    {
        g10 = 1;                     
        printf("g10=%d, g11=%d, g12=%d\n", g10.read(), g11.read(), g12.read());
        fflush(stdout);
        std::this_thread::sleep_for(5s);
        
        g10 = 0;        
        printf("g10=%d, g11=%d, g12=%d\n", g10.read(), g11.read(), g12.read());        
        fflush(stdout);
        std::this_thread::sleep_for(5s);              
    }
}


int main()
{
    puts("ioig Gpio Test");                                                                    
    puts("INFO: Wire together (GP10 - GP11) or (GP10 - GP12)");
    
    gpio_test();
    
    return 0;
}


