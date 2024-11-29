#include "Arduino.h"
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "Wire.h"
#include "ioig_private.h"



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
    ioig::Packet txBufffer;
    ioig::Packet rxBufffer;
    voidFuncPtrParamInt onReceiveCb = nullptr;
    voidFuncPtr onRequestCb = nullptr;
#ifdef DEVICE_I2CSLAVE
    //TODO: slave_th;
    void receiveThd();
#endif    
};

IoIgI2C::IoIgI2C(int sda, int scl)
{
    pimpl->master = new ioig::I2C(sda,scl);
}

IoIgI2C::~IoIgI2C()
{
    delete pimpl->master;
    pimpl->master = nullptr;
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
    pimpl->master->checkAndInitialize(); 
}

void IoIgI2C::begin(uint8_t address)
{
    pimpl->master->set_addr(address);
    pimpl->master->checkAndInitialize();

    if (address != 0) 
    {
       //LOG_ERR(TAG, "Peripheral mode (slave) not implemented!");
    }            
}

void IoIgI2C::end()
{
    pimpl->master->checkAndInitialize();
    
    ioig::Packet txPkt(4);
    ioig::Packet rxPkt(4);
 
    txPkt.setType(ioig::Packet::Type::I2C_DEINIT);

    ioig::UsbManager::transfer(txPkt, rxPkt, pimpl->master->getUsbPort()); 
}


void IoIgI2C::beginTransmission(uint8_t address)
{
    pimpl->master->checkAndInitialize();

    pimpl->master->set_addr(address);
    pimpl->txBufffer.reset();
    pimpl->txBufffer.reset();
}

uint8_t IoIgI2C::endTransmission(bool stopBit)
{
    pimpl->master->checkAndInitialize();
    auto addr = pimpl->master->get_addr();

    int ret = 4;
    if (pimpl->txBufffer.getPayloadLength()>0) 
    {
        ret = pimpl->master->write(addr, pimpl->txBufffer.getPayloadBuffer() , pimpl->txBufffer.getPayloadLength() , !stopBit);	
	}else 
    {	
        // we are scanning, return 0 if the addresed device responds with an ACK
		uint8_t buf[1];
		ret = pimpl->master->read(addr, buf, 1, !stopBit);		        
    }
    return ret;    

}

uint8_t IoIgI2C::endTransmission(void)
{
    pimpl->master->checkAndInitialize();

    return endTransmission(true);
}

size_t IoIgI2C::requestFrom(uint8_t address, size_t len, bool stopBit)
{
    pimpl->master->checkAndInitialize();

    pimpl->rxBufffer.reset();

    if (len > pimpl->rxBufffer.getFreePayloadSlots()) 
    {            
        //LOG_ERR(TAG, "Read buffer overflow, requested %d bytes, available %d bytes", (int)len, pimpl->rxBufffer.getFreePayloadSlots());    
        return 0;
    }

	int ret = pimpl->master->read(address, pimpl->rxBufffer.getPayloadBuffer(), len, !stopBit);
    if (ret != 0) 
    {
        return 0;
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
    pimpl->master->checkAndInitialize();

    if (pimpl->txBufffer.addPayloadItem8(data) < 0)
    {
        //LOG_ERR(TAG, "Wr Buffer overflow");
        return 0;
    }
    return 1;    
}




size_t IoIgI2C::write(const uint8_t* data, int len)
{
    pimpl->master->checkAndInitialize();
    
    if (len > (int)pimpl->txBufffer.getFreePayloadSlots()) 
    {          
        //LOG_ERR(TAG, "Write buffer overflow, requested %d bytes, available %d bytes", len, txBufffer.getFreePayloadSlots());
        len = pimpl->txBufffer.getFreePayloadSlots();
    }

    pimpl->txBufffer.addPayloadBuffer(data, len);

	return len;
}


void IoIgI2C::flush()
{
    pimpl->rxBufffer.flush();
}

int IoIgI2C::available()
{
    return pimpl->rxBufffer.getFreePayloadSlots();
}


void IoIgI2C::onReceive(voidFuncPtrParamInt cb) {
	pimpl->onReceiveCb = cb;
}
void IoIgI2C::onRequest(voidFuncPtr cb) {
	pimpl->onRequestCb = cb;
}

int IoIgI2C::read() 
{
	// if (available()) 
    // {        
    //     uint8_t value = _aucBuffer[_iTail];
    //     _iTail = nextIndex(_iTail);
    //     _numElems = _numElems - 1;
      
    //     return value;
	// }
	// return -1;    
}

int IoIgI2C::peek()
{
//   if (isEmpty())
//     return -1;

//   return _aucBuffer[_iTail];
}



}//namespace