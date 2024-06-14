#include "ioig_private.h"
#include <iostream>
#include "analog.h"

using namespace ioig;



AnalogIn::AnalogIn(int pin, unsigned resolution, unsigned channel) 
   : _pin(pin) , 
     _channel(channel), 
     _resolution(resolution)
{

    if (_pin != A0 && _pin != A1 && _pin != A2 && _pin != A3 && _pin != ADC_TEMP)
    {
        LOG_ERR(TAG, "Invalid pin %d, analog pins = %d,%d,%d,%d,%d(onboard temp)", _pin,  A0,A1,A2,A3, ADC_TEMP);
    }

    if (_resolution <= 0  || _resolution >= 32) 
    {        
        LOG_ERR(TAG, "Invalid resolution, using 10 bits resolution");
        _resolution = 10;
    }   

    if (_pin == ADC_TEMP && _channel == 4)  //Onboard Temp sensor
    {
        return; 
    }

    if (_channel > 3) 
    {
        LOG_ERR(TAG, "Invalid channel, using channel 0");
        _channel = 0;
    } 
}


AnalogIn::~AnalogIn()
{    
    Packet txPkt;
    Packet rxPkt;
    txPkt.setType(Packet::Type::ANALOG_DEINIT);
    txPkt.addPayloadItem8(_pin);
    txPkt.addPayloadItem8(0); 

    UsbManager::transfer(txPkt, rxPkt, _usbPort);
}


void AnalogIn::initialize()
{
    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::ANALOG_INIT);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(0); //mode 0 = ADC , 1 = PWM

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( mode ) : expected = %d, received = %d", txp1, rxp1);
    }
}


uint16_t AnalogIn::read_u16()
{
    checkAndInitialize();

    Packet txPkt(8);
    Packet rxPkt(16);

    txPkt.setType(Packet::Type::ANALOG_READ);    
    txPkt.addPayloadItem8(_channel);
    txPkt.addPayloadItem8(_resolution);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);

    if (rxp0 != _channel) 
    {
        LOG_ERR(TAG, "Invalid response from device ( channel ) : expected = %d, received = %d", _channel, rxp0);
    }

    uint16_t readVal = rxPkt.getPayloadItem16(1);

    return readVal;
}


float AnalogIn::readOnboardTemp()
{
    checkAndInitialize();

    Packet txPkt(8);
    Packet rxPkt(8);

    txPkt.setType(Packet::Type::ANALOG_READ_TEMP);    

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    return rxPkt.getPayloadItemFloat(0);
}



/*==================================================================================*/

AnalogOut::AnalogOut(int pin, unsigned resolution): 
        _pin(pin),
        _resolution(resolution),  
        _pwmSlice(0),
        _pwmCountTop(1000), 
        _pwmPercent(0.5f),
        _pwmPeriod_us(2000 /*500Hz*/)
{   
    if (_pin >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid pin");
    }

    if (_resolution > 32 /*TODO:*/) 
    {        
        LOG_ERR(TAG, "Invalid resolution, using 8 bits resolution");
        _resolution = 8;
    }
}

AnalogOut::~AnalogOut()
{
    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::ANALOG_DEINIT);
    txPkt.addPayloadItem8(_pin);
    txPkt.addPayloadItem8(1); //PWM Mode
    txPkt.addPayloadItem8(_pwmSlice); 

    UsbManager::transfer(txPkt, rxPkt, _usbPort);
}

void AnalogOut::initialize()
{
    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::ANALOG_INIT);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(1); //PWM Mode
    auto txp2 = txPkt.addPayloadItem32(_pwmCountTop); 
    auto txp3 = txPkt.addPayloadItem64(_pwmPeriod_us); 

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);   //pin
    auto rxp1 = rxPkt.getPayloadItem8(1);   //mode
    auto rxp2 = rxPkt.getPayloadItem32(2);  //count top
    auto rxp3 = rxPkt.getPayloadItem64(6);  //period us
    _pwmSlice = rxPkt.getPayloadItem8(14);  //slice
    auto rxp4 = rxPkt.getPayloadItem8(15);  //channel


    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }


    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( mode ) : expected = %d, received = %d", txp1, rxp1);
    }

    if (  txp2 != rxp2  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( count top ) : expected = %d, received = %d", txp2, rxp2);
    }

    if ( txp3 != rxp3 ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( period ) : expected = %lld, received = %lld", (long long int)txp3, (long long int)rxp3);
    }

    constexpr int max_slices = 7; /*datasheet*/ 
    if ( _pwmSlice >  max_slices) 
    {
        LOG_ERR(TAG, "Invalid response from device : slice");
    }

    constexpr int max_channels = 1; /* datasheet : 0 = A , 1  = B*/
    if (rxp4 > max_channels) 
    {
        LOG_ERR(TAG, "Invalid response from device : channel");
    }

}

void AnalogOut::write_u16(uint16_t value)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt(16);

    txPkt.setType(Packet::Type::ANALOG_WRITE);
    
    txPkt.addPayloadItem8(_pin);
    txPkt.addPayloadItem32(_pwmSlice);
    txPkt.addPayloadItem16(value);
    txPkt.addPayloadItem32(_pwmCountTop);
    txPkt.addPayloadItem8(_resolution);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    _pwmPercent = rxPkt.getPayloadItemFloat(0);    
    
}

void AnalogOut::setWriteResolution(int bits)
{
    checkAndInitialize();

    if (_resolution <= 32) 
    {
        _resolution = bits;
    }else 
    {
        LOG_ERR(TAG, "Invalid resolution, using 8 bits resolution");
        _resolution = 8;
    }
}
