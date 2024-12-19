#include <hardware/irq.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include "fw/tasks/serial.h"
#include "fw/main.h"

SerialTask &serialTask = SerialTask::instance();

SerialTask::SerialTask()
{
}


void SerialTask::init()
{ 
    setState(Task::State::RUNNING);
}

void SerialTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);    
    sleep_ms(2); 
    for (int q=0; q < UART_INSTANCES ; q++) 
    {
        while (queue_get_level(&_irqEventQueue[q]) > 0) 
        {
          uint32_t tmp=0;
          queue_remove_blocking(&_irqEventQueue[q], &tmp);
        }     
    }
    setState(prevState);    
}


void SerialTask::onRxInterrupt(uart_inst_t *uart_port)
{
    if (getStateUnsafe() == Task::State::STOPPED)
    {
      return;
    }     

    if (uart_is_readable(uart_port)) 
    {  
        int uart_num = uart_port == uart0 ? UART_0 : UART_1;
        char c = uart_getc(uart_port);
    
        if (!queue_try_add(&_irqEventQueue[uart_num], &c))
        {   
            //queue if full
            char tmp=0;
            queue_remove_blocking(&_irqEventQueue[uart_num], &tmp);    
            queue_add_blocking(&_irqEventQueue[uart_num], &c);
            DBG_MSG("Serial: event queue full! discarging items...\n");
        }
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
        
        // Turn off FIFO's - we want to do this character by character
        uart_set_fifo_enabled(hwInstance, false);  
    }

    txPkt.addPayloadItem8(rxPkt.getPayloadItem8(0));
    txPkt.addPayloadItem8(tx);
    txPkt.addPayloadItem8(rx);
    txPkt.addPayloadItem32(baud);

}

inline void SerialTask::processDeInit(Packet & rxPkt, Packet & txPkt)
{     
    auto uart_num = rxPkt.getPayloadItem8(0);
    queue_free(&_irqEventQueue[uart_num % UART_INSTANCES]);
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
    auto uart_num = rxPkt.getPayloadItem8(0) % UART_INSTANCES;
    auto hwInstance = uart_num == UART_0 ? uart0 : uart1; 
    auto uart_irq = hwInstance == uart0 ? UART0_IRQ : UART1_IRQ;

    auto enable = rxPkt.getPayloadItem8(1); 
    auto irq = rxPkt.getPayloadItem8(2); 

    auto & callback = hwInstance == uart0 ? irqHandlerUART0_Rx : irqHandlerUART1_Rx;
    
    queue_init(&_irqEventQueue[uart_num], sizeof(char), EVT_QUEUE_MAX_SIZE);    

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

void SerialTask::processEvents(Packet & txPkt)
{   
    
    for (int q=0 ; q < UART_INSTANCES ; q++)
    {
        uint8_t qsz = (uint8_t)queue_get_level(&_irqEventQueue[q]);  
    
        int evt_cnt = qsz < txPkt.getFreePayloadSlots() ? qsz : txPkt.getFreePayloadSlots()-1 /*env_cnt slot*/;        
    
        if (evt_cnt == 0) 
        {
            continue;
        }

        txPkt.addPayloadItem8(evt_cnt); 
        //printf("evt_cnt=%d\n",evt_cnt);
    
        //chain events in a single pkt
        while (evt_cnt-- > 0) 
        {
            uint8_t evt_data;

            queue_remove_blocking(&_irqEventQueue[q], &evt_data);            
            txPkt.addPayloadItem8(evt_data);
            //printf("%c\n",evt_data);
        }
    
        mainTask.cdcWrite(CDCItf::EVENT, txPkt.getBuffer(), txPkt.getBufferLength());        
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
    case Packet::Type::SERIAL_READ:
        processRead(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_WRITE:
        processWrite(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_GETC:
        processGetC(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_PUTC:
        processPutC(rxPkt,txPkt);
    break;
    case Packet::Type::SERIAL_EVENT:
        processEvents(txPkt);
    break;
    default:                   
        break;
    } 
                
        
}

