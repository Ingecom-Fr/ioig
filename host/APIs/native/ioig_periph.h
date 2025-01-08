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

            // Disable Copy Constructor and Copy Assignment
            Peripheral(const Peripheral&) = delete;
            Peripheral& operator=(const Peripheral&) = delete;
    
            // Enable Move Constructor
            Peripheral(Peripheral&& other) noexcept
                : _usbPort(other._usbPort),
                  _usbInitMap(std::move(other._usbInitMap)) // Move bitset
            {
                // `_mutex` is not transferable; each object gets its own mutex
            }
    
            // Enable Move Assignment Operator
            Peripheral& operator=(Peripheral&& other) noexcept
            {
                if (this != &other) {
                    std::lock_guard<std::mutex> lock(_mutex); // Ensure thread-safety
                    _usbPort = other._usbPort;
                    _usbInitMap = std::move(other._usbInitMap); // Move bitset
                    // `_mutex` is not transferable.
                }
                return *this;
            }

            virtual ~Peripheral() = default;
            
            virtual void initialize() = 0;

            void checkAndInitialize();

            void attachToUsbPort(unsigned usb_port);

            unsigned getUsbPort() { return _usbPort; };

                
        protected:
            unsigned _usbPort;

        
        private:
            std::bitset<MAX_USB_DEVICES> _usbInitMap;  /**< Each bit represents an usb index */
            std::mutex _mutex;
        
        };     
}

#endif