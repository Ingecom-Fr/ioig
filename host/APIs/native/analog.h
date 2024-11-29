#pragma once

#include "ioig.h"

#ifdef IOIG_HOST

namespace ioig
{

    /**
     * @brief Analog Input Class
     */
    class AnalogIn : public Peripheral
    {

    public:
       
        AnalogIn() : AnalogIn(ANALOGIN0) {}
        AnalogIn(int pin, unsigned resolution = 10, unsigned channel = 0);
    
        // Disable Copy Constructor and Copy Assignment
        AnalogIn(const AnalogIn&) = delete;
        AnalogIn& operator=(const AnalogIn&) = delete;
    
        // Enable Move Constructor
        AnalogIn(AnalogIn&& other) noexcept
            : Peripheral(std::move(other)),  // Move base class
              _pin(other._pin),
              _channel(other._channel),
              _resolution(other._resolution) {}
    
        // Enable Move Assignment Operator
        AnalogIn& operator=(AnalogIn&& other) noexcept
        {
            if (this != &other) {
                Peripheral::operator=(std::move(other)); // Move base class
                _pin = other._pin;               
                _channel = other._channel;
                _resolution = other._resolution;
            }
            return *this;
        }
    
        ~AnalogIn();

        /** 
         * @brief Read the input voltage.
         *
         * @returns A 16-bit unsigned integer representing the current input voltage, normalized to a 16-bit value.
         */
        uint16_t read_u16();

        /**
         * @brief Read onboard temperature sensor.
         * 
         * @return The temperature in Celsius as a float.
         */
        float readOnboardTemp();

    private:
        void initialize() override;

        int      _pin;         /**< The pin to which the AnalogIn is connected. */
        uint8_t _channel;      /**< The channel of the AnalogIn. */
        uint8_t _resolution;   /**< The resolution of the AnalogIn. */
        static constexpr const char* TAG = "AnalogIn"; /**< Tag for debugging purposes. */
    };

    /**
     * @brief Analog Output class
     */
    class AnalogOut : public Peripheral
    {

    public:

        AnalogOut() : AnalogOut(PWM0_A) {}
        AnalogOut(int pin, unsigned resolution = 8);            
    
        // Disable Copy Constructor and Copy Assignment
        AnalogOut(const AnalogOut&) = delete;
        AnalogOut& operator=(const AnalogOut&) = delete;
    
        // Enable Move Constructor
        AnalogOut(AnalogOut&& other) noexcept
            : Peripheral(std::move(other)),  // Move base class
              _pin(other._pin),
              _resolution(other._resolution),
              _pwmSlice(other._pwmSlice),
              _pwmCountTop(other._pwmCountTop),
              _pwmPercent(other._pwmPercent),
              _pwmPeriod_us(other._pwmPeriod_us) {}
    
        // Enable Move Assignment Operator
        AnalogOut& operator=(AnalogOut&& other) noexcept
        {
            if (this != &other) {
                Peripheral::operator=(std::move(other)); // Move base class
                _pin = other._pin;
                _resolution = other._resolution;
                _pwmSlice = other._pwmSlice;
                _pwmCountTop = other._pwmCountTop;
                _pwmPercent = other._pwmPercent;
                _pwmPeriod_us = other._pwmPeriod_us;
            }
            return *this;
        }
    
        ~AnalogOut();


        /** 
         * @brief Set the output voltage.
         *
         * @param value A 16-bit unsigned short representing the output voltage,
         *              normalized to a 16-bit value (0x0000 = 0v, 0xFFFF = 3.3v).
         */
        void write_u16(uint16_t value);

        /**
         * @brief Set the resolution for writing.
         * 
         * @param bits The resolution in bits.
         */
        void setWriteResolution(int bits);        

    private:
        void initialize() override;

        int      _pin;         /**< The pin to which the AnalogOut is connected. */
        uint8_t  _resolution;  /**< The resolution of the AnalogOut. */
        uint32_t _pwmSlice;    /**< Slice for PWM operation. */
        uint32_t _pwmCountTop; /**< Count top for PWM operation. */
        float    _pwmPercent;  /**< Percentage for PWM operation. */
        uint64_t _pwmPeriod_us; /**< Period in microseconds for PWM operation. */

        static constexpr const char* TAG = "AnalogOut"; /**< Tag for debugging purposes. */
    };    

} // namespace

#endif
