#pragma once 
#include "main.h"

class SerialTask : public Task {

public:   

    void init() override;
    void reset() override;

    void process(Packet &rxPkt,Packet &txPkt) override;       


public:
    static SerialTask & instance() 
    {
        static SerialTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    SerialTask(const SerialTask&) = delete;
    SerialTask& operator=(const SerialTask&) = delete;
    virtual ~SerialTask() {}  

private:      
    SerialTask();       

    //task actions
    void processInit(Packet & rxPkt, Packet & txPkt);
    void processDeInit(Packet & rxPkt, Packet & txPkt);
    void processSetBaud(Packet & rxPkt, Packet & txPkt);
    void processSetFormat(Packet & rxPkt, Packet & txPkt);
    void processSetIrq(Packet & rxPkt, Packet & txPkt);
    void processSetFlowControl(Packet & rxPkt, Packet & txPkt);
    void processReadable(Packet & rxPkt, Packet & txPkt);
    void processWritable(Packet & rxPkt, Packet & txPkt);
    void processSetBreak(Packet & rxPkt, Packet & txPkt);
    void processGetC(Packet & rxPkt, Packet & txPkt);
    void processPutC(Packet & rxPkt, Packet & txPkt);
    void processWrite(Packet & rxPkt, Packet & txPkt);
    void processRead(Packet & rxPkt, Packet & txPkt);
    void processEvent(Packet & rxPkt, Packet & txPkt);


    void onRxInterrupt(uart_inst_t *uart_port);
    static void irqHandlerUART0_Rx(void);
    static void irqHandlerUART1_Rx(void);

    char     _irqRxC[2]; //UART0, UART1
    bool     _rxReady[2];      
    critical_section_t _critSection;	

};

extern SerialTask & serialTask;

