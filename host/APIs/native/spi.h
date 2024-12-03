/**
 * @file spi.h
 * @brief Definition of Spi class for Spi communication.
 */

#pragma once

#ifdef IOIG_HOST

#include <cstdint>
#include <cstddef>

namespace ioig
{

    /**
     * @class Spi
     * @brief Represents an Spi (Serial Peripheral Interface) communication interface.
     *
     * The default format is set to 8-bits, mode 0, and a clock frequency of 1MHz.
     *
     * @note Synchronization level: Thread safe
     */
    class Spi : public Peripheral
    {
    public:
        /**
         * @brief Construct a new Spi object using default Spi pinout.
         */
        Spi() : Spi(SPI0_PINOUT0) {};

        /**
         * @brief Construct a new Spi object.
         * @param sclk Spi clock pin
         * @param tx Spi transmit pin
         * @param rx Spi receive pin
         * @param cs Spi chip select pin
         * @param freq_hz Clock frequency in Hz
         * @param hw_instance Hardware instance number
         */
        Spi(int sclk, int tx, int rx, int cs = -1, unsigned long freq_hz = 1000000, unsigned hw_instance = 0);

        // Disable Copy Constructor and Copy Assignment
        Spi(const Spi &) = delete;
        Spi &operator=(const Spi &) = delete;

        // Enable Move Constructor
        Spi(Spi &&other) noexcept
            : Peripheral(std::move(other)), // Move base class
              _sclk(other._sclk),
              _tx(other._tx),
              _rx(other._rx),
              _cs(other._cs),
              _freq(other._freq),
              _hwInstance(other._hwInstance)
        {
        }

        // Enable Move Assignment Operator
        Spi &operator=(Spi &&other) noexcept
        {
            if (this != &other)
            {
                Peripheral::operator=(std::move(other)); // Move base class
                _sclk = other._sclk;
                _tx = other._tx;
                _rx = other._rx;
                _cs = other._cs;
                _freq = other._freq;
                _hwInstance = other._hwInstance;
            }
            return *this;
        }
        
        /**
         * @brief Configure Spi.
         *
         * Configure how the Spi serializes and deserializes data.
         *
         * @param data_bits Number of data bits per transfer. Valid values: 4..16.
         * @param cpol SSPCLKOUT polarity, applicable to Motorola Spi frame format only.
         * @param cpha SSPCLKOUT phase, applicable to Motorola Spi frame format only.
         * @param order Must be SPI_MSB_FIRST=0, no other values supported on the PL022.
         */
        void format(unsigned data_bits, unsigned cpol = 0, unsigned cpha = 0, unsigned order = 0);

        /**
         * @brief Set Spi frequency
         * @param The frequency in Hz
         */
        void set_freq(uint32_t freq);

        /**
         * @brief Transfer Spi data.
         *
         * @note Blocking operation.
         *
         * @param tx_buffer The TX buffer with data to be transferred. If NULL is passed,
         *                  the default Spi value is sent.
         * @param rx_buffer The RX buffer which is used for received data. If NULL is passed,
         *                  received data are ignored.
         * @param length Length of BOTH buffers.
         */
        int transfer(const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t length);

        /**
         * @brief Transfer repeated Spi data.
         *
         * @note Blocking operation.
         *
         * @param val       The repeated byte to transfer.
         * @param rx_buffer The RX buffer which is used for received data.
         * @param length    The length of RX buffer in bytes.
         */
        int transfer(const uint8_t val, uint8_t *rx_buffer, size_t length);

        /**
         * @brief Write Spi data.
         *
         * @note Blocking operation.
         *
         * @param buf       The buffer to be sent.
         * @param length    The length of buffer in bytes.
         */
        int write(const uint8_t *buf, size_t length);

        /**
         * @brief Read Spi data.
         *
         * @note Blocking operation.
         *
         * @param buf The buffer to receive read data.
         * @param len The buffer length.
         * @param repeated_tx_data Data to send during read operations.
         */
        int read(uint8_t *buf, size_t len, uint8_t repeated_tx_data = 0);

        /**
         * @brief Transfer a single byte.
         *
         * @note Blocking operation.
         *
         * @param b  The byte to transfer.
         */
        int write(const uint8_t b);

    private:
        /**
         * @brief Initializes Spi object.
         */
        void initialize() override;

        int _sclk;       ///< Spi clock pin
        int _tx;         ///< Spi transmit pin
        int _rx;         ///< Spi receive pin
        int _cs;         ///< Spi chip select pin
        uint32_t _freq;  ///< Clock frequency in Hz
        int _hwInstance; ///< Hardware instance identifier

        static constexpr const char *TAG = "Spi"; ///< Log tag
    };

} // namespace ioig

#endif