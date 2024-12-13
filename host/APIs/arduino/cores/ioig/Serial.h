#pragma once

#include "HardwareSerial.h"
#include <memory>

namespace arduino
{

	class IoIgSerialImpl;

	class UART : public HardwareSerial
	{

	friend class IoIgSerialImpl;

	public:
		UART(int tx, int rx, int rts = -1, int cts = -1, unsigned hw_instance = 0);
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
		size_t write(const uint8_t *, size_t);
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool();

	private:
		std::unique_ptr<IoIgSerialImpl> pimpl; ///< Pointer to implementation.
	};

}

extern arduino::UART _UART1_;
extern arduino::UART _UART2_;
