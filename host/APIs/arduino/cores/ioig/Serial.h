#pragma once

#include "Arduino.h"
#include "HardwareSerial.h"

namespace arduino {

class IoIgSerialImpl;

class UART : public HardwareSerial {

	public:
		UART(int tx, int rx, int rts = -1, int cts = -1,unsigned hw_instance = 0);
		UART();
		~UART();
			
		void begin(unsigned long);
		void begin(unsigned long baudrate, uint16_t config);
		void begin(unsigned long baudrate, uint16_t config, bool no_rx_pullup);
		void end();
		int available(void);
		int peek(void);
		int read(void);
		void flush(void);
		size_t write(uint8_t c);
		size_t write(const uint8_t*, size_t);
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool();

#if defined(SERIAL_CDC)
		uint32_t baud();
		uint8_t stopbits();
		uint8_t paritytype();
		uint8_t numbits();
		bool dtr();
		bool rts();
#endif	
	private:
        std::unique_ptr<IoIgSerialImpl> pimpl; ///< Pointer to implementation.
};

}

extern arduino::UART _UART1_;
extern arduino::UART _UART2_;
extern arduino::UART _UART3_;
extern arduino::UART _UART4_;
extern arduino::UART _UART_USB_;

