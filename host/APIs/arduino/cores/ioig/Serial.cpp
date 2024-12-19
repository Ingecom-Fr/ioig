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

#ifdef Serial
#undef Serial
#endif

arduino::UART _UART1_(GP0, GP1, NC, NC, UART_0);
arduino::UART _UART2_(GP4, GP5, NC, NC, UART_1);

using namespace arduino;

class arduino::IoIgSerialImpl
{
public:
	IoIgSerialImpl(UART &parent) : _parent(parent) {}
	~IoIgSerialImpl() {};

	UART &_parent;
	void on_rx(const char *data, size_t len);
	const size_t WRITE_BUFF_SZ = 32;
	ioig::UART *_serial = nullptr;
	int _tx, _rx, _rts, _cts;
	std::vector<char> rxDataVec;
	int _hw_instance = 0;
};

void IoIgSerialImpl::on_rx(const char *data, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		rxDataVec.push_back(data[i]);
	}
}

UART::UART()
{
}

UART::UART(int tx, int rx, int rts, int cts, unsigned hw_instance)
{
	pimpl = std::make_unique<IoIgSerialImpl>(*this);
	pimpl->_tx = tx;
	pimpl->_rx = rx;
	pimpl->_rts = rts;
	pimpl->_cts = cts;
	pimpl->_hw_instance = hw_instance;
	pimpl->rxDataVec.clear();
}

UART::~UART()
{
	end();
}

void UART::begin(unsigned long baudrate)
{
	if (pimpl->_serial == nullptr)
	{
		pimpl->_serial = new ioig::UART(pimpl->_tx, pimpl->_rx, baudrate, pimpl->_hw_instance);
		pimpl->_serial->checkAndInitialize();
	}
	if (pimpl->_rts != NC)
	{
		pimpl->_serial->setFlowControl(ioig::FlowControl::FlowControlRTSCTS, pimpl->_rts, pimpl->_cts);
	}
	if (pimpl->_serial != nullptr)
	{
		pimpl->rxDataVec.clear();
		pimpl->_serial->setInterrupt([&](const char *data, size_t len)
									 { 
										pimpl->on_rx(data, len); 
									 });
	}
}

void UART::begin(unsigned long baudrate, uint16_t config)
{
	begin(baudrate);
	int bits = 8;
	auto parity = ioig::Parity::ParityNone;
	int stop_bits = 1;

	switch (config & SERIAL_DATA_MASK)
	{
	case SERIAL_DATA_7:
		bits = 7;
		break;
	case SERIAL_DATA_8:
		bits = 8;
		break;
	}

	switch (config & SERIAL_STOP_BIT_MASK)
	{
	case SERIAL_STOP_BIT_1:
		stop_bits = 1;
		break;
	case SERIAL_STOP_BIT_2:
		stop_bits = 2;
		break;
	}

	switch (config & SERIAL_PARITY_MASK)
	{
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

	pimpl->_serial->setFormat(bits, parity, stop_bits);
}

void UART::begin(unsigned long baudrate, uint16_t config, bool no_rx_pullup)
{
	begin(baudrate, config);
	if (no_rx_pullup)
	{
		// TODO:
		// SET_GPIO_PULL_FUNCTION(_rx, NO_PULL);
	}
}

void UART::end()
{
	if (pimpl->_serial != nullptr)
	{
		pimpl->_serial->checkAndInitialize();

		ioig::Packet txPkt(4);
		ioig::Packet rxPkt(4);

		txPkt.setType(ioig::Packet::Type::SERIAL_DEINIT);

		ioig::UsbManager::transfer(txPkt, rxPkt, pimpl->_serial->getUsbPort());

		delete pimpl->_serial;
		pimpl->_serial = nullptr;
	}
	pimpl->rxDataVec.clear();
}

int UART::available()
{
	auto cur_sz = pimpl->rxDataVec.size();
	int av = cur_sz < 256 ? 256 - cur_sz : 0;
	return av;
}

int UART::peek()
{
	if (pimpl->rxDataVec.empty())
		return -1;

	return pimpl->rxDataVec.back();
}

int UART::read()
{
	if (pimpl->rxDataVec.empty())
		return -1;

	uint8_t value = pimpl->rxDataVec.back();
	pimpl->rxDataVec.pop_back();
	return value;
}

void UART::flush()
{
	while (!pimpl->_serial->writeable())
		;
}

size_t UART::write(uint8_t c)
{
	printf("%c", c);
	int ret = pimpl->_serial->putc(c);
	return ret == -1 ? 0 : 1;
}

size_t UART::write(const uint8_t *buf, size_t len)
{

	printf("%.*s", (int)len, buf);
	while (!pimpl->_serial->writeable())
		;

	int ret = pimpl->_serial->write(buf, len);

	return ret == -1 ? 0 : len;
}

UART::operator bool()
{
	return pimpl->_serial != nullptr;
}
