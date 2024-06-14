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
       
        /**
         * @brief Construct AnalogIn object using default settings.
         */
        AnalogIn() : AnalogIn(ANALOGIN0) {};

        /** 
         * @brief Create an AnalogIn connected to the specified pin.
         *
         * @param pin The pin to which the AnalogIn is connected.
         * @param resolution The resolution of the AnalogIn (default is 10).
         * @param channel The channel of the AnalogIn (default is 0).
         */
        AnalogIn(int pin, unsigned resolution=10, unsigned channel=0);

        // Copy constructor
        AnalogIn(const AnalogIn &) = default;

        // Assignment operator
        AnalogIn &operator=(const AnalogIn &other) = default;

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
        static constexpr char TAG[] = "AnalogIn"; /**< Tag for debugging purposes. */
    };

    /**
     * @brief Analog Output class
     */
    class AnalogOut : public Peripheral
    {

    public:

        /**
         * @brief Construct the AnalogOut object using default pin.
         */
        AnalogOut() : AnalogOut(PWM0_A) {};

        /** 
         * @brief Create an AnalogOut connected to the specified pin.
         *
         * @param pin The pin to which the AnalogOut is connected.
         * @param resolution The resolution of the AnalogOut (default is 8).
         */
        AnalogOut(int pin, unsigned resolution=8);

        // Assignment operator
        AnalogOut &operator=(const AnalogOut &other) = default;

        // Copy constructor
        AnalogOut(const AnalogOut &) = default;

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

        static constexpr char TAG[] = "AnalogOut"; /**< Tag for debugging purposes. */
    };    

} // namespace

#endif
