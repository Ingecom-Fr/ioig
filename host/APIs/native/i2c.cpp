#include <iostream>

#include "ioig_private.h"
#include "i2c.h"

using namespace ioig;
using namespace std::chrono_literals;


I2C::I2C(int sda, int scl, unsigned long freq_hz, unsigned hw_instance)
    :_sda(sda),
     _scl(scl),
     _freq(freq_hz),
     _addr(0),
     _hwInstance(hw_instance)
{  

    if (_sda >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid sda pin %d, max = %d", _sda, TARGET_PINS_COUNT-1);
    }       
    
    if (_scl >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid scl pin %d, max = %d", _scl, TARGET_PINS_COUNT-1);
    } 

    if (_hwInstance >= I2C_INSTANCES) 
    {
        LOG_ERR(TAG, "Invalid I2C hardware instance %d, max = %d, using 0...", _hwInstance, I2C_INSTANCES-1);
        _hwInstance = I2C_0;
    } 

    if (_freq == 0 /*TODO: range*/) 
    {
        LOG_ERR(TAG, "Invalid freq %d, using = 100000 Hz", _freq);
        _freq = 1000000;
    }

}

void I2C::initialize()
{
    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::I2C_INIT);
    auto txp0 = txPkt.addPayloadItem8(_hwInstance);
    auto txp1 = txPkt.addPayloadItem8(_sda);
    auto txp2 = txPkt.addPayloadItem8(_scl);
    auto txp3 = txPkt.addPayloadItem32(_freq);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);        

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);
    auto rxp2 = rxPkt.getPayloadItem8(2);
    auto rxp3 = rxPkt.getPayloadItem32(3);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( sda ) : expected = %d, received = %d", txp1, rxp1);
    }    

    if (  txp2 != rxp2  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( scl ) : expected = %d, received = %d", txp2, rxp2);
    }  

    if (  txp3 != rxp3  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( freq ) : expected = %d, received = %d", txp3, rxp3);        
    }  

}

I2C::~I2C()
{
    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::I2C_DEINIT);   
}

void I2C::setFrequency(int hz)
{
    checkAndInitialize();

    Packet txPkt(8);
    Packet rxPkt(8);

    txPkt.setType(Packet::Type::I2C_SET_FREQ);
    auto txp0 = txPkt.addPayloadItem8(_hwInstance);
    auto txp1 = txPkt.addPayloadItem32(hz);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);    

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem32(1);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( frequency ) : expected = %d, received = %d", txp1, rxp1);
    }       
}

void I2C::setTimeout(unsigned long timeout)
{
    checkAndInitialize();

    Packet txPkt(8);
    Packet rxPkt(8);

    txPkt.setType(Packet::Type::I2C_SET_TIMEOUT);
    auto txp0 = txPkt.addPayloadItem32(timeout);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort); 
    
    auto rxp0 = rxPkt.getPayloadItem32(0);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( timeout ) : expected = %d, received = %d", txp0, rxp0);
    }        
}

int I2C::read(int address, uint8_t *data, int length, bool nostop)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::I2C_READ);
    txPkt.addPayloadItem8(_hwInstance);  
    txPkt.addPayloadItem8(address);
    txPkt.addPayloadItem8(length); 
    txPkt.addPayloadItem8(nostop); 
       
    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    if (rxPkt.getPayloadLength() > (size_t)length) 
    {
        LOG_ERR(TAG, "Invalid rx length : %d", length);
        return -1;
    }
    
    memcpy(data, rxPkt.getPayloadBuffer(), rxPkt.getPayloadLength());

    switch (rxPkt.getStatus())
    {
    case Packet::Status::RSP:
        return 0;
    case Packet::Status::RSP_I2C_NACK:
        return -1;        
    case Packet::Status::RSP_I2C_TIMEOUT:
        return -2;
    default:
        return -3;
    } 
}


int I2C::write(int address, const uint8_t *data, int length, bool nostop)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::I2C_WRITE);
    txPkt.addPayloadItem8(_hwInstance);        
    txPkt.addPayloadItem8(address);
    txPkt.addPayloadItem8(length);    
    txPkt.addPayloadItem8(nostop); 

    if (txPkt.addPayloadBuffer(data,length) < 0)
    {
        LOG_ERR(TAG, "Wr buffer overflow, requested %d bytes, available %d bytes", length, txPkt.getFreePayloadSlots());
    }  

    UsbManager::transfer(txPkt, rxPkt, _usbPort);        
    
    switch (rxPkt.getStatus())
    {
    case Packet::Status::RSP:
        return 0;
    case Packet::Status::RSP_I2C_NACK:
        return -1;        
    case Packet::Status::RSP_I2C_TIMEOUT:
        return -2;
    default:
        return -3;
    }    

}

