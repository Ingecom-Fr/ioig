#include <hardware/irq.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include "fw/tasks/serial.h"
#include "fw/main.h"

SerialTask &serialTask = SerialTask::instance();

SerialTask::SerialTask()
{
    _rxReady[0] = false;
    _rxReady[1] = false;
}


void SerialTask::init()
{
    _rxReady[0] = false;
    _rxReady[1] = false;
    setState(Task::State::RUNNING);
}

void SerialTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);    
    _rxReady[0] = false;
    _rxReady[1] = false;
    sleep_ms(2);
    setState(prevState);    
}


void SerialTask::onRxInterrupt(uart_inst_t *uart_port)
{
    if (getStateUnsafe() == Task::State::STOPPED)
    {
      return;
    }    
    int idx = uart_port == uart0 ? 0 : 1;

    if (uart_is_readable(uart_port)) 
    {
       critical_section_enter_blocking(&serialTask._critSection);	
       _rxReady[idx] = true;
       _irqRxC[idx] = uart_getc(uart_port);
       critical_section_exit(&serialTask._critSection);
    }   
}


void SerialTask::irqHandlerUART0_Rx(void)
{
    serialTask.onRxInterrupt(uart0);
}

void SerialTask::irqHandlerUART1_Rx(void)
{
    serialTask.onRxInterrupt(uart1);
}


inline void SerialTask::processInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;   
    auto tx   = rxPkt.getPayloadItem8(1);
    auto rx   = rxPkt.getPayloadItem8(2);
    auto baud = rxPkt.getPayloadItem32(3);

    if (!uart_is_enabled(hwInstance)) 
    {
        uart_init(hwInstance, baud);

        gpio_set_function(tx, GPIO_FUNC_UART);
        gpio_set_function(rx, GPIO_FUNC_UART);

        uart_set_fifo_enabled(hwInstance, false);  
    }

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(tx);
    txPkt.addPayloadItem8(rx);
    txPkt.addPayloadItem32(baud);

}

inline void SerialTask::processDeInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;
    uart_deinit(hwInstance);
    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));

}

inline void SerialTask::processSetBaud(Packet &rxPkt, Packet &txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1; 
    auto baud = rxPkt.getPayloadItem32(1);    

    uart_init(hwInstance, baud);
    uart_set_fifo_enabled(hwInstance, false);

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem32(baud);

}

inline void SerialTask::processSetFormat(Packet & rxPkt, Packet & txPkt)
{
    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1; 
    auto parity    = rxPkt.getPayloadItem8(1);
    auto data_bits = rxPkt.getPayloadItem8(2);
    auto stop_bits = rxPkt.getPayloadItem8(3);

    uart_parity_t hal_parity=UART_PARITY_NONE;
    switch (parity) {
        case ParityNone:
            hal_parity = UART_PARITY_NONE;
            break;
        case ParityOdd:
            hal_parity = UART_PARITY_ODD;
            break;
        case ParityEven:
            hal_parity = UART_PARITY_EVEN;
            break;
        default:            
            break;
    }
    uart_set_format(hwInstance, data_bits, stop_bits, hal_parity);

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(parity);
    txPkt.addPayloadItem8(data_bits);
    txPkt.addPayloadItem8(stop_bits);

}


inline void SerialTask::processSetIrq(Packet & rxPkt, Packet & txPkt)
{
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1; 
    auto enable = rxPkt.getPayloadItem8(1); 
    auto irq = rxPkt.getPayloadItem8(2); 
    auto uart_irq = hwInstance == uart0 ? UART0_IRQ : UART1_IRQ;

    auto & callback = hwInstance == uart0 ? irqHandlerUART0_Rx : irqHandlerUART1_Rx;
    _rxReady[0] = false;
    _rxReady[1] = false;

    irq_set_exclusive_handler(uart_irq, callback);      

    irq_set_enabled(uart_irq, enable);
    uart_set_irq_enables(hwInstance, irq == RxIrq, irq == TxIrq);    

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(enable);
    txPkt.addPayloadItem8(irq);
}

