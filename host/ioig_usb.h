#pragma once

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <bitset>
#include <memory>

#include "fw/device.h" //firmware definitions

#include "ioig_private.h"

namespace ioig
{

    class EventHandler  
    {
        public: 
            EventHandler() = default;
            virtual ~EventHandler() {}
            virtual void onEvent(Packet & evtPkt) = 0;
    };


    /**
     * @class UsbManager
     *
     * @brief Manages idividual IOIG devices.
     *
     * The UsbManager is the backend class for UsbDeviceHandler.
     * It provides functionality for handling USB Devices.
     */
    class UsbManager
    {    
    public:

        UsbManager(const UsbManager &) = delete;
        UsbManager &operator=(const UsbManager &) = delete;
        UsbManager() = delete;
        ~UsbManager() = delete;       

        /**
         * @brief Registers a peripheral to handle events
         * @param usb_port The usb device index
         * @param
         * 
         */     
        static void registerEventHandler(EventHandler * evHandler, int usb_port);
        static void removeEventHandler(EventHandler * evHandler, int usb_port);


        /**
         * @brief Transfers data to the device and waits for a response.
         * @note Blocking operation.
         *
         * @param txPkt The packet to transmit.
         * @param rxPkt The packet to receive.
         */
        static int transfer(Packet &txPkt, Packet &rxPkt, int usb_port, unsigned timeout_ms=600);

        
        
    private:

        static void checkAndInitialize(int usb_port);

 
        /**
         * @brief Deinitializes the USB host controller.
         */
        static void deinit();            


        static libusb_device_handle * getDeviceHandler(const unsigned usb_port);




        static bool openUsbDevice(int usb_port);
        static void initUsbDevice(int usb_port);

        static void closeUsbDevice(int usb_port);
        static void sendResetCmd(int usb_port);


        /**
         * @brief Sends a packet to the device.
         *
         * @param pkt The packet to send.
         * @param ep The endpoint.
         * @param timeout_ms Timeout in milliseconds.
         * @return zero on success, negative value on error
         */
        static int sendPacket(Packet &pkt, int ep, int usb_port, unsigned timeout_ms);

        /**
         * @brief Receives a packet from the device.
         *
         * @param pkt The packet received.
         * @param ep The endpoint.
         * @param timeout_ms Timeout in milliseconds.
         * @return zero on success, negative value on error
         */
        static int recvPacket(Packet &pkt, int ep, int usb_port, unsigned timeout_ms);


        static std::vector<libusb_device_handle *> _usbDevHandlerVec;
        static std::vector<libusb_context *> _usbContextVec;
        static std::vector<EventHandler *>  _eventHandlerVec[MAX_USB_DEVICES]; //An event vector per usb device
        static void eventThread(int usb_port);

        static std::atomic_bool _running;
        static std::mutex _mutex;
        static std::mutex _printMutex;
        static std::bitset<MAX_USB_DEVICES> _usbIndexInitMap;  /**< Each bit represents an usb index */
        
        static uint64_t _pktSeqNum;

        static constexpr unsigned MAX_PKT_SEQ_NUM = 255;
        static constexpr const char* TAG = "UsbManager";

    };


}


