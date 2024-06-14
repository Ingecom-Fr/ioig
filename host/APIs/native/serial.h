#pragma once

#include "ioig.h"

#ifdef IOIG_HOST  
#include <functional>
#include <memory>
#endif

namespace ioig
{
    /**
     * @brief Enumeration for parity modes in serial communication.
     */
    typedef enum
    {
        ParityNone = 0,    /**< No parity */
        ParityOdd = 1,     /**< Odd parity */
        ParityEven = 2,    /**< Even parity */
        ParityForced1 = 3, /**< Forced parity 1 */
        ParityForced0 = 4  /**< Forced parity 0 */
    } Parity;

    /**
     * @brief Enumeration for serial interrupt sources.
     */
    typedef enum
    {
        RxIrq = 0, /**< Receive Data Register Full */
        TxIrq      /**< Transmit Data Register Empty */
    } Irq;

    /**
     * @brief Enumeration for flow control types in serial communication.
     */
    typedef enum
    {
        FlowControlNone = 0,  /**< No flow control */
        FlowControlRTS,       /**< RTS flow control */
        FlowControlCTS,       /**< CTS flow control */
        FlowControlRTSCTS     /**< RTS/CTS flow control */
    } FlowControl;

#ifdef IOIG_HOST   
    class SerialImpl;

    /**
     * @brief Serial communication interface class.
     */
    class Serial : public Peripheral
    {
    friend class SerialImpl;
        
    public:
        using InterruptHandler = std::function<void(const char evtData)>; /**< Type for interrupt handler function pointer. */

        /**
         * @brief Default constructor.
         *        Builds object using default UART pinout.
         */
        Serial() : Serial(UART0_PINOUT0) {}; 

        /**
         * @brief Constructor to create a Serial port.
         * @param tx Transmit pin.
         * @param rx Receive pin.
         * @param baud The baud rate of the serial port (defaults 115200).
         * @param hw_instance Hardware instance.
         */
        Serial(int tx, int rx, int baud = 115200, unsigned hw_instance = 0);

        // Copy constructor
        Serial(const Serial &other) = default;

        // Assignment operator
        Serial &operator=(const Serial &other) = default;

        ~Serial();

        /**
         * @brief Set the baud rate of the serial port.
         * @param baudrate The baudrate of the serial port (default = 9600).
         */
        void baud(int baudrate);

        /**
         * @brief Generate a break condition on the serial line.
         *        NOTE: Clear break needs to run at least one frame after set_break is called.
         */
        void setBreak();

        /**
         * @brief Clear a break condition on the serial line.
         *        NOTE: Should be run at least one frame after set_break is called.
         */
        void clearBreak();

        /**
         * @brief Set the transmission format used by the serial port.
         * @param bits The number of bits in a word (5-8; default = 8).
         * @param parity The parity used (None, Odd, Even, Forced1, Forced0; default = None).
         * @param stop_bits The number of stop bits (1 or 2; default = 1).
         */
        void setFormat(int bits = 8, int parity = ParityNone, int stop_bits = 1);

        /**
         * @brief Set the flow control type on the serial port.
         * @param type The flow control type (Disabled, RTS, CTS, RTSCTS).
         * @param flow1_pin The first flow control pin (RTS for RTS or RTSCTS, CTS for CTS).
         * @param flow2_pin The second flow control pin (CTS for RTSCTS).
         */
        void setFlowControl(int type, int flow1_pin = NC, int flow2_pin = NC);

        /**
         * @brief Determine if there is a character available to read.
         * @returns 1 if there is a character available to read, 0 otherwise.
         */
        int readable();

        /**
         * @brief Determine if there is space available to write a character.
         * @returns 1 if there is space to write a character, 0 otherwise.
         */
        int writeable();

        /**
         * @brief Attach a function to call whenever a serial interrupt is generated.
         * @param func A pointer to a void function, or 0 to set as none.
         * @param type Which serial interrupt to attach the member function to (RxIrq for receive, TxIrq for transmit buffer empty).
         */
        void setInterrupt(const InterruptHandler &func, int type = RxIrq);

        /**
         * @brief Get character. This is a blocking call, waiting for a character.
         * @return The character read or negative error on failure.
         */
        int getc();

        /**
         * @brief Send a character. This is a blocking call, waiting for a peripheral to be available for writing.
         * @param c The character to be sent.
         * @return The character wrote or negative error on failure.
         */
        int putc(int c);

        /**
         * @brief Write the contents of a buffer to a file.
         * @param buffer The buffer to write from.
         * @param length The number of bytes to write.
         * @return The number of bytes written, negative error on failure.
         */
        int write(const uint8_t *buffer, size_t length);

        /**
         * @brief Read the contents of a file into a buffer.
         * @param buffer The buffer to read into.
         * @param length The number of bytes to read.
         * @return The number of bytes read, 0 at end of file, negative error on failure.
         */
        int read(uint8_t *buffer, size_t length);

    private:
        void initialize() override; /**< Initialize function from the base class. */

        int _tx;            /**< Transmit pin. */
        int _rx;            /**< Receive pin. */
        int _rts;           /**< RTS flow control pin. */
        int _cts;           /**< CTS flow control pin. */
        int _baud;          /**< Baud rate. */
        int _hwInstance;    /**< Hardware instance. */

        std::unique_ptr<SerialImpl> pimpl; /**< Pointer to implementation. */

        static constexpr char TAG[] = "Serial"; ///< Log tag

    };
#endif    

}
