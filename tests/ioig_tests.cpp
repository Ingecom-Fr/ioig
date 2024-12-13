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
#include <gtest/gtest.h>
#include <cstdint>

using namespace ioig;
using namespace std::chrono_literals;

#define WAIT_MS(ms) std::this_thread::sleep_for(  std::chrono::milliseconds(ms)  );

#define USB_PORT 0

class AnalogIn_TestBench
{
public:
    AnalogIn_TestBench()
     :tempSensor(std::move(ioig::AnalogIn(ADC_TEMP,10,4)))
    {
        tempSensor.attachToUsbPort(USB_PORT);
    }

    void run()
    {
        float prevRead=0;
        float tempC=0;
        for (int i = 0; i < 20; i++)
        {   
                                             
            tempC = tempSensor.readOnboardTemp();
            
            //assuming room temperature
            ASSERT_GE(tempC, 14) << "Temperature sensor read < 14";
            ASSERT_LE(tempC, 35) << "Temperature sensor read > 35";      
          
            if (i>5) //first reads are not stable
            { 
               ASSERT_TRUE( std::abs(tempC-prevRead) <= 1.5f );
            }
            
            prevRead = tempC;           
        }        
        std::cout << "             On Board Temperatue sensor: " << tempC << " °C" << std::endl;
    }

    ioig::AnalogIn tempSensor;          
};


class AnalogOut_TestBench
{
public:
    AnalogOut_TestBench()
        : fader(ioig::AnalogOut(LED1))
    {
        fader.attachToUsbPort(USB_PORT);
    }

    void run()
    {
        for (size_t i = 0; i < 200 ; i++)
        {         
            fader.write_u16(brightness);
            // change the brightness for next time through the loop:
            brightness = brightness + fadeAmount;
    
            // reverse the direction of the fading at the ends of the fade:
            if (brightness <= 0 || brightness >= 255) 
            {
              fadeAmount = -fadeAmount;
            }                                 
            std::this_thread::sleep_for(30ms);      
        }
    }

    int brightness = 0;  // how bright the LED is
    int fadeAmount = 5;  // how many points to fade the LED by
    ioig::AnalogOut fader;     
};


class GpioTestBench
{
public:
    GpioTestBench()
        : driver1(Gpio(GP10)),
          driver2(Gpio(GP12)),
          echo1(Gpio(GP11)),
          echo2(Gpio(GP13)),
          event1RiseCnt(0),
          event1FallCnt(0),
          event2FallCnt(0)
    {

        driver1.output();
        driver2.output();

        echo1.input();
        echo1.mode(PullNone);

        echo2.input();
        echo2.mode(PullNone);

        auto evtCallback = [&](const int pin, const uint32_t events, void * arg)
        {            
            (void)pin;
            (void)arg;
            switch (events)
            {
            case RiseEdge:
                event1RiseCnt += 1;                          
                break;
            case FallEdge:                            
                event1FallCnt += 1;                
                break;
            default:
                break;
            }
        };
        echo1.setInterrupt(RiseEdge | FallEdge , evtCallback);

        echo2.setInterrupt(FallEdge, [&](const int pin, const uint32_t events, void * arg)
        {
            (void)pin;
            (void)arg;
            
            switch (events)
            {
            case RiseEdge:
                FAIL()  << "RiseEdge should never be triggered";
                break;
            case FallEdge:
                event2FallCnt += 1; 
                break;
            default:
                break;                
            }
        });
    }


    void run() 
    {
        auto thread1 = std::thread([&]()
        {
            int cycles=250;
            for (int i = 0; i < cycles; i++)
            {
                driver1 = 1;
                EXPECT_EQ(driver1.read(), echo1.read()) << "driver1 != echo1, test count = " << i;                
                WAIT_MS(1);

                driver1 = 0;
                EXPECT_EQ(driver1.read(), echo1.read()) << "driver1 != echo1, test count = " << i;   
                WAIT_MS(1);
            }
            EXPECT_EQ(event1RiseCnt, cycles) << "event1RiseCnt mismatch";            
            EXPECT_EQ(event1RiseCnt, event1FallCnt);

        });
        

        auto thread2 = std::thread([&]()
        {
            int cycles=250;
            for (int i = 0; i < cycles; i++)
            {

                driver2 = 1;
                EXPECT_EQ(driver2.read(), echo2.read()) << "driver2 != echo2, test count = " << i;               
                WAIT_MS(1);

                driver2 = 0;
                EXPECT_EQ(driver2.read(), echo2.read()) << "driver2 != echo2, test count = " << i;         
                WAIT_MS(1);
            }
            EXPECT_EQ(event2FallCnt, cycles) << "event2FallCnt mismatch"; 
        });

        thread1.join();
        thread2.join();  

    }

    //GPIO
    ioig::Gpio driver1;
    ioig::Gpio driver2;
    ioig::Gpio echo1;
    ioig::Gpio echo2;
    int event1RiseCnt;
    int event1FallCnt;
    int event2FallCnt;

};


class SpiTestBench
{
public:
    SpiTestBench()
        : spi(ioig::Spi(SPI0_PINOUT0))
    {
        //Seed for random number generation
        std::srand(static_cast<unsigned>(std::time(nullptr)));        
    }


    void run()
    {        
        for (size_t i = 0; i < 250 ; i++)
        {        
            // Fill the buffer with random values
            for (unsigned i = 0; i < BUFF_SIZE; ++i) {
                txBuf[i] = static_cast<uint8_t>(std::rand() % 256); // Range: 0 to 255
            }        
    
            spi.transfer(txBuf, rxBuf, BUFF_SIZE);
            
            for (unsigned i = 0; i < BUFF_SIZE; ++i) 
            {
                EXPECT_EQ(rxBuf[i] , txBuf[i]) << "SPI tx != rx , test count = " << i;
            }
            WAIT_MS(1);
        }
    }

