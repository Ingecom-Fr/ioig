#include <iostream>

#include "ioig_private.h"
#include "serial.h"

using namespace std::chrono_literals;

namespace ioig
{
    class UARTImpl : public EventHandler
    {
    public:
        UARTImpl(UART &parent) : _parent(parent), _event(0), _eventCallback(nullptr) {}
        ~UARTImpl() { UsbManager::removeEventHandler(this, _parent._usbPort); };

        void onEvent(Packet &eventPkt) override
        {
            auto pktType = eventPkt.getType();

            if (pktType != Packet::Type::SERIAL_EVENT)
            {
                return;
            }

            auto hwInstance = eventPkt.getPayloadItem8(0);

            if (hwInstance != _parent._hwInstance)
            {
                return;
            }

            if (_eventCallback != nullptr)
            {
                _eventCallback((char)eventPkt.getPayloadItem8(1));
            }
        }

        UART &_parent;
        uint32_t _event;
        UART::InterruptHandler _eventCallback;
    };

    UART::UART(int tx, int rx, int baud, unsigned hw_instance) : _tx(tx),
                                                                     _rx(rx),
                                                                     _baud(baud),
                                                                     _hwInstance(hw_instance),
                                                                     pimpl(std::make_unique<UARTImpl>(*this))
    {

        if (_tx < 0 || _tx >= TARGET_PINS_COUNT)
        {
            LOG_ERR(TAG, "Invalid tx pin %d, max = %d", _tx, TARGET_PINS_COUNT - 1);
        }

        if (_rx < 0 || _rx >= TARGET_PINS_COUNT)
        {
            LOG_ERR(TAG, "Invalid rx pin %d, max = %d", _rx, TARGET_PINS_COUNT - 1);
        }

        if (_baud < 300 || _baud > 4'000'000)
        {
            LOG_ERR(TAG, "Invalid baud %d, using 115200", _baud);
            _baud = 115200;
        }

        if (_hwInstance >= UART_INSTANCES)
        {
            LOG_ERR(TAG, "Invalid UART hardware instance %d, max = %d, using 0...", _hwInstance, UART_INSTANCES - 1);
            _hwInstance = UART_0;
        }
    }

    UART::UART(UART &&other) noexcept
        : Peripheral(std::move(other)), // Move base class
          _tx(other._tx),
          _rx(other._rx),
          _rts(other._rts),
          _cts(other._cts),
          _baud(other._baud),
          _hwInstance(other._hwInstance),
          pimpl(std::move(other.pimpl))
    {
    }

    UART &UART::operator=(UART &&other) noexcept
    {
        if (this != &other)
        {
            Peripheral::operator=(std::move(other)); // Move base class
            _tx = other._tx;
            _rx = other._rx;
            _rts = other._rts;
            _cts = other._cts;
            _baud = other._baud;
            _hwInstance = other._hwInstance;
            pimpl = std::move(other.pimpl); // Transfer ownership of pimpl
        }
        return *this;
    }