inline void SerialTask::processSetFlowControl(Packet & rxPkt, Packet & txPkt)
{
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1; 
    auto pin_rts   = rxPkt.getPayloadItem8(1);
    auto pin_cts   = rxPkt.getPayloadItem8(2);
    auto type      = rxPkt.getPayloadItem8(3);

    gpio_set_function(pin_rts, GPIO_FUNC_UART);
    gpio_set_function(pin_cts, GPIO_FUNC_UART);
    
    if (type == FlowControlRTSCTS) 
    {
        uart_set_hw_flow(hwInstance, true, true);
    } else {
        uart_set_hw_flow(hwInstance, type == FlowControlCTS, type == FlowControlRTS);
    }

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(pin_rts);
    txPkt.addPayloadItem8(pin_cts);
    txPkt.addPayloadItem8(type);
}

inline void SerialTask::processReadable(Packet & rxPkt, Packet & txPkt)
{
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;     
    txPkt.addPayloadItem8(uart_is_readable(hwInstance));
}

inline void SerialTask::processWritable(Packet & rxPkt, Packet & txPkt)
{    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1; 
    txPkt.addPayloadItem8(uart_is_writable(hwInstance));
}

inline void SerialTask::processSetBreak(Packet & rxPkt, Packet & txPkt)
{    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;
    auto state = rxPkt.getPayloadItem8(1);
    uart_set_break(hwInstance, state);
}

inline void SerialTask::processGetC(Packet & rxPkt, Packet & txPkt)
{
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;

    if (!uart_is_readable(hwInstance))
    {
        txPkt.setStatus(Packet::Status::RSP_SERIAL_NOT_READABLE);
        return;
    }

    txPkt.addPayloadItem8(uart_getc(hwInstance));
}

inline void SerialTask::processPutC(Packet & rxPkt, Packet & txPkt)
{    
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;
    char c  = (char)rxPkt.getPayloadItem8(1);
    
    while (!uart_is_writable(hwInstance)) 
    {
        txPkt.setStatus(Packet::Status::RSP_SERIAL_NOT_WRITABLE);
        return;
    }

    uart_putc_raw(hwInstance, c);

    txPkt.addPayloadItem8(c);
}

void SerialTask::processWrite(Packet & rxPkt, Packet & txPkt)
{        
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;
    auto len = rxPkt.getPayloadItem8(1);
    uint8_t * buf = rxPkt.getPayloadBuffer(2);
    
    if (uart_is_writable(hwInstance)) 
    {
        uart_write_blocking(hwInstance, buf, len);
    }else 
    {
        txPkt.setStatus(Packet::Status::RSP_SERIAL_NOT_WRITABLE);
    }    
}

void SerialTask::processRead(Packet & rxPkt, Packet & txPkt)
{
    auto hwInstance = rxPkt.getPayloadItem8(0) == UART_0 ? uart0 : uart1;
    auto len = rxPkt.getPayloadItem8(1);

    if (uart_is_readable(hwInstance)) 
    {
        uart_read_blocking(hwInstance, txPkt.getBuffer(), len);
    }else 
    {
        txPkt.setStatus(Packet::Status::RSP_SERIAL_NOT_READABLE);
    }
}

void SerialTask::processEvent(Packet & rxPkt, Packet & txPkt)
{   
    
    for (int i=0 ; i < 2 ; i++) //UART0 , UART1
    {
        critical_section_enter_blocking(&_critSection);
        if (_rxReady[i]) 
        {
            txPkt.addPayloadItem8(i); //UART0 , UART1
            txPkt.addPayloadItem8(_irqRxC[i]);
            _rxReady[i] = false;
        }        
        critical_section_exit(&_critSection);

        if (txPkt.getPayloadLength() > 0)             
        {
          mainTask.cdcWrite(CDCItf::EVENT, txPkt.getBuffer(), txPkt.getBufferLength());
        }                
    }
}

void SerialTask::process(Packet &rxPkt,Packet &txPkt)
{
     
    CHECK_STATE();  

    auto rxPktType = rxPkt.getType();   

    switch (rxPktType)
    {
    case Packet::Type::SERIAL_INIT:
        processInit(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_DEINIT:
        processDeInit(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_SET_BAUD:
        processSetBaud(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_SET_FORMAT:
        processSetFormat(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_SET_IRQ:
        processSetIrq(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_SET_FLOW_CONTROL:
        processSetFlowControl(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_READABLE:
        processReadable(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_WRITABLE:
        processWritable(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_SET_BREAK:
        processSetBreak(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_GETC:
        processGetC(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_PUTC:
        processPutC(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_EVENT:
        processEvent(rxPkt,txPkt);
    break;
    default:                   
        break;
    } 
                
        
}

