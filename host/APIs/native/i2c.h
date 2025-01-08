#pragma once

#include "ioig.h"

#ifdef IOIG_HOST   

namespace ioig
{

    /**
     * An I2C Master, used for communicating with I2C slave devices
     */
    class I2C : public Peripheral
    {
    
    public:

        enum Acknowledge
        {
            NoACK = 0,
            ACK = 1
        };

        /**
         * @brief Default constructor.
         *        Builds object using default I2C pinout.
         */
        I2C() : I2C(I2C0_PINOUT0) {};                


        /** 
         * @brief Create an I2C Master interface, connected to the specified pins.
         *
         * @param sda The pin for the data line (SDA).
         * @param scl The pin for the clock line (SCL).
         * @param freq_hz The frequency in hertz (default is 100000).
         * @param hw_instance The hardware instance (default is 0).
         */
        I2C(int sda, int scl, unsigned long freq_hz=100000, unsigned hw_instance=0);

        // Disable Copy Constructor and Copy Assignment
        I2C(const I2C&) = delete;
        I2C& operator=(const I2C&) = delete;
    

        // Enable Move Constructor
        I2C(I2C&& other) noexcept
            : Peripheral(std::move(other)),  // Move base class
            _sda(other._sda),
            _scl(other._scl),
            _freq(other._freq),
            _addr(other._addr),
            _hwInstance(other._hwInstance),
            _timeout(other._timeout) {}
    
        // Enable Move Assignment Operator
        I2C& operator=(I2C&& other) noexcept
        {
            if (this != &other) {
                Peripheral::operator=(std::move(other)); // Move base class
                _sda = other._sda;
                _scl = other._scl;
                _freq = other._freq;
                _addr = other._addr;
                _hwInstance = other._hwInstance;
                _timeout = other._timeout;
            }
            return *this;
        }

        ~I2C();

        /**
         * @brief Set the Timeout.
         * 
         * @param timeout The timeout value in microseconds.
         */
        void setTimeout(unsigned long timeout);        


        /** 
         * @brief Set the frequency of the I2C interface.
         *
         * @param hz The bus frequency in hertz.
         */
        void setFrequency(int hz);

        /** Read from an I2C slave
         *
         * Performs a complete read transaction. The bottom bit of
         * the address is forced to 1 to indicate a read.
         *
         *  @param address 8-bit I2C slave address [ addr | 1 ]
         *  @param data Pointer to the byte-array to read data in to
         *  @param length Number of bytes to read
         *  @param nostop Repeated start, true - don't send stop at end
         *         default value is false.
         *
         *  @returns 0 on success, -1 on NAC, -2 on Timeout, -3 unknown error
         */
        int read(int address, uint8_t *data, int length, bool nostop = false);


        /** Write to an I2C slave
         *
         * Performs a complete write transaction. The bottom bit of
         * the address is forced to 0 to indicate a write.
         *
         *  @param address 8-bit I2C slave address [ addr | 0 ]
         *  @param data Pointer to the byte-array data to send
         *  @param length Number of bytes to send
         *  @param nostop Repeated start, true - do not send stop at end
         *         default value is false.
         *
         *  @returns 0 on success, -1 on NAC, -2 on Timeout, -3 unknown error
         */
        int write(int address, const uint8_t *data, int length, bool nostop = false);


        void set_addr(int addr) { _addr = addr; }
        int get_addr() { return _addr; }
        
        
    private:
            
        void initialize() override;

        int      _sda;
        int      _scl;
        uint32_t _freq;
        int      _addr;
        int      _hwInstance;
        uint32_t _timeout;

        static constexpr const char* TAG = "I2C";

    };

}
#endif