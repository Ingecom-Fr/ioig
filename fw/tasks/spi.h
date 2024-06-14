#pragma once 

#include "main.h"
#include <hardware/spi.h>

class SpiTask : public Task {

public:   

    void init() override;
    void reset() override;

    void process(Packet &rxPkt,Packet &txPkt) override;       


public:
    static SpiTask & instance() 
    {
        static SpiTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    SpiTask(const SpiTask&) = delete;
    SpiTask& operator=(const SpiTask&) = delete;
    virtual ~SpiTask() {}  

private:  
    SpiTask() {};        
    
    //task actions
    void processInit(Packet & rxPkt, Packet & txPkt);
    void processDeInit(Packet & rxPkt, Packet & txPkt);
    void processSetFreq(Packet & rxPkt, Packet & txPkt);
    void processWrite(Packet & rxPkt, Packet & txPkt);
    void processRead(Packet & rxPkt, Packet & txPkt);
    void processTransfer(Packet & rxPkt, Packet & txPkt);
    void processSetFormat(Packet & rxPkt, Packet & txPkt);

};

extern SpiTask & spiTask;