    //SPI
    ioig::Spi spi;        
    static constexpr unsigned BUFF_SIZE = 32;
    uint8_t rxBuf[BUFF_SIZE] = {0};
    uint8_t txBuf[BUFF_SIZE] = {0};    
};



class I2CTestBench
{
public:
    I2CTestBench()
        : i2c(ioig::I2C(I2C1_PINOUT0, /*hz*/ 100'000, I2C_1)),
          addr(0x12)
    {
        i2c.setTimeout(50'000);

        //Seed for random number generation
        std::srand(static_cast<unsigned>(std::time(nullptr)));        
    }

    void run()
    {
        //TODO: need an external I2C device

        for (size_t i = 0; i < 10 ; i++)
        {        
            // Fill the buffer with random values
            for (unsigned i = 0; i < BUFF_SIZE; ++i) {
                txBuf[i] = static_cast<uint8_t>(std::rand() % 256); // Range: 0 to 255
            }

            //TODO:
   
            // EXPECT_EQ(i2c.write(addr, txBuf, BUFF_SIZE),-1 /*NACK*/)  << "I2C write error, test count = " << i;
            // EXPECT_EQ(i2c.read(addr, rxBuf, BUFF_SIZE), -1 /*NACK*/)  << "I2C read error, test count = " << i;
                    
            WAIT_MS(2);
        } 
    }

    ioig::I2C i2c;        
    int addr;
    static constexpr unsigned BUFF_SIZE = 32;
    uint8_t rxBuf[BUFF_SIZE];
    uint8_t txBuf[BUFF_SIZE];    
};



class SerialTestBench
{
public:
    SerialTestBench()
        : serial(ioig::UART(UART1_PINOUT1, 115200, UART_1))
    {
        // Populate the array with printable ASCII characters
        for (int i = 32; i <= 126; ++i) {
            printableAsciiTable[i - 32] = static_cast<char>(i);
        }

        rxCnt = 0;
        serial.setInterrupt([&](const char evtData)
        {
            rxBuf[rxCnt++] = evtData;        
        });        
    }

    void run()
    {

        for (size_t i = 0; i < sizeof(printableAsciiTable) ; i++)
        {            
            serial.putc(printableAsciiTable[i]);                                       
            WAIT_MS(1);
        }

        for (size_t i = 0; i < sizeof(printableAsciiTable) ; i++)
        {                                    
            EXPECT_EQ(rxBuf[i],printableAsciiTable[i]);
        }        
    }
   
    ioig::UART serial;        
    static constexpr unsigned BUFF_SIZE = 256;
    char printableAsciiTable[95];
    uint8_t rxBuf[BUFF_SIZE] = {0};
    int rxCnt;
};



TEST(IoIgTests, AnalogIn_TestBench) 
{
    AnalogIn_TestBench test;
    test.run();
}

TEST(IoIgTests, AnalogOut_TestBench) 
{
    AnalogOut_TestBench test;
    test.run();
}

TEST(IoIgTests, Gpio_TestBench) 
{
    GpioTestBench test;
    test.run();
}

TEST(IoIgTests, SPI_TestBench) 
{
    SpiTestBench test;
    test.run();    
}

TEST(IoIgTests, Serial_TestBench) 
{
    SerialTestBench test;
    test.run();
}

TEST(IoIgTests, I2C_TestBench) 
{
    I2CTestBench test;
    test.run();
}

TEST(IoIgTests, GPIO_SPI_Test) 
{
    auto thread1 = std::thread([&]() 
    {        
        GpioTestBench test;    
        test.run();       
    });

    auto thread2 = std::thread([&]() 
    {        
        SpiTestBench test;
        test.run();   
    });
    
    thread1.join();
    thread2.join();
}


TEST(IoIgTests, GPIO_Serial_Test) 
{
    auto thread1 = std::thread([&]() 
    {        
        GpioTestBench test;    
        test.run();       
    });

    auto thread2 = std::thread([&]() 
    {        
        SerialTestBench test;
        test.run();   
    });
    
    thread1.join();
    thread2.join();
}



int main(int argc, char **argv) 
{
    // Code here will be called immediately after the constructor (right
    // before each test).
    std::string msg =
        "*********************************************************************\n"
        "*   IoIg TestBench                                                  *\n"
        "*   Hardware Requirements:                                          *\n"
        "*    - A Raspberry PI PICO Board                                    *\n"
        "*    - Connection Wires                                             *\n"
        "*   Test preparation:                                               *\n"
        "*    - UART Tests: Wire together (GP8 + GP9)                        *\n"
        "*    - GPIO Tests: Wire together (GP10 + GP11) and (GOP12 + GP13)   *\n"
        "*    - SPI  Tests: Wire together (GP19 + GP16)                      *\n"
        "*    - I2C  tests: No wiring needed                                 *\n"
        "*   Manual Checks:                                                  *\n"
        "*    - Check FADE effect on the board LED                           *\n"
        "*   NOTE: Don't touch the board while running tests,                *\n"
        "*         electrical noise can cause test failure                   *\n"   
        "*********************************************************************\n";

    std::cerr << msg << std::endl;
    std::cout << "Press Enter to continue...";
    std::string line;
    std::getline(std::cin, line); // Read a line of input
    std::cout << "Running TestBench..." << std::endl;

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}