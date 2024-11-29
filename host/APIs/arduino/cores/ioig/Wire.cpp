#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "Wire.h"
#include "ioig_private.h"
#include "RingBuffer.h"



#if I2C_INSTANCES > 0
arduino::IoIgI2C Wire(I2C0_PINOUT0);
#endif
#if I2C_INSTANCES > 1
arduino::IoIgI2C Wire1(I2C1_PINOUT0);
#endif
#if I2C_INSTANCES > 2
arduino::IoIgI2C Wire2(I2C1_PINOUT1);
#endif


namespace arduino
{

class IoIgI2CImpl 
{
public:
    IoIgI2CImpl(IoIgI2C& parent): _parent(parent) {}
    ~IoIgI2CImpl() {   };
        
    IoIgI2C & _parent;

#ifdef DEVICE_I2CSLAVE
    //TODO:
#endif
    ioig::I2C*   master = nullptr;
    uint8_t txBuffer[ioig::Packet::MAX_SIZE];
    uint32_t usedTxBuffer{0};
    RingBufferN<ioig::Packet::MAX_SIZE> rxBuffer;
    voidFuncPtrParamInt onReceiveCb = nullptr;
    voidFuncPtr onRequestCb = nullptr;
#ifdef DEVICE_I2CSLAVE
    //TODO: slave_th;
    void receiveThd();
#endif    
};

IoIgI2C::IoIgI2C(int sda, int scl)
{
    pimpl = std::make_unique<IoIgI2CImpl>(*this);
    pimpl->master = new ioig::I2C(sda,scl);
}

IoIgI2C::~IoIgI2C()
{
    end();
}


void  IoIgI2C::setClock(uint32_t freq) 
{
	if (pimpl->master != nullptr) {
		pimpl->master->setFrequency(freq);
	}
#ifdef DEVICE_I2CSLAVE
	if (slave != nullptr) {
		slave->frequency(freq);
	}
#endif
}



void IoIgI2C::begin()
{
    //end(); //TODO: 
    pimpl->master->checkAndInitialize(); 
}

void IoIgI2C::begin(uint8_t address)
{
    (void)address;
    //TODO: slave
}

void IoIgI2C::end()
{

	if (pimpl->master != nullptr) 
    {
        pimpl->master->checkAndInitialize();
        
        ioig::Packet txPkt(4);
        ioig::Packet rxPkt(4);
     
        txPkt.setType(ioig::Packet::Type::I2C_DEINIT);
    
        ioig::UsbManager::transfer(txPkt, rxPkt, pimpl->master->getUsbPort()); 

		delete pimpl->master;
		pimpl->master = nullptr;
	}

}


void IoIgI2C::beginTransmission(uint8_t address)
{
    pimpl->master->checkAndInitialize();

    pimpl->master->set_addr(address);
    pimpl->usedTxBuffer = 0;
    pimpl->rxBuffer.clear();
}

uint8_t IoIgI2C::endTransmission(bool stopBit)
{
    // pimpl->master->checkAndInitialize();
    // auto addr = pimpl->master->get_addr();

    // int ret = 4;
    // if (pimpl->txBuffer.getPayloadLength()>0) 
    // {
    //     ret = pimpl->master->write(addr, pimpl->txBuffer.getPayloadBuffer() , pimpl->txBuffer.getPayloadLength() , !stopBit);	
	// }else 
    // {	
    //     // we are scanning, return 0 if the addresed device responds with an ACK
	// 	uint8_t buf[1];
	// 	ret = pimpl->master->read(addr, buf, 1, !stopBit);		        
    // }
    // return ret;    

    pimpl->master->checkAndInitialize();
	if ( pimpl->master->write( pimpl->master->get_addr() , pimpl->txBuffer,  pimpl->usedTxBuffer, !stopBit) == 0) return 0;
	return 2;    

}

uint8_t IoIgI2C::endTransmission(void)
{
    pimpl->master->checkAndInitialize();

    return endTransmission(true);
}

size_t IoIgI2C::requestFrom(uint8_t address, size_t len, bool stopBit)
{
    // pimpl->master->checkAndInitialize();

    // pimpl->rxBuffer.reset();

    // if (len > pimpl->rxBuffer.getFreePayloadSlots()) 
    // {            
    //     //LOG_ERR(TAG, "Read buffer overflow, requested %d bytes, available %d bytes", (int)len, pimpl->rxBuffer.getFreePayloadSlots());    
    //     return 0;
    // }

	// int ret = pimpl->master->read(address, pimpl->rxBuffer.getPayloadBuffer(), len, !stopBit);
    // if (ret != 0) 
    // {
    //     return 0;
    // }

	// return len;
    
    pimpl->master->checkAndInitialize();

	char buf[ioig::Packet::MAX_SIZE];
	len = std::min(len, sizeof(buf));
	int ret =  pimpl->master->read(address << 1, (uint8_t *)buf, len, !stopBit);
	if (ret != 0) {
		return 0;
	}
	for (size_t i=0; i<len; i++) {
		pimpl->rxBuffer.store_char(buf[i]);
	}
	return len;    
}

size_t IoIgI2C::requestFrom(uint8_t address, size_t len)
{
    pimpl->master->checkAndInitialize();

    return requestFrom(address, len, true);
}


size_t IoIgI2C::write(uint8_t data)
{
    // pimpl->master->checkAndInitialize();

    // if (pimpl->txBuffer.addPayloadItem8(data) < 0)
    // {
    //     //LOG_ERR(TAG, "Wr Buffer overflow");
    //     return 0;
    // }
    // return 1;    

    pimpl->master->checkAndInitialize();

	if (pimpl->usedTxBuffer == ioig::Packet::MAX_SIZE) return 0;
	pimpl->txBuffer[pimpl->usedTxBuffer++] = data;
    
	return 1;    
}




size_t IoIgI2C::write(const uint8_t* data, int len)
{
    // pimpl->master->checkAndInitialize();
    
    // if (len > (int)pimpl->txBuffer.getFreePayloadSlots()) 
    // {          
    //     //LOG_ERR(TAG, "Write buffer overflow, requested %d bytes, available %d bytes", len, txBuffer.getFreePayloadSlots());
    //     len = pimpl->txBuffer.getFreePayloadSlots();
    // }

    // pimpl->txBuffer.addPayloadBuffer(data, len);

	// return len;

    pimpl->master->checkAndInitialize();
	if (pimpl->usedTxBuffer + len > ioig::Packet::MAX_SIZE) len = ioig::Packet::MAX_SIZE - pimpl->usedTxBuffer;
	memcpy(pimpl->txBuffer + pimpl->usedTxBuffer, data, len);
	pimpl->usedTxBuffer += len;
	return len;    
}


void IoIgI2C::flush()
{
    //pimpl->rxBuffer.flush();
}

int IoIgI2C::available()
{
    //return pimpl->rxBuffer.getFreePayloadSlots();
    return pimpl->rxBuffer.available();
}


void IoIgI2C::onReceive(voidFuncPtrParamInt cb) {
	pimpl->onReceiveCb = cb;
}
void IoIgI2C::onRequest(voidFuncPtr cb) {
	pimpl->onRequestCb = cb;
}

int IoIgI2C::read() 
{
	if (pimpl->rxBuffer.available()) {
		return pimpl->rxBuffer.read_char();
	}
	return -1;  
}

int IoIgI2C::peek()
{
    return pimpl->rxBuffer.peek();
}



}//namespace