#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <memory.h>

#include "ioig.h"
#include "ioig_usb.h"

#define PKT_DEBUG 0
#define LIBUSB_LOG 0

#if PKT_DEBUG
#define PRINT_PKT(label, pkt, mtx)             \
    do                                         \
    {                                          \
        std::lock_guard<std::mutex> lock(mtx); \
        printf("%s", label);                   \
        pkt.print();                           \
    } while (0)
#else
#define PRINT_PKT(label, pkt, mtx) \
    do                             \
    {                              \
    } while (0)
#endif

using namespace ioig;
using namespace std::chrono_literals;



void UsbManager::deinit()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_running.load()) 
    {
        return;        
    }

    _running.store(false);
    
    for (unsigned i=0; i < MAX_USB_DEVICES ; i++) 
    {
        closeUsbDevice(i);
    }
}


bool UsbManager::openUsbDevice(int usb_port)
{
    bool deviceFound=false;

    libusb_device** deviceList;
    int targetDevCnt=0;

    auto & context = _usbContextVec[usb_port];
 
 
    ssize_t deviceCount = libusb_get_device_list(context, &deviceList);

    if (deviceCount < 0) 
    {
        LOG_ERR(TAG, "Can't get device list!");
        libusb_free_device_list(deviceList, true);   
        return false;          
    }
    
    // Iterate through the list and find the specified device    
    for (ssize_t i = 0; i < deviceCount; ++i) 
    {
        libusb_device* device = deviceList[i];
        libusb_device_descriptor descriptor;

        if (libusb_get_device_descriptor(device, &descriptor) != LIBUSB_SUCCESS && usb_port == i) 
        {
            LOG_ERR(TAG, "Can't get descriptor for device : %d" , usb_port);   
            libusb_free_device_list(deviceList, true);  
            return false;                   
        }

        if (descriptor.idVendor == IOIG_VID && descriptor.idProduct == IOIG_PID) 
        {
            if (usb_port == targetDevCnt)
            {
                libusb_device_handle* dhandle = libusb_open_device_with_vid_pid(context, IOIG_VID, IOIG_PID);     
                if (dhandle == NULL) 
                {
                    LOG_ERR(TAG, "Can't open device : %d" , usb_port); 
                    return false;
                }
                _usbDevHandlerVec[usb_port] = dhandle;
                deviceFound=true;
            }                                
            targetDevCnt += 1;            
        }
    }     

    // Free the list of devices
    libusb_free_device_list(deviceList, 1);

    if (!deviceFound) 
    { 
        LOG_ERR(TAG, "Can't find device index %d", usb_port); 
    }

    return deviceFound;    
}


void UsbManager::initUsbDevice(int usb_port)
{    
    if (!openUsbDevice(usb_port))   //creates a device handle in _usbDevHandlerVec[usb_port]
    {
        std::exit(-1);
    }


    auto & usbDevHandler =  _usbDevHandlerVec[usb_port];    

#ifdef __linux__
    int ret = libusb_set_auto_detach_kernel_driver(usbDevHandler, true);
    if (ret != LIBUSB_SUCCESS)
    {        
        LOG_ERR(TAG, "Failed to detach kernel driver, error : %s" , LIBUSB_ERR(ret));
        std::exit(ret);        
    }
#endif    
    
    auto claimItf = [&](const char * iname, const unsigned inum) 
    {        
        int ret = libusb_claim_interface(usbDevHandler, inum);
        if (ret != LIBUSB_SUCCESS)
        {            
            LOG_ERR(TAG, "Can't claim interface %d (%s), error : %s", inum, iname , LIBUSB_ERR(ret));                      
            std::exit(ret);
        }
    };
  
    claimItf("Data  Notif",0);  
    claimItf("Data",1);  
    claimItf("Event Notif",2);  
    claimItf("Event",3);  
}

void UsbManager::closeUsbDevice(int usb_port)
{
    if (_usbDevHandlerVec[usb_port] != nullptr) 
    {    
        auto & devHanlder = _usbDevHandlerVec[usb_port];
        libusb_release_interface(devHanlder, 0);
        libusb_release_interface(devHanlder, 1);
        libusb_release_interface(devHanlder, 2);
        libusb_release_interface(devHanlder, 3);
        libusb_close(devHanlder);      
    }
}

void UsbManager::sendResetCmd(int usb_port)
{
    //Request firmware reset
    Packet txPkt(16);
    Packet rxPkt(16);

    txPkt.setType(Packet::Type::SYS_SW_RESET);    
    txPkt.setStatus(Packet::Status::CMD); 

    sendPacket(txPkt, CDC_DATA_EP_OUT, usb_port, 1000);
    recvPacket(rxPkt, CDC_DATA_EP_IN, usb_port, 1000);
    std::this_thread::sleep_for(50ms); //wait fw reset   
}


void UsbManager::eventThread(int usb_port)
{
    Packet evtPkt;

    auto & myEventHandlerVec  = _eventHandlerVec[usb_port];
    
    while (_running.load()) 
    {           
        //Wait indefinitely for an async event         
        recvPacket(evtPkt, CDC_EVENT_EP_IN , usb_port ,0); 

        for ( size_t i = 0 ; i < myEventHandlerVec.size() ; ++i ) 
        {
            auto evtHandler = myEventHandlerVec[i];         
            evtHandler->onEvent(evtPkt);
        }
         
    }
}


void UsbManager::registerEventHandler(EventHandler * evHandler, int usb_port)
{
    checkAndInitialize(usb_port);

    std::unique_lock<std::mutex> lock(_mutex);

    if (usb_port >= (int)MAX_USB_DEVICES) 
    {
        LOG_ERR(TAG, "Can't register event handler for USB device %d, max USB device index = %d", usb_port, MAX_USB_DEVICES-1);
        return;
    }
    auto & eventHandlerVec  = _eventHandlerVec[usb_port]; //get the vector for this port
    bool evHandlerFound = false;

    for (auto it = eventHandlerVec.begin(); it != eventHandlerVec.end(); ++it) 
    {
        if (*it == evHandler) 
        {
            evHandlerFound = true;            
        }
    }
    
    
    if (eventHandlerVec.empty())
    {
        eventHandlerVec.push_back(evHandler);        
        std::thread th = std::thread(&UsbManager::eventThread, usb_port);
        th.detach();  
    }else 
    {
        if (!evHandlerFound) 
        {
            eventHandlerVec.push_back(evHandler);       
        }else 
        {
            LOG_WARN(TAG,"Event handler already registered");
        }
    }  
}

void UsbManager::removeEventHandler(EventHandler * evHandler, int usb_port)
{
    std::unique_lock<std::mutex> lock(_mutex);

    auto & eventHandlerVec  = _eventHandlerVec[usb_port]; //get the vector for this port
    for (auto it = eventHandlerVec.begin(); it != eventHandlerVec.end(); ++it) 
    {
        if (*it == evHandler) 
        {
            eventHandlerVec.erase(it);
            break;
        }
    }
}


int UsbManager::sendPacket(Packet &pkt, int ep, int usb_port, unsigned timeout_ms)
{   
    int transferred = 0;
    int ret = 0;    
    int length = pkt.getBufferLength();
    uint8_t *buf = pkt.getBuffer();
    pkt.setStatus(Packet::Status::CMD);   
    auto & usbHandler = _usbDevHandlerVec[usb_port];

    if (length > (int)Packet::MAX_SIZE) 
    {
        LOG_ERR(TAG, "Tx packet length(%d) > max(%d)", length, Packet::MAX_SIZE);
        std::exit(-1);
    }
   
    PRINT_PKT("tx:", pkt, _printMutex);  

    ret = libusb_bulk_transfer(usbHandler, ep, buf, length, &transferred, timeout_ms);

    switch (ret)
    {
    case LIBUSB_SUCCESS:
        break;
    case LIBUSB_ERROR_NO_DEVICE:
        LOG_ERR(TAG, "Device disconnected!");
        exit(ret);
        break;
    case LIBUSB_ERROR_PIPE:
        libusb_clear_halt(usbHandler, ep);
        break;
    default:
        LOG_ERR(TAG, "tx err = %s" , LIBUSB_ERR(ret));
        break;
    }

    if (transferred != length)
    {
        LOG_ERR(TAG, "Tx transferred bytes=%d, expected=%d" , transferred, length);
    }

    return ret == LIBUSB_SUCCESS ? transferred : ret;

}

