#pragma once 

#include <hardware/i2c.h>

#include "main.h"

class I2CTask : public Task {

public:   

    void init() override;
    void reset() override;

    void process(Packet &rxPkt,Packet &txPkt) override;       


public:
    static I2CTask & instance() 
    {
        static I2CTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    I2CTask(const I2CTask&) = delete;
    I2CTask& operator=(const I2CTask&) = delete;
    virtual ~I2CTask() {}  

private:      
    I2CTask() {};       

    //task actions
    void processInit(Packet & rxPkt, Packet & txPkt);
    void processDeInit(Packet & rxPkt, Packet & txPkt);
    void processSetFreq(Packet & rxPkt, Packet & txPkt);
    void processSetTimeout(Packet &rxPkt, Packet &txPkt);
    void processWrite(Packet & rxPkt, Packet & txPkt);
    void processRead(Packet & rxPkt, Packet & txPkt);

    long unsigned _timeout_us;

};

extern I2CTask & i2cTask;

