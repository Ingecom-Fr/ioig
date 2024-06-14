#include <hardware/irq.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include "fw/tasks/i2c.h"
#include "fw/main.h"


I2CTask &i2cTask = I2CTask::instance();


void I2CTask::init()
{
    _timeout_us = 1000'000; /*1s*/
    setState(Task::State::RUNNING);
}

void I2CTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);
    sleep_ms(2);
    setState(prevState);    
}


inline void I2CTask::processInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == I2C_0 ? i2c0 : i2c1;   
    auto sda  = rxPkt.getPayloadItem8(1);
    auto scl  = rxPkt.getPayloadItem8(2);  
    auto freqHz  = rxPkt.getPayloadItem32(3);

    i2c_init(hwInstance, freqHz);

    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);  

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(sda);
    txPkt.addPayloadItem8(scl);    
    txPkt.addPayloadItem32(freqHz);

}

inline void I2CTask::processDeInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == I2C_0 ? i2c0 : i2c1;   
    auto sda  = rxPkt.getPayloadItem8(1);
    auto scl  = rxPkt.getPayloadItem8(2);
    
    i2c_deinit(hwInstance);
    gpio_deinit(sda);
    gpio_deinit(scl);

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(sda);
    txPkt.addPayloadItem8(scl);       
}

inline void I2CTask::processSetFreq(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == I2C_0 ? i2c0 : i2c1;
    auto freqHz = rxPkt.getPayloadItem32(1);

    i2c_set_baudrate(hwInstance, freqHz);

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem32(freqHz);

}


inline void I2CTask::processSetTimeout(Packet &rxPkt, Packet &txPkt)
{
    _timeout_us = rxPkt.getPayloadItem32(0); 
    txPkt.addPayloadItem32(_timeout_us);    
}

inline void I2CTask::processWrite(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance   = rxPkt.getPayloadItem8(0) == I2C_0 ? i2c0 : i2c1;
    auto addr   = rxPkt.getPayloadItem8(1);
    auto len    = rxPkt.getPayloadItem8(2);
    auto nostop = rxPkt.getPayloadItem8(3);

    uint8_t *buf = rxPkt.getPayloadBuffer(4);

    int ret = i2c_write_timeout_us(hwInstance, addr, buf, len, nostop, _timeout_us);

    if (ret == PICO_ERROR_GENERIC)
    {
        txPkt.setStatus(Packet::Status::RSP_I2C_NACK);
    }else 
    if (ret == PICO_ERROR_TIMEOUT) 
    {
        txPkt.setStatus(Packet::Status::RSP_I2C_TIMEOUT);
    } 
}

inline void I2CTask::processRead(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance   = rxPkt.getPayloadItem8(0) == I2C_0 ? i2c0 : i2c1;
    auto addr   = rxPkt.getPayloadItem8(1);
    auto len    = rxPkt.getPayloadItem8(2);
    auto nostop = rxPkt.getPayloadItem8(3);

    uint8_t *buf = txPkt.getPayloadBuffer();

    int ret = 0;

    ret = i2c_read_timeout_us(hwInstance, addr, buf, len, nostop, _timeout_us);

    if (ret > 0) // returned number of bytes read
    {
        if (txPkt.increasePayloadLength(ret) < 0)
        {
            txPkt.setStatus(Packet::Status::RSP_I2C_BUF_OVERFLOW);
        }
    }else 
    if (ret == PICO_ERROR_GENERIC)
    {
        txPkt.setStatus(Packet::Status::RSP_I2C_NACK);

    }else 
    if (ret == PICO_ERROR_TIMEOUT) 
    {
        txPkt.setStatus(Packet::Status::RSP_I2C_TIMEOUT);
    }
}

void I2CTask::process(Packet &rxPkt,Packet &txPkt)
{
     

    CHECK_STATE();  

    auto rxPktType = rxPkt.getType();   

    switch (rxPktType)
    {
    case Packet::Type::I2C_INIT:
        processInit(rxPkt,txPkt);
    break;
    case Packet::Type::I2C_DEINIT:
        processDeInit(rxPkt,txPkt);
    break;
    case Packet::Type::I2C_READ:
        processRead(rxPkt,txPkt);
    break;
    case Packet::Type::I2C_WRITE:
        processWrite(rxPkt,txPkt);
    break;
    case Packet::Type::I2C_SET_FREQ:
        processSetFreq(rxPkt,txPkt);
    break;
    case Packet::Type::I2C_SET_TIMEOUT:
        processSetTimeout(rxPkt,txPkt);
    break;

    default:                   
        break;
    } 
                
        
}

