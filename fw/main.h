#pragma once

#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <atomic>

#include <pico/multicore.h>
#include <pico/util/queue.h>
#include <pico/stdlib.h>
#include <pico/critical_section.h>
#include <pico/stdio.h>

#include <hardware/resets.h>
#include <hardware/watchdog.h>
#include <bsp/board_api.h>
#include <hardware/gpio.h>
#include <hardware/sync.h>
#include <hardware/structs/ioqspi.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/ssi.h>
#include "hardware/vreg.h"

#include "host/APIs/native/ioig.h"
#include "host/ioig_protocol.h"

#include "defines.h"
#include "device.h"

using namespace ioig;


/**
 * @brief Task Abstract Class
 * 
 */

class Task
{
public:
    enum class State
    {
        NONE = 0,
        RUNNING,
        IDLE,
        STOPPED       
    };

    Task();

    virtual ~Task() {}

    virtual void init() = 0;
    virtual void deinit(){};
    virtual void reset(){};
    virtual void process(Packet &rxPkt,Packet &txPkt) = 0;    

    void setState(State new_state);

    Task::State getState();
    Task::State getStateUnsafe() { return _state; };

private:

    Task::State _state; 
    mutex_t _mutex;

    // Private copy constructor (non-implemented)
    Task(const Task &other) = delete;

    // Private assignment operator (non-implemented)
    Task &operator=(const Task &other) = delete;
};



/**
 * @brief The Main Task Class
 * Manages all sub-tasks, receive rx events, sent tx....
 */
class MainTask : public Task
{

public:
    void init() override;   

    void reset() override;

    void onRx(const CDCItf itf);

    void onTx(const CDCItf itf);

    void process(Packet &rxPkt,Packet &txPkt) override;

    void cdcRead(const CDCItf itf, uint8_t *buf,  const unsigned len);

    void cdcWrite(const CDCItf itf, uint8_t *buf, const unsigned len);


    // singleton
public:
    static MainTask &instance()
    {
        static MainTask inst;
        return inst;
    }
    // Prevent copy construction and assignment
    MainTask(const MainTask &) = delete;
    MainTask &operator=(const MainTask &) = delete;
    virtual ~MainTask() {}

private:

    MainTask() = default;
    static void mainLoop1();
    void mainLoop0();    

    static constexpr unsigned RX_PKT_QUEUE_MAX_SIZE = 6;        
    Packet   _rxPacketVec[RX_PKT_QUEUE_MAX_SIZE];
    queue_t  _rxPktIndexQueue; 
    uint32_t _rxPktIdx;

};


class Board
{
public:
    void init();
    void reset();
    uint32_t getTotalHeap();
    uint32_t getFreeHeap();
    void onDeviceMounted();
    void onDeviceUnMounted();
    void onSuspend();
    void onReseume();
    void ledBlink();    

    // singleton
public:
    static Board &instance()
    {
        static Board inst;
        return inst;
    }
    // Prevent copy construction and assignment
    Board(const Board &) = delete;
    Board &operator=(const Board &) = delete;
    virtual ~Board() {}    

private:
    
    enum BlinkPattern
    {
      BLINK_MOUNTED = 250,
      BLINK_NOT_MOUNTED = 500,      
      BLINK_SUSPENDED = 2500,
    };

    uint32_t blinkInterval_ms = BLINK_NOT_MOUNTED;
    Board() = default;    
};

extern MainTask &mainTask;
extern Board &board;

