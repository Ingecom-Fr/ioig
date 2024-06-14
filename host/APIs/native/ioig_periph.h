#pragma once

#ifdef IOIG_HOST

#include <bitset>
#include <mutex>

namespace ioig
{
        class Peripheral
        {
        public:

            Peripheral();

            virtual ~Peripheral();
            
            virtual void initialize() = 0;

            void checkAndInitialize();

            void attachToUsbPort(unsigned usb_port);

                
        protected:
            unsigned _usbPort;

        
        private:
            std::bitset<MAX_USB_DEVICES> _usbInitMap;  /**< Each bit represents an usb index */
            std::mutex _mutex;
        
        };     
}

#endif