    UART::~UART()
    {
        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_DEINIT);
    }

    void UART::initialize()
    {
        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_INIT);
        auto txp0 = txPkt.addPayloadItem8(_hwInstance);
        auto txp1 = txPkt.addPayloadItem8(_tx);
        auto txp2 = txPkt.addPayloadItem8(_rx);
        auto txp3 = txPkt.addPayloadItem32(115200);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        auto rxp0 = rxPkt.getPayloadItem8(0);
        auto rxp1 = rxPkt.getPayloadItem8(1);
        auto rxp2 = rxPkt.getPayloadItem8(2);
        auto rxp3 = rxPkt.getPayloadItem32(3);

        if (txp0 != rxp0)
        {
            LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
        }

        if (txp1 != rxp1)
        {
            LOG_ERR(TAG, "Invalid response from device ( tx pin ) : expected = %d, received = %d", txp1, rxp1);
        }

        if (txp2 != rxp2)
        {
            LOG_ERR(TAG, "Invalid response from device ( rx pin ) : expected = %d, received = %d", txp2, rxp2);
        }

        if (txp3 != rxp3)
        {
            LOG_ERR(TAG, "Invalid response from device ( baud ) : expected = %d, received = %d", txp3, rxp3);
        }
    }

    void UART::baud(int baudrate)
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_SET_BAUD);
        auto txp0 = txPkt.addPayloadItem8(_hwInstance);
        auto txp1 = txPkt.addPayloadItem32(baudrate);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        auto rxp0 = rxPkt.getPayloadItem8(0);
        auto rxp1 = rxPkt.getPayloadItem32(1);

        if (txp0 != rxp0)
        {
            LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
        }

        if (txp1 != rxp1)
        {
            LOG_ERR(TAG, "Invalid response from device ( baud ) : expected = %d, received = %d", txp1, rxp1);
        }
    }

    void UART::setBreak()
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_SET_BREAK);
        txPkt.addPayloadItem8(_hwInstance);
        txPkt.addPayloadItem8(true);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);
    }

    void UART::clearBreak()
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_SET_BREAK);
        txPkt.addPayloadItem8(_hwInstance);
        txPkt.addPayloadItem8(false);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);
    }

    int UART::write(const uint8_t *buffer, size_t length)
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_WRITE);
        txPkt.addPayloadItem8(_hwInstance);
        txPkt.addPayloadItem8(length);

        if (txPkt.addPayloadBuffer(buffer, length) < 0)
        {
            LOG_ERR(TAG, "Wr buffer overflow, requested %d bytes, available %d bytes", (int)length, (int)txPkt.getFreePayloadSlots());
            return -1;
        }

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        if (rxPkt.getStatus() == Packet::Status::RSP)
        {
            return length;
        }
        return -1;
    }

    int UART::read(uint8_t *buffer, size_t length)
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_READ);
        txPkt.addPayloadItem8(_hwInstance);
        txPkt.addPayloadItem8(length);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        if (rxPkt.getStatus() == Packet::Status::RSP)
        {
            memcpy(buffer, rxPkt.getPayloadBuffer(), rxPkt.getPayloadLength());
            return rxPkt.getPayloadLength();
        }
        return -1;
    }

    void UART::setFormat(int bits, int parity, int stop_bits)
    {
        checkAndInitialize();

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_SET_FORMAT);
        auto txp0 = txPkt.addPayloadItem8(_hwInstance);
        auto txp1 = txPkt.addPayloadItem8(parity);
        auto txp2 = txPkt.addPayloadItem8(bits);
        auto txp3 = txPkt.addPayloadItem8(stop_bits);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        auto rxp0 = rxPkt.getPayloadItem8(0);
        auto rxp1 = rxPkt.getPayloadItem8(1);
        auto rxp2 = rxPkt.getPayloadItem8(2);
        auto rxp3 = rxPkt.getPayloadItem8(3);

        if (txp0 != rxp0)
        {
            LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
        }

        if (txp1 != rxp1)
        {
            LOG_ERR(TAG, "Invalid response from device ( parity ) : expected = %d, received = %d", txp1, rxp1);
        }

        if (txp2 != rxp2)
        {
            LOG_ERR(TAG, "Invalid response from device ( data bits ) : expected = %d, received = %d", txp2, rxp2);
        }

        if (txp3 != rxp3)
        {
            LOG_ERR(TAG, "Invalid response from device ( stop bits ) : expected = %d, received = %d", txp3, rxp3);
        }
    }

    void UART::setFlowControl(int type, int flow1_pin, int flow2_pin)
    {
        checkAndInitialize();

        if (type < FlowControlNone || type > FlowControlRTSCTS)
        {
            LOG_ERR(TAG, "Invalid flow control type");
            return;
        }

        if (flow1_pin < 0 || flow1_pin >= TARGET_PINS_COUNT)
        {
            LOG_ERR(TAG, "Invalid flow1 pin pin %d, max = %d", flow1_pin, TARGET_PINS_COUNT - 1);
            return;
        }

        if (flow2_pin < 0 || flow2_pin >= TARGET_PINS_COUNT)
        {
            LOG_ERR(TAG, "Invalid flow2  pin %d, max = %d", flow2_pin, TARGET_PINS_COUNT - 1);
            return;
        }

        Packet txPkt;
        Packet rxPkt;

        txPkt.setType(Packet::Type::SERIAL_SET_FLOW_CONTROL);
        auto txp0 = txPkt.addPayloadItem8(_hwInstance);
        auto txp1 = txPkt.addPayloadItem8(flow1_pin);
        auto txp2 = txPkt.addPayloadItem8(flow2_pin);
        auto txp3 = txPkt.addPayloadItem8(type);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        auto rxp0 = rxPkt.getPayloadItem8(0);
        auto rxp1 = rxPkt.getPayloadItem8(1);
        auto rxp2 = rxPkt.getPayloadItem8(2);
        auto rxp3 = rxPkt.getPayloadItem8(3);

        if (txp0 != rxp0)
        {
            LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
        }

        if (txp1 != rxp1)
        {
            LOG_ERR(TAG, "Invalid response from device ( flow1 pin ) : expected = %d, received = %d", txp1, rxp1);
        }

        if (txp2 != rxp2)
        {
            LOG_ERR(TAG, "Invalid response from device ( flow2 pin ) : expected = %d, received = %d", txp2, rxp2);
        }

        if (txp3 != rxp3)
        {
            LOG_ERR(TAG, "Invalid response from device ( flow control type ) : expected = %d, received = %d", txp3, rxp3);
        }
    }

    int UART::readable()
    {
        checkAndInitialize();

        Packet txPkt(2);
        Packet rxPkt(2);

        txPkt.setType(Packet::Type::SERIAL_READABLE);
        txPkt.addPayloadItem8(_hwInstance);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        return rxPkt.getPayloadItem8(0);
    }

    int UART::writeable()
    {
        checkAndInitialize();

        Packet txPkt(2);
        Packet rxPkt(2);

        txPkt.setType(Packet::Type::SERIAL_WRITABLE);
        txPkt.addPayloadItem8(_hwInstance);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        return rxPkt.getPayloadItem8(0);
    }

    void UART::setInterrupt(const InterruptHandler &func, int type)
    {
        checkAndInitialize();

        Packet txPkt(8);
        Packet rxPkt(8);

        txPkt.setType(Packet::Type::SERIAL_SET_IRQ);
        auto txp0 = txPkt.addPayloadItem8(_hwInstance);
        auto txp1 = txPkt.addPayloadItem8(true); // enable
        auto txp2 = txPkt.addPayloadItem8(type);

        pimpl->_eventCallback = func;
        pimpl->_event = type;

        UsbManager::registerEventHandler(pimpl.get(), _usbPort);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        auto rxp0 = rxPkt.getPayloadItem8(0);
        auto rxp1 = rxPkt.getPayloadItem8(1);
        auto rxp2 = rxPkt.getPayloadItem8(2);

        if (txp0 != rxp0)
        {
            LOG_ERR(TAG, "Invalid response from device ( hw instance ) : expected = %d, received = %d", txp0, rxp0);
        }

        if (txp1 != rxp1)
        {
            LOG_ERR(TAG, "Invalid response from device ( interrupt enable flag ) : expected = %d, received = %d", txp1, rxp1);
        }

        if (txp2 != rxp2)
        {
            LOG_ERR(TAG, "Invalid response from device ( interrupt type ) : expected = %d, received = %d", txp2, rxp2);
        }
    }

    int UART::getc()
    {
        checkAndInitialize();

        Packet txPkt(4);
        Packet rxPkt(4);

        txPkt.setType(Packet::Type::SERIAL_GETC);
        txPkt.addPayloadItem8(_hwInstance);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        if (rxPkt.getStatus() == Packet::Status::RSP)
        {
            return rxPkt.getPayloadItem8(0);
        }

        return -1;
    }

    int UART::putc(int c)
    {
        checkAndInitialize();

        Packet txPkt(4);
        Packet rxPkt(4);

        txPkt.setType(Packet::Type::SERIAL_PUTC);
        txPkt.addPayloadItem8(_hwInstance);
        txPkt.addPayloadItem8(c);

        UsbManager::transfer(txPkt, rxPkt, _usbPort);

        if (rxPkt.getStatus() == Packet::Status::RSP)
        {
            return rxPkt.getPayloadItem8(0);
        }

        return -1;
    }

} // namespace