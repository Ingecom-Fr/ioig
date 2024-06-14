#include <hardware/irq.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/spi.h>
#include "hardware/sync.h"
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <stdlib.h>
#include <tusb.h>

#include "fw/tasks/spi.h"
#include "fw/main.h"


SpiTask &spiTask = SpiTask::instance();


void SpiTask::init()
{
    setState(Task::State::RUNNING);
}

void SpiTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);
    sleep_ms(2);
    setState(prevState);    
}

inline void SpiTask::processInit(Packet &rxPkt, Packet &txPkt)
{
   
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    auto sck_pin  = rxPkt.getPayloadItem8(1);    
    auto tx_pin   = rxPkt.getPayloadItem8(2); 
    auto rx_pin   = rxPkt.getPayloadItem8(3); 
    auto cs_pin   = rxPkt.getPayloadItem8(4); 
    auto freqHz   = rxPkt.getPayloadItem32(5);

    gpio_set_function(sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(tx_pin, GPIO_FUNC_SPI);
    gpio_set_function(rx_pin, GPIO_FUNC_SPI);
    
    freqHz = spi_init(hwInstance, freqHz); 

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(sck_pin);
    txPkt.addPayloadItem8(tx_pin);
    txPkt.addPayloadItem8(rx_pin);
    txPkt.addPayloadItem8(cs_pin);
    txPkt.addPayloadItem32(freqHz);

}

inline void SpiTask::processDeInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    spi_deinit(hwInstance);
}

inline void SpiTask::processSetFreq(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    uint32_t freqHz = rxPkt.getPayloadItem32(1);
    spi_set_baudrate(hwInstance, freqHz);
}

inline void SpiTask::processWrite(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    int  len = rxPkt.getPayloadItem8(1);
    uint8_t *buf = rxPkt.getPayloadBuffer(2); // skip params

    int trBytes = 0;
   

    trBytes = spi_write_blocking(hwInstance, buf, len);

    if (trBytes != len)
    {
        txPkt.setStatus(Packet::Status::RSP_SPI_LEN_MISMATCH);
    }
}

inline void SpiTask::processRead(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    int len = rxPkt.getPayloadItem8(1);
    int repeatedVal = rxPkt.getPayloadItem8(2);

    uint8_t *buf = txPkt.getPayloadBuffer();

    int trBytes = 0; // Number of bytes transfered

    trBytes = spi_read_blocking(hwInstance, repeatedVal, buf, len);

    int ret = txPkt.increasePayloadLength(trBytes);
    
    if (trBytes != len || ret < 0)
    {
        txPkt.setStatus(Packet::Status::RSP_SPI_LEN_MISMATCH);
    }
}

inline void SpiTask::processTransfer(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    int len = rxPkt.getPayloadItem8(1);

    uint8_t *inBuf = rxPkt.getPayloadBuffer(2); // skip params
    uint8_t *outBuf = txPkt.getPayloadBuffer();

    int trBytes = spi_write_read_blocking(hwInstance, inBuf, outBuf, len);

    int ret = txPkt.increasePayloadLength(trBytes);

    if (trBytes != len || ret < 0)
    {
        txPkt.setStatus(Packet::Status::ERR);
    }
}

inline void SpiTask::processSetFormat(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == SPI_0 ? spi0 : spi1;
    auto dataBits = rxPkt.getPayloadItem8(1);
    auto cpol = rxPkt.getPayloadItem8(2);
    auto cpha =rxPkt.getPayloadItem8(3);
    auto order = rxPkt.getPayloadItem8(4);
    spi_set_format(hwInstance, dataBits, (spi_cpol_t)cpol,  (spi_cpha_t)cpha, (spi_order_t)order);
}


void SpiTask::process(Packet &rxPkt,Packet &txPkt)
{
     

    CHECK_STATE();  

    auto rxPktType = rxPkt.getType();   

    switch (rxPktType)
    {
    case Packet::Type::SPI_INIT:
        processInit(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_DEINIT:
        processDeInit(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_SET_FREQ:
        processSetFreq(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_READ:
        processRead(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_WRITE:
        processWrite(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_TRANSFER: 
        processTransfer(rxPkt,txPkt);
    break;
    case Packet::Type::SPI_SET_FORMAT: 
        processSetFormat(rxPkt,txPkt);
    break;

    default:                   
        break;
    } 
                
        
}

