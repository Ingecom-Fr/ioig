#include "Arduino.h"
#include <chrono>
#include <thread>


static std::chrono::high_resolution_clock::time_point startTimestamp;


// Declared weak in Arduino.h to allow user redefinitions.
int atexit(void (* /*func*/ )()) { return 0; }

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() {}

void setup(void) __attribute__((weak));
void setup(void) {}

void loop(void) __attribute__((weak));
void loop(void) {}


void yield(void) 
{ 
    std::this_thread::yield(); 
}


void init()
{
    startTimestamp = std::chrono::high_resolution_clock::now();
}

unsigned long millis()
{
    auto now = std::chrono::high_resolution_clock::now();  
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTimestamp).count();
}

unsigned long micros() 
{
    auto now = std::chrono::high_resolution_clock::now(); 
    return std::chrono::duration_cast<std::chrono::microseconds>(now - startTimestamp).count();
}

void delay(unsigned long ms)
{
    std::this_thread::sleep_for( std::chrono::milliseconds(ms) );
}

void delayMicroseconds(unsigned int us)
{
  std::this_thread::sleep_for( std::chrono::microseconds(us) );
}


int main() 
{
	init();
	initVariant();

#if defined(SERIAL_CDC)
  PluggableUSBD().begin();
  _SerialUSB.begin(115200);
#endif

	setup();

	for (;;) {
		loop();
		//if (arduino::serialEventRun) arduino::serialEventRun();
	}

	return 0;
}