#include <chrono>
#include <thread>
#include <iostream>
#include <random>
#include <string>
#include <cstdlib>
#include <type_traits>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Arduino.h"
#include "deprecated-avr-comp/avr/dtostrf.h"

static std::chrono::high_resolution_clock::time_point startTimestamp;

// Seed the random number generator
void randomSeed(unsigned long seed) {
    if (seed == 0) {
        // If seed is 0, use the current time as the seed
        seed = (unsigned long)time(NULL);
    }
    srand((unsigned int)seed);
}

// Generate a random number between 0 and max (exclusive)
long random(long max) {
    if (max <= 0) {
        return 0; // Return 0 for invalid max values
    }
    return rand() % max;
}

// Generate a random number between min and max (exclusive)
long random(long min, long max) {
    if (min >= max) {
        return min; // Return min if the range is invalid
    }
    return min + (rand() % (max - min));
}

char *dtostrf(double val, signed char width, unsigned char prec, char *sout) 
{
    if (sout == nullptr) return nullptr;

    char format[32];
    snprintf(format, sizeof(format), "%%%d.%df", width, prec);

    // Format the double value into the output buffer
    snprintf(sout, width + 1, format, val);

    return sout;
}


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

	setup();

	for (;;) {
		loop();
		//if (arduino::serialEventRun) arduino::serialEventRun();
	}

	return 0;
}


#if defined(_WIN32) || defined(_WIN64)
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
	init();
	initVariant();

	setup();

	for (;;) {
		loop();
	}

	return 0;
}
#endif
