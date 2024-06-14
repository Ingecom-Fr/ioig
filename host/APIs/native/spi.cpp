#include <iostream>

#include "ioig_private.h"
#include "i2c.h"

using namespace ioig;

Spi::Spi(int sclk, int tx, int rx, int cs, unsigned long freq_hz, unsigned hw_instance)
       :_sclk(sclk),
        _tx(tx),
        _rx(rx),        
        _cs(cs),
        _freq(freq_hz),
        _hwInstance(hw_instance)
{
   
    if (_sclk < 0 || _sclk >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid sclk pin %d, max = %d", _sclk, TARGET_PINS_COUNT-1);
    }    

    if (_tx < 0 || _tx >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid tx pin %d, max = %d", _tx, TARGET_PINS_COUNT-1);
    }   

    if (_rx < 0 || _rx >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid rx pin %d, max = %d", _rx, TARGET_PINS_COUNT-1);
    }   

    if (_hwInstance >= UART_INSTANCES) 
    {
        LOG_ERR(TAG, "Invalid SPI hardware instance %d, max = %d, using 0...", _hwInstance, UART_INSTANCES-1);
        _hwInstance = SPI_0;
    } 
}

void Spi::initialize()
{
    Packet txPkt;
    Packet rxPkt;
 

    txPkt.setType(Packet::Type::SPI_INIT);
    auto txp0 = txPkt.addPayloadItem8(_hwInstance);
    auto txp1 = txPkt.addPayloadItem8(_sclk);
    auto txp2 = txPkt.addPayloadItem8(_tx);
    auto txp3 = txPkt.addPayloadItem8(_rx);
    auto txp4 = txPkt.addPayloadItem8(_cs);
    auto txp5 = txPkt.addPayloadItem32(_freq);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);   
   

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);
    auto rxp2 = rxPkt.getPayloadItem8(2);
    auto rxp3 = rxPkt.getPayloadItem8(3);
    auto rxp4 = rxPkt.getPayloadItem8(4);
    auto rxp5 = txPkt.getPayloadItem32(5);


    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( sclk ) : expected = %d, received = %d", txp1, rxp1);
    }    

    if (  txp2 != rxp2  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( tx ) : expected = %d, received = %d", txp2, rxp2);
    }  

    if (  txp3 != rxp3  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( rx ) : expected = %d, received = %d", txp3, rxp3);        
    }  

    if (  txp4 != rxp4  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( rx ) : expected = %d, received = %d", txp4, rxp4);        
    }  

    if (  txp5 != rxp5  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( rx ) : expected = %d, received = %d", txp5, rxp5);        
    }          

}

void Spi::format(unsigned data_bits, unsigned cpol, unsigned cpha, unsigned order)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::SPI_SET_FORMAT);
    txPkt.addPayloadItem8(_hwInstance);        
    txPkt.addPayloadItem8(data_bits);
    txPkt.addPayloadItem8(cpol);
    txPkt.addPayloadItem8(cpha);
    txPkt.addPayloadItem8(order);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);    
}


int Spi::transfer(const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t length)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::SPI_TRANSFER);
    txPkt.addPayloadItem8(_hwInstance);    
    txPkt.addPayloadItem8(length);

    if (txPkt.addPayloadBuffer(tx_buffer, length) < 0)
    {
        LOG_ERR(TAG, "Transfer buffer overflow, requested %d bytes, available %d bytes", (int)length, txPkt.getFreePayloadSlots());
    }      

    UsbManager::transfer(txPkt, rxPkt, _usbPort); 

    if ( rxPkt.getStatus() == Packet::Status::RSP ) 
    {        
        memcpy(rx_buffer, rxPkt.getPayloadBuffer(), rxPkt.getPayloadLength());
        return rxPkt.getPayloadLength();
    }    

    return -1;        
}

int Spi::transfer(const uint8_t val, uint8_t *rx_buffer, size_t length)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::SPI_TRANSFER);
    txPkt.addPayloadItem8(_hwInstance);        
    txPkt.addPayloadItem8(length);  

    if ( txPkt.addRepeatedPayloadItems(val, length) < 0 ) 
    {
        LOG_ERR(TAG, "Repeated transfer buffer overflow, requested %d bytes, available %d bytes", (int)length, txPkt.getFreePayloadSlots());
    }

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    if ( rxPkt.getStatus() == Packet::Status::RSP ) 
    {
        memcpy(rx_buffer, rxPkt.getPayloadBuffer(), rxPkt.getPayloadLength());
        return rxPkt.getPayloadLength();
    }    

    return -1;    
}

int Spi::write(const uint8_t *buf, size_t length)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::SPI_WRITE);
    txPkt.addPayloadItem8(_hwInstance); 
    txPkt.addPayloadItem8(length);  

    if (txPkt.addPayloadBuffer(buf,length) < 0)
    {
        LOG_ERR(TAG, "Write buffer overflow, requested %d bytes, available %d bytes", (int)length, txPkt.getFreePayloadSlots());
    }  

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    if ( rxPkt.getStatus() == Packet::Status::RSP ) 
    {
        return length;
    }

    return -1;
}


int Spi::write(const uint8_t b)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::SPI_WRITE);
    txPkt.addPayloadItem8(_hwInstance);        
    txPkt.addPayloadItem8(1);    
    txPkt.addPayloadItem8(b);

    UsbManager::transfer(txPkt, rxPkt, _usbPort); 

    if ( rxPkt.getStatus() == Packet::Status::RSP ) 
    {
        return rxPkt.getPayloadItem8(0);
    }
    
    return -1;    
}

int Spi::read(uint8_t *buf, size_t len, uint8_t repeated_tx_data)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;    

    txPkt.setType(Packet::Type::SPI_READ);
    txPkt.addPayloadItem8(_hwInstance);        
    txPkt.addPayloadItem8(len);
    txPkt.addPayloadItem8(repeated_tx_data); 
     
    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    if ( rxPkt.getStatus() == Packet::Status::RSP ) 
    {
        memcpy(buf, rxPkt.getPayloadBuffer(), rxPkt.getPayloadLength());
        return rxPkt.getPayloadLength();
    }

    return -1;
    
}

