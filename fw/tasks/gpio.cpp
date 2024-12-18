#include <hardware/irq.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>
#include <pico/util/queue.h>

#include <tusb.h>
#include <stdlib.h>

#include "fw/tasks/gpio.h"
#include "fw/main.h"


GpioTask &gpioTask = GpioTask::instance();

void GpioTask::init()
{
    queue_init(&_irqEventQueue, sizeof(uint32_t), EVT_QUEUE_MAX_SIZE);  //TODO: move to processSetIrq
    setState(Task::State::RUNNING);
}

void GpioTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);
    sleep_ms(2); 
    while (queue_get_level(&_irqEventQueue) > 0) 
    {
      uint32_t tmp=0;
      queue_remove_blocking(&_irqEventQueue, &tmp);
    }    
    setState(prevState);
}

void GpioTask::irqHandler(unsigned pin, uint32_t evt)
{
    if (gpioTask.getStateUnsafe() == Task::State::STOPPED)
    {
      return;
    }    

    // since gpio/event_mask are small values(<=255), we combine them in a single uint32_t 
    uint32_t pin_  = (uint16_t)(pin & 0xFFFF);
    uint32_t event_ = (uint16_t)(evt & 0xFFFF);

    // Combine the low bits
    uint32_t combined = static_cast<uint32_t>((pin_ << 16) | event_);

    if (!queue_try_add(&gpioTask._irqEventQueue, &combined))
    {   
        //queue if full
        uint32_t tmp=0;
        queue_remove_blocking(&gpioTask._irqEventQueue, &tmp);    
        queue_add_blocking(&gpioTask._irqEventQueue, &combined);
        DBG_MSG("Gpio: event queue full! discarging items...\n");
    }
}

inline void GpioTask::processInit(Packet &rxPkt,Packet &txPkt)
{
    
    auto pin      = rxPkt.getPayloadItem8(0); 
    auto mode     = rxPkt.getPayloadItem8(1);
    auto dir      = rxPkt.getPayloadItem8(2);

    gpio_init(pin);
    gpio_set_dir(pin, dir);

    switch (mode)
    {
    case PullUp:
        gpio_pull_up(pin);
        break;
    case PullDown:
        gpio_pull_down(pin);
        break;
    default:
        gpio_disable_pulls(pin);
        break;
    }  

    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(mode);
    txPkt.addPayloadItem8(dir);
}

inline void GpioTask::processDeInit(Packet &rxPkt,Packet &txPkt)
{    
    auto pin = rxPkt.getPayloadItem8(0); 
    gpio_deinit(pin);
    //TODO: queue_free
    txPkt.addPayloadItem8(pin);
}

inline void GpioTask::processSetMode(Packet &rxPkt,Packet &txPkt)
{    
    auto pin   = rxPkt.getPayloadItem8(0); 
    auto mode  = rxPkt.getPayloadItem8(1);

    switch (mode)
    {
    case PullUp:
        gpio_pull_up(pin);
        break;
    case PullDown:
        gpio_pull_down(pin);
        break;
    default:
        gpio_disable_pulls(pin);
        break;
    }
    
    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(mode);

}


inline void GpioTask::processEvents(Packet &txPkt)
{        
    uint8_t qsz = (uint8_t)queue_get_level(&gpioTask._irqEventQueue);   

    int evt_cnt = qsz < EVT_QUEUE_MAX_SIZE ? qsz : EVT_QUEUE_MAX_SIZE-1 /*evt_cnt slot*/;

    if (evt_cnt == 0) 
    {
        return;
    }

    txPkt.addPayloadItem8(evt_cnt);      

    //chain events in a single pkt
    while (evt_cnt-- > 0) 
    {
        uint32_t gpio_event = 0;

        queue_remove_blocking(&gpioTask._irqEventQueue, &gpio_event);
        txPkt.addPayloadItem32(gpio_event);
    }   


    mainTask.cdcWrite(CDCItf::EVENT, txPkt.getBuffer(), txPkt.getBufferLength());    
}


inline void GpioTask::processSetValue(Packet &rxPkt,Packet &txPkt)
{    
    auto pin = rxPkt.getPayloadItem8(0);
    auto val = rxPkt.getPayloadItem8(1);
    gpio_put(pin, val);
}


inline void GpioTask::processGetValue(Packet &rxPkt,Packet &txPkt)
{
    auto pin = rxPkt.getPayloadItem8(0);
    auto val = gpio_get(pin);
    txPkt.addPayloadItem8(val);
}

inline void GpioTask::processSetIrq(Packet &rxPkt,Packet &txPkt)
{
    auto pin = rxPkt.getPayloadItem8(0);
    auto enable = rxPkt.getPayloadItem8(1); 
    uint32_t events = rxPkt.getPayloadItem32(2); 
            
    if (enable >= 1)
    {        
        gpio_set_irq_enabled_with_callback(pin, events, true, &GpioTask::irqHandler);
    }
    else
    {
        gpio_set_irq_enabled(pin, events, false);
    }

    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(enable);
    txPkt.addPayloadItem32(events);
}

inline void GpioTask::processSetDir(Packet &rxPkt,Packet &txPkt)
{       
    auto pin = rxPkt.getPayloadItem8(0); 
    auto dir = rxPkt.getPayloadItem8(1);
    gpio_set_dir(pin, dir);

    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(dir);
}


inline void GpioTask::processPulseIn(Packet &rxPkt,Packet &txPkt)
{
    
    auto pin = rxPkt.getPayloadItem8(0); 
    auto state = rxPkt.getPayloadItem8(1);
    uint64_t timeout = rxPkt.getPayloadItem64(2);

    unsigned long startMicros = time_us_64();

    // wait for any previous pulse to end
    while (gpio_get(pin) == state)
    {
        tight_loop_contents();
        if (time_us_64() - startMicros > timeout)
            return;
    }

    // wait for the pulse to start
    while (gpio_get(pin) != state)
    {
        tight_loop_contents();
        if (time_us_64() - startMicros > timeout)
            return;
    }

    unsigned long start = time_us_64();
    // wait for the pulse to stop
    while (gpio_get(pin) == state)
    {
        tight_loop_contents();
        if (time_us_64() - startMicros > timeout)
            return;
    }

    uint64_t result = time_us_64() - start;

    txPkt.addPayloadItem64(result);
}


void GpioTask::process(Packet &rxPkt,Packet &txPkt)
{
          
    CHECK_STATE();
    
    auto rxPktType = rxPkt.getType();    

    switch (rxPktType)
    {
    case Packet::Type::GPIO_INIT:            
        processInit(rxPkt,txPkt);   
        break;    
    case Packet::Type::GPIO_DEINIT:            
        processDeInit(rxPkt,txPkt);   
        break;    
    case Packet::Type::GPIO_EVENT:
        processEvents(txPkt);        
        break;
    case Packet::Type::GPIO_SET_MODE:
        processSetMode(rxPkt,txPkt);         
        break;
    case Packet::Type::GPIO_SET_VALUE:
        processSetValue(rxPkt,txPkt);
        break;
    case Packet::Type::GPIO_GET_VALUE:
        processGetValue(rxPkt,txPkt);
        break;
    case Packet::Type::GPIO_SET_IRQ:
        processSetIrq(rxPkt,txPkt);
        break;
    case Packet::Type::GPIO_SET_DIR:
        processSetDir(rxPkt,txPkt);
    break;
    case Packet::Type::GPIO_PULSE_IN:    
        processPulseIn(rxPkt,txPkt);    
    break;
    default:        
        break;
    }

    
}

