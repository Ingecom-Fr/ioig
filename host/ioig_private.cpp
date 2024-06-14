#include "ioig_private.h"
#include <mutex>

using namespace ioig;


Peripheral::Peripheral() : _usbPort(0)
{
   
}

Peripheral::~Peripheral()
{
   
}


void Peripheral::checkAndInitialize()
{    
    std::lock_guard<std::mutex> lock(_mutex);

    if (_usbInitMap.test(_usbPort))     
    {   
        return;
    }
    
    _usbInitMap.set(_usbPort);    
    initialize();                
}

void Peripheral::attachToUsbPort(unsigned usb_port)
{    
    if (usb_port < MAX_USB_DEVICES) 
    {    
       _usbPort = usb_port;
       checkAndInitialize();
    }else 
    {
        LOG_ERR("USB Manager", "Can't attach to usb index %d, max index %d" , usb_port, MAX_USB_DEVICES-1 );
    }
}