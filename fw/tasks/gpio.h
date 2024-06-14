#pragma once


#include "main.h"
#include <pico/util/queue.h>


class GpioTask : public Task {

public:   

    void init() override;
    void reset() override;
    void process(Packet &rxPkt,Packet &txPkt) override;
    


public:
    static GpioTask & instance() 
    {
        static GpioTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    GpioTask(const GpioTask&) = delete;
    GpioTask& operator=(const GpioTask&) = delete;
    virtual ~GpioTask() {}  

private:  
    GpioTask() {};   


    //task actions
    void processInit(Packet &rxPkt,Packet &txPkt);
    void processDeInit(Packet &rxPkt,Packet &txPkt);
    void processSetMode(Packet &rxPkt,Packet &txPkt);
    void processEvents(Packet &txPkt);
    void processSetValue(Packet &rxPkt,Packet &txPkt);
    void processGetValue(Packet &rxPkt,Packet &txPkt);
    void processSetIrq(Packet &rxPkt,Packet &txPkt);
    void processSetDir(Packet &rxPkt,Packet &txPkt);
    void processPulseIn(Packet &rxPkt,Packet &txPkt);

    static void irqHandler(unsigned gpio, uint32_t event_mask);
    queue_t _irqEventQueue;    
    static constexpr uint8_t EVT_QUEUE_MAX_SIZE=8;    


};

extern GpioTask & gpioTask;

