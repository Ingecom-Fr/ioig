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

        //Copy constructor
        I2C(const I2C& other) = default;
        
        //Assignment operator 
        I2C &operator=(const I2C &other) = default;


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
        
        
        //Arduino API : TODO: move it to api implementation
        void begin();    
        void begin(uint8_t address); //slave
        void end();  
        void beginTransmission(uint8_t address);
        uint8_t endTransmission(bool stopBit);
        uint8_t endTransmission(void);
        size_t requestFrom(uint8_t address, size_t len, bool stopBit);
        size_t requestFrom(uint8_t address, size_t len);
        size_t write(uint8_t data);
        size_t write(const uint8_t* data, int len);
        int read();
        int peek();
        void flush();
        int available();
        
    private:
            
        void initialize() override;

        int      _sda;
        int      _scl;
        uint32_t _freq;
        int      _addr;
        int      _hwInstance;
        uint32_t _timeout;

        static constexpr char TAG[] = "I2C";
    };

}
#endif