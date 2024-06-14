#include <csignal>
#include <thread>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <string>
#include <sched.h>
#include <unistd.h>

#include "ioig.h"

using namespace ioig;
using namespace std::chrono_literals;

#define TRIG_PIN 11
#define ECHO_PIN 10
#define SOUND_SPEED 0.034

#define DELAY(x) std::this_thread::sleep_for(x)

int main() 
{
	std::cout << std::unitbuf; // enable automatic flushing
	std::cerr << std::unitbuf; // enable automatic flushing
        
    puts("HC-SR04 Ultrasonic Sensor Example");
    puts("INFO: Plug HC-SR05 VCC in 5V power pin!");
    
    fflush(stdout);                                                                         
    
    ioig::Gpio trigPin(TRIG_PIN, Output);    
    ioig::Gpio echoPin(ECHO_PIN, Input);

    while (1) 
    {
        trigPin.write(0);
        DELAY(2us);
        trigPin.write(1);
        DELAY(10us);
        trigPin.write(0);        
        auto duration = echoPin.pulseIn(1);
        float distance_cm = duration * SOUND_SPEED/2;
        std::cout << "Distance (cm) : " << distance_cm << std::endl;
        sleep(4);
    }

    return 0;
}
