#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "Serial.h"
#include "ioig_private.h"
#include "RingBuffer.h"

namespace arduino
{

    class IoIgSerialImpl
    {
    public:
        IoIgSerialImpl(UART &parent) : _parent(parent) {} 
        ~IoIgSerialImpl() {};

        UART &_parent;
		void on_rx(const char evtData);
		void block_tx(int);
		bool _block;
		const size_t WRITE_BUFF_SZ = 32;
		ioig::Serial* _serial = nullptr;
		int _tx, _rx, _rts, _cts;
		RingBufferN<ioig::Packet::MAX_SIZE> rx_buffer;
		uint8_t intermediate_buf[4];
        int _hw_instance=0;
    };

    void IoIgSerialImpl::on_rx(const char evtData) 
    {
    	while(_serial->readable()) 
        {
    		if (rx_buffer.availableForStore()) 
            {
    			rx_buffer.store_char(evtData);
    		}		
    	}
    }    

    UART::UART(int tx, int rx, int rts, int cts,unsigned hw_instance)
    {
        pimpl = std::make_unique<IoIgSerialImpl>(*this);
        pimpl->_tx = tx;
        pimpl->_rx = rx;
        pimpl->_rts = rts;
        pimpl->_cts = cts;
        pimpl->_hw_instance = hw_instance;        
    }
    

    UART::~UART()
    {                
    }    

    void UART::begin(unsigned long baudrate) 
    {

    	if (pimpl->_serial == nullptr) 
        {
            pimpl->_serial = new ioig::Serial(pimpl->_tx,pimpl->_rx,baudrate,pimpl->_hw_instance);
    	}
    	if (pimpl->_rts != NC) {
            pimpl->_serial->setFlowControl(ioig::FlowControl::FlowControlRTSCTS, pimpl->_rts, pimpl->_cts);		
    	} 
    	if (pimpl->_serial != nullptr) 
        {
            pimpl->_serial->setInterrupt([this](const char evtData) 
            {
                this->pimpl->on_rx(evtData);
            });            
    	}
    }

    void UART::begin(unsigned long baudrate, uint16_t config) 
    {
    	begin(baudrate);
    	int bits = 8;
        auto parity = ioig::Parity::ParityNone;
    	int stop_bits = 1;

    	switch (config & SERIAL_DATA_MASK) {
    		case SERIAL_DATA_7:
    			bits = 7;
    			break;
    		case SERIAL_DATA_8:
    			bits = 8;
    			break;
    /*
    		case SERIAL_DATA_9:
    			bits = 9;
    			break;
    */
    	}

    	switch (config & SERIAL_STOP_BIT_MASK) {
    		case SERIAL_STOP_BIT_1:
    			stop_bits = 1;
    			break;
    		case SERIAL_STOP_BIT_2:
    			stop_bits = 2;
    			break;
    	}

    	switch (config & SERIAL_PARITY_MASK) {
    		case SERIAL_PARITY_EVEN:
    			parity = ioig::Parity::ParityEven;
    			break;
    		case SERIAL_PARITY_ODD:
    			parity = ioig::Parity::ParityOdd;
    			break;
    		case SERIAL_PARITY_NONE:
    			parity = ioig::Parity::ParityNone;
    			break;
    	}

        pimpl->_serial->setFormat(bits,parity,stop_bits);
    }


    void UART::begin(unsigned long baudrate, uint16_t config, bool no_rx_pullup) 
    {
    	begin(baudrate, config);
    	if (no_rx_pullup) 
        {
            //TODO:
    		//SET_GPIO_PULL_FUNCTION(_rx, NO_PULL);
    	}
    }    



}//namespace