#pragma once

#include "ioig.h"

#ifdef IOIG_HOST  
#include <vector>
#include <functional>
#include <cstdint>
#include <memory>
#endif

namespace ioig
{
    /**
     * @brief GPIO pin directions
     */
    typedef enum
    {
        Input = 0,  ///< Input direction
        Output      ///< Output direction
    } PinDirection;

    /**
     * @brief GPIO pin mode
     */
    typedef enum
    {
        PullNone = 0,               ///< No pull mode
        PullUp = 1,                 ///< Pull-up mode
        PullDown = 2,               ///< Pull-down mode
        OpenDrainPullUp = 3,        ///< Open-drain with pull-up mode
        OpenDrainNoPull = 4,        ///< Open-drain without pull mode
        OpenDrainPullDown = 5,      ///< Open-drain with pull-down mode
        PushPullNoPull = PullNone,  ///< Push-pull without pull mode
        PushPullPullUp = PullUp,    ///< Push-pull with pull-up mode
        PushPullPullDown = PullDown,///< Push-pull with pull-down mode
        OpenDrain = OpenDrainPullUp,///< Alias for OpenDrainPullUp
        PullDefault = PullNone      ///< Default pull mode
    } PinMode;

    /**
     * @brief Enumeration representing the strength of a GPIO pin.
     */
    typedef enum
    {
        Strength_2ma = 0,   ///< 2 mA nominal drive strength
        Strength_4ma = 1,   ///< 4 mA nominal drive strength
        Strength_8ma = 2,   ///< 8 mA nominal drive strength
        Strength_12ma = 3   ///< 12 mA nominal drive strength
    } PinStrength;

    /**
     * @brief Enumeration representing interrupt events on a GPIO pin.
     */
    typedef enum
    {
        LevelLow = 0x1u,    ///< Level low event
        LevelHigh = 0x2u,   ///< Level high event
        FallEdge = 0x4u,    ///< Falling edge event
        RiseEdge = 0x8u     ///< Rising edge event
    } PinEvent;    

#ifdef IOIG_HOST    

    // Forward declaration of the implementation class
    class GpioImpl;

    /**
     * @brief Gpio Class for General Purpose Input/Output Operations.
     *
     * This class provides an interface for controlling GPIO pins, including
     * setting direction, mode, strength, reading and writing pin values,
     * and handling interrupts.
     */
    class Gpio : public Peripheral
    {
    friend class GpioImpl;

    public:
        /**
         * @brief Type definition for interrupt handler function.
         */
        using InterruptHandler = std::function<void(const int pin, const uint32_t events, void * arg)>;

        /**
         * @brief Default constructor.
         *
         * This constructor is deleted to prevent instantiation without a pin number.
         */
        Gpio() = delete;

        /**
         * @brief Constructor to create a Gpio object connected to a specified pin.
         *
         * @param pin The GPIO pin to connect to.
         * @param direction The initial direction of the pin (Input or Output).
         * @param mode The initial pull mode of the pin (PullUp, PullDown, PullNone, OpenDrain).
         */
        Gpio(int pin, int direction = PinDirection::Output, int mode = PinMode::PullDefault);

        
        // Disable Copy Constructor and Copy Assignment
        Gpio(const Gpio&) = delete;
        Gpio& operator=(const Gpio&) = delete;
    
        // Enable Move Constructor and Move Assignment
        Gpio(Gpio&& other) noexcept;
        Gpio& operator=(Gpio&& other) noexcept;


        /**
         * @brief Destructor.
         */
        ~Gpio();

        /**
         * @brief Get the pin associated with the Gpio.
         *
         * @return The pin associated with the Gpio.
         */
        int getPin() { return _pin; }

        /**
         * @brief Write a value to the GPIO pin.
         *
         * @param value The value to write (0 for logical low, 1 for logical high).
         */
        void write(int value);

        /**
         * @brief Read the value from the GPIO pin.
         *
         * @return The value read from the pin (0 for logical low, 1 for logical high).
         */
        int read();

        /**
         * @brief Set the Gpio as an output.
         */
        void output();

        /**
         * @brief Set the Gpio as an input.
         */
        void input();

        /**
         * @brief Set the input pin mode.
         *
         * @param pull The pull mode to set (PullUp, PullDown, PullNone, OpenDrain).
         */
        void mode(int pull);

        /**
         * @brief Set the pin drive strength.
         *
         * @param strength The drive strength to set (Strength_2ma, Strength_4ma, Strength_8ma, Strength_12ma).
         */
        void setStrength(int strength);

        /**
         * @brief Measure the duration of a pulse on the GPIO pin.
         *
         * @param state The state of the pulse to measure (0 or 1).
         * @param timeout The maximum duration to wait for the pulse (in microseconds).
         * @return The duration of the pulse (in microseconds), or 0 if timeout occurs.
         */
        unsigned long pulseIn(uint8_t state, unsigned long timeout = 1000000);

        /**
         * @brief Shorthand for write() function.
         *
         * @param value The value to write (0 for logical low, 1 for logical high).
         * @return Reference to the modified Gpio object.
         */
        Gpio &operator=(int value)
        {
            write(value);
            return *this;
        }

        /**
         * @brief Enable interrupts for the GPIO pin.
         *
         * @param events The event mask specifying which events to listen for.
         * @param cbk The callback function invoked on interrupt events.
         */
        void setInterrupt(const uint32_t events, const InterruptHandler &cbk, void * arg=nullptr);

        /**
         * @brief Disable interrupts for the GPIO pin.
         */
        void disableInterrupt();

    private:
        /**
         * @brief Initialize the GPIO pin.
         */
        void initialize() override;

        int _pin;           ///< The GPIO pin number.
        int _dir;           ///< The direction of the GPIO pin.
        int _mode;          ///< The mode of the GPIO pin.

        static constexpr const char* TAG = "GPIO";  ///< Tag used for logging.

        std::unique_ptr<GpioImpl> pimpl;   ///< Pointer to implementation.
    };
#endif

} // namespace ioig
