#include "ioig_private.h"
#include "gpio.h"

using namespace ioig;
using namespace std::chrono_literals;


class ioig::GpioImpl : public EventHandler 
{
public:
    GpioImpl(Gpio& parent): _parent(parent), _eventMask(0), _eventCallback(nullptr) {}
    ~GpioImpl() {  UsbManager::removeEventHandler(this, _parent._usbPort ); };
    
    void onEvent(Packet &eventPkt) override
    {
        int event_cnt = eventPkt.getPayloadItem8(0);
        auto pktType = eventPkt.getType();

        if (pktType != Packet::Type::GPIO_EVENT)
        {
            return;
        }

        for (int i = 1; i < event_cnt * 4; i += 4) // skip four bytes on every iteration
        {
            uint32_t gpio_evt = eventPkt.getPayloadItem32(i);

            uint32_t evt_pin = ((gpio_evt >> 16) & 0xFFFF); // hi
            uint32_t evts = (gpio_evt & 0xFFFF);            // lo

            uint32_t evtFilter = evts & _eventMask;

            if (_parent._pin == (int)evt_pin && evtFilter != 0 && _eventCallback != nullptr)
            {
                _eventCallback(evt_pin, evts, _callbackArg);
            }
        }
    }

    Gpio & _parent;
    uint32_t _eventMask;
    Gpio::InterruptHandler _eventCallback;
    void * _callbackArg;
};



Gpio::Gpio(int pin, int direction, int mode) 
    : _pin(pin),
      _dir(direction),
      _mode(mode),
      pimpl(std::make_unique<GpioImpl>(*this))
{

    if (_pin >= TARGET_PINS_COUNT) 
    {
        LOG_ERR(TAG, "Invalid pin %d, max = %d", _pin, TARGET_PINS_COUNT-1);
    }   

    if (_dir != PinDirection::Input && _dir != PinDirection::Output) 
    {
        LOG_ERR(TAG, "Invalid pin direction");
    }

    if (_mode < PullNone || _mode > OpenDrainPullDown) 
    {
        LOG_ERR(TAG, "Invalid pin mode");
    }
}

Gpio::~Gpio()
{
    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_DEINIT);    

}

void Gpio::initialize()
{
    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::GPIO_INIT);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(_mode);
    auto txp2 = txPkt.addPayloadItem8(_dir);
          
    UsbManager::transfer(txPkt, rxPkt, _usbPort);    

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);
    auto rxp2 = rxPkt.getPayloadItem8(2);


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
        LOG_ERR(TAG, "Invalid response from device ( direction ) : expected = %d, received = %d", txp2, rxp2);
    }        
}


void Gpio::write(int value)
{
    checkAndInitialize();

    if ( _dir == PinDirection::Input )
    {
        LOG_ERR(TAG, "Can't write on input pin");
    }

    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_SET_VALUE);    
    txPkt.addPayloadItem8(_pin);
    txPkt.addPayloadItem8(value);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

}

int Gpio::read()
{
    checkAndInitialize();

    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_GET_VALUE);
    txPkt.addPayloadItem8(_pin);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    return rxPkt.getPayloadItem8(0);    
}

void Gpio::output()
{
    checkAndInitialize();

    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_SET_DIR);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8((uint8_t)PinDirection::Output);
    
    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( dir ) : expected = %d, received = %d", txp1, rxp1);
    }      

}

void Gpio::input()
{
    checkAndInitialize();

    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_SET_DIR);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8((uint8_t)PinDirection::Input);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);    

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( dir ) : expected = %d, received = %d", txp1, rxp1);
    }   
}


void Gpio::mode(int pull)
{
    checkAndInitialize();

    Packet txPkt(4);
    Packet rxPkt(4);

    txPkt.setType(Packet::Type::GPIO_SET_MODE);
    
    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(_mode = pull);
  
    
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

void ioig::Gpio::setStrength(int strength)
{
    (void)strength;
    //TODO:
}

unsigned long Gpio::pulseIn(uint8_t state, unsigned long timeout)
{
    checkAndInitialize();

    Packet txPkt;
    Packet rxPkt;

    txPkt.setType(Packet::Type::GPIO_PULSE_IN);
    
    txPkt.addPayloadItem8(_pin);
    txPkt.addPayloadItem8(state);
    txPkt.addPayloadItem64(timeout);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    return rxPkt.getPayloadItem64(0);
}

void Gpio::setInterrupt(const uint32_t events, const InterruptHandler &cbk, void * arg)
{ 
    checkAndInitialize();

    pimpl->_callbackArg = arg;

    Packet txPkt(16);
    Packet rxPkt(16);

    txPkt.setType(Packet::Type::GPIO_SET_IRQ);

    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(1); //enable interrupt
    auto txp2 = txPkt.addPayloadItem32(events);

    pimpl->_eventMask = events;    
    pimpl->_eventCallback = cbk;


    UsbManager::registerEventHandler( pimpl.get() , _usbPort);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1); 
    auto rxp2 = rxPkt.getPayloadItem32(2);

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( enable ) : expected = %d, received = %d", txp1, rxp1);
    }    

    if (  txp2 != rxp2  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( events ) : expected = %d, received = %d", txp2, rxp2);
    }
}


void Gpio::disableInterrupt()
{
    checkAndInitialize();

    pimpl->_callbackArg = nullptr;

    //TODO: usbDevice.removeEventHandler

    Packet txPkt(16);
    Packet rxPkt(16);

    txPkt.setType(Packet::Type::GPIO_SET_IRQ);

    auto txp0 = txPkt.addPayloadItem8(_pin);
    auto txp1 = txPkt.addPayloadItem8(0); //disable interrupt
    auto txp2 = txPkt.addPayloadItem32(0);

    UsbManager::transfer(txPkt, rxPkt, _usbPort);

    auto rxp0 = rxPkt.getPayloadItem8(0);
    auto rxp1 = rxPkt.getPayloadItem8(1);
    auto rxp2 = rxPkt.getPayloadItem32(2);    

    if (  txp0 != rxp0  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( pin ) : expected = %d, received = %d", txp0, rxp0);
    }

    if (  txp1 != rxp1  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( enable ) : expected = %d, received = %d", txp1, rxp1);
    }    

    if (  txp2 != rxp2  ) 
    {
        LOG_ERR(TAG, "Invalid response from device ( events ) : expected = %d, received = %d", txp2, rxp2);
    }    
}