int UsbManager::recvPacket(Packet &pkt, int ep, int usb_port, unsigned timeout_ms)
{
    int transferred = 0;
    int ret = 0;
    uint8_t *buf = pkt.getBuffer();
    int length = pkt.getBufferSize(); 
    auto & usbHandler = _usbDevHandlerVec[usb_port];

    if (length > (int)Packet::MAX_SIZE) 
    {
        LOG_ERR(TAG, "Rx packet length(%d) > max(%d)", length, Packet::MAX_SIZE);
        std::exit(-1);
    }

    pkt.reset();
    ret = libusb_bulk_transfer(usbHandler, ep, buf, length, &transferred, timeout_ms);
    pkt.flush(); 

    PRINT_PKT("rx:", pkt, _printMutex);   

    switch (ret)
    {
    case LIBUSB_SUCCESS:
        break;
    case LIBUSB_ERROR_NO_DEVICE:
        LOG_ERR(TAG, "Device disconnected!");
        std::exit(ret);
        break;
    case LIBUSB_ERROR_PIPE:
        libusb_clear_halt(usbHandler, ep);
        break;
    default:
        LOG_ERR(TAG, "Rx err = %s" , LIBUSB_ERR(ret));
        break;
    }

    bool pktErr = pkt.checkErr(transferred);

    if (pktErr)
    {
        LOG_ERR(TAG, "Tx transferred bytes=%d" , transferred);
    }

    return ret == LIBUSB_SUCCESS ? transferred : ret;   
}

int UsbManager::transfer(Packet &txPkt, Packet &rxPkt, int usb_port, unsigned timeout_ms)
{
    checkAndInitialize(usb_port);

    std::lock_guard<std::mutex> lock(_mutex); 
    
    int txRet = 0;
    int rxRet = 0;
    
    uint64_t seqNum = _pktSeqNum++ & MAX_PKT_SEQ_NUM; // range 0..maxPktSeqNum
    txPkt.setSeqNum(seqNum);        

    int retry = 4;
    while (retry-- > 0) 
    {                           
        txRet = sendPacket(txPkt, CDC_DATA_EP_OUT, usb_port, timeout_ms);
        rxRet = recvPacket(rxPkt, CDC_DATA_EP_IN, usb_port, timeout_ms);       

        if (rxRet > 0 && txRet > 0) 
        {
            break; //success
        } 
        
        continue;        
    }

    if (retry < 0) 
    {   
        LOG_ERR(TAG, "Impossible to transfer!");       
        std::exit(-1);
    }

    if (rxPkt.getType() != txPkt.getType())
    {
        LOG_ERR(TAG, "Tx/Rx packet type mismatch!");  
    }

    if (rxPkt.getSeqNum() != seqNum)
    {
        LOG_ERR(TAG, "Error: Invalid packet sequence number = %d, expected =  %d" , (int)rxPkt.getSeqNum() , (int)seqNum);  
        std::exit(-1);
    }   

    return 0;   
}

void ioig::UsbManager::checkAndInitialize(int usb_port)
{ 
    std::unique_lock<std::mutex> lock(_mutex);

    if (usb_port >= MAX_USB_DEVICES)
    {
        LOG_ERR(TAG, "Can't request USB device index %d, max index = %d", usb_port , MAX_USB_DEVICES-1);
        std::exit(-1);
    }

    if (_usbIndexInitMap.test(usb_port)) //already initialized
    {           
        return;
    }
    _usbIndexInitMap.set(usb_port);    


    if (_usbDevHandlerVec.empty()) 
    {
        for (unsigned i=0; i < MAX_USB_DEVICES; i++) 
        {        
            _usbDevHandlerVec.push_back( nullptr );        
            _usbContextVec.push_back( nullptr );
        }
    }
    // Initialize libusb for the given context
    if (libusb_init(&_usbContextVec[usb_port]) != LIBUSB_SUCCESS) 
    {
        LOG_ERR(TAG, "Failed to initialize libusb!");        
        std::exit(-1);            
    }    

    _running.store(true);    

    initUsbDevice(usb_port);
    lock.unlock();

    sendResetCmd(usb_port);
}


