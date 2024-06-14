#pragma once 

#include "main.h"


class AnalogTask : public Task {

public:   

    void init() override;
    void reset() override;

    void process(Packet &rxPkt,Packet &txPkt) override;       


public:
    static AnalogTask & instance() 
    {
        static AnalogTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    AnalogTask(const AnalogTask&) = delete;
    AnalogTask& operator=(const AnalogTask&) = delete;
    virtual ~AnalogTask() {}  

private:  
    AnalogTask() {};           

    static constexpr float    ADC_VREF_VOLTAGE      = 3.3f; /* 3.3V */
    static constexpr uint16_t ADC_RESOLUTION_BITS   = 12;
    static constexpr float    ADC_CONVERSION_FACTOR = ADC_VREF_VOLTAGE / (1 << ADC_RESOLUTION_BITS);    

    //task actions
    void processInit(Packet & rxPkt, Packet & txPkt);
    void processDeInit(Packet & rxPkt, Packet & txPkt);
    void processWrite(Packet & rxPkt, Packet & txPkt);
    void processRead(Packet & rxPkt, Packet & txPkt);
    void processReadTemp(Packet & rxPkt, Packet & txPkt);
};

extern AnalogTask & analogTask;

