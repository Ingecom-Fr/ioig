#include "tasks/analog.h"
#include "tasks/gpio.h"
#include "tasks/i2c.h"
#include "tasks/serial.h"
#include "tasks/spi.h"

#include "tusb.h"

#define PKT_DEBUG 0
#define ENABLE_OVERCLOCK 0
#define DBG_INTERVAL_MS 10000
#define ENABLE_MULTICORE 1
#define ENABLE_WD 1

MainTask & mainTask = MainTask::instance();
Board & board = Board::instance();

//================================================================================
// Task 
//================================================================================

Task::Task() : _state(State::NONE) 
{
  mutex_init(&_mutex);
}

void Task::setState(State new_state) 
{ 
  mutex_enter_blocking(&_mutex);
  _state = new_state; 
  mutex_exit(&_mutex);
}

Task::State Task::getState() 
{ 
  auto ret = State::NONE;
  mutex_enter_blocking(&_mutex);
  ret = _state; 
  mutex_exit(&_mutex);
  return ret; 
}

//================================================================================
// MainTask
//================================================================================


void MainTask::init()
{
  _rxPktIdx = 0;

  queue_init(&_rxPktIndexQueue, sizeof(uint32_t), RX_PKT_QUEUE_MAX_SIZE);
  
  analogTask.init();
  gpioTask.init();
  i2cTask.init();
  spiTask.init();
  serialTask.init();

#if ENABLE_MULTICORE
  multicore_reset_core1();
  multicore_launch_core1(mainTask.mainLoop1);
#endif  
  mainLoop0();
}

void MainTask::reset()
{
  auto prevState = Task::getState();

  setState(Task::State::STOPPED);

  sleep_ms(2); //wait stable state on getStateUnsafe()

  tud_cdc_n_read_flush(CDCItf::DATA);
  tud_cdc_n_write_flush(CDCItf::DATA);

  tud_cdc_n_read_flush(CDCItf::EVENT);
  tud_cdc_n_write_flush(CDCItf::EVENT);

  while (queue_get_level(&_rxPktIndexQueue) > 0) 
  {
    uint32_t tmp=0;
    queue_remove_blocking(&_rxPktIndexQueue, &tmp);
  }
  _rxPktIdx = 0;

  analogTask.reset();
  gpioTask.reset();
  i2cTask.reset();
  serialTask.reset();
  spiTask.reset();

  setState(prevState);
}

void MainTask::process(Packet &rxPkt, Packet &txPkt)
{
 
  rxPkt.flush();
  txPkt.reset();
  txPkt.cloneHeader(rxPkt);
  txPkt.setStatus(Packet::Status::RSP);

  auto op = rxPkt.getType();
  switch (op)
  {
    case Packet::Type::SYS_INIT:
      break;
    case Packet::Type::SYS_DEINIT:
      break;
    case Packet::Type::SYS_GET_FW_VER:
      break;
    case Packet::Type::SYS_HW_RESET:
      printf("Hw reset cmd from host!\n");
      board.reset();
      break;      
    case Packet::Type::SYS_SW_RESET:
      reset();
      printf("Sw reset cmd from host!\n");
    break;
    default:          
    break;
  }

  gpioTask.process(rxPkt, txPkt);
  spiTask.process(rxPkt, txPkt);
  analogTask.process(rxPkt, txPkt);
  i2cTask.process(rxPkt, txPkt);
  serialTask.process(rxPkt, txPkt);

  txPkt.flush();
  
  if (txPkt.getType() != Packet::Type::GPIO_EVENT && txPkt.getType() != Packet::Type::SERIAL_EVENT) 
  {
    mainTask.cdcWrite(CDCItf::DATA, txPkt.getBuffer(), txPkt.getBufferLength());  
  }

}


void MainTask::mainLoop0()
{
  setState(Task::State::RUNNING);

  while (1)
  {
    tud_task(); // tinyusb device task    

    board.ledBlink();

#if ENABLE_WD
    watchdog_update();
#endif    
  }
}




void MainTask::mainLoop1()
{
  auto &rxPktIndexQueue  = mainTask._rxPktIndexQueue; 
  auto &rxPacketVec = mainTask._rxPacketVec;
  uint32_t rxPktIdx=0; 
  Packet eventReqPkt(2);
  Packet txPkt;  

  while (1)
  {        
    //----------------------------------------
    //poll events
    //----------------------------------------
    eventReqPkt.setType(Packet::Type::GPIO_EVENT);
    mainTask.process(eventReqPkt, txPkt);
  
    eventReqPkt.setType(Packet::Type::SERIAL_EVENT);
    mainTask.process(eventReqPkt, txPkt);
    

    //----------------------------------------
    //process incoming commands
    //----------------------------------------
    rxPktIdx=0;
    if (!queue_try_remove(&rxPktIndexQueue, &rxPktIdx)) 
    {
      continue; //nothing to do
    }

    Packet & rxPkt = rxPacketVec[rxPktIdx]; //get the next available rx packet index
    mainTask.process(rxPkt, txPkt);
 
  }
}

inline void MainTask::onRx(const CDCItf itf)
{

  if ( mainTask.getStateUnsafe() == Task::State::STOPPED ) 
  {
    return;
  }

  uint32_t idx = _rxPktIdx++ % RX_PKT_QUEUE_MAX_SIZE; //circular buff

  Packet & rxPkt = _rxPacketVec[idx];

  mainTask.cdcRead(itf, rxPkt.getBuffer() , Packet::MAX_SIZE);

  if (!queue_try_add(&_rxPktIndexQueue, &idx))
  {
    DBG_MSG("Error: rx queue is full! discarding an item...\n");
    int lostIdx;
    queue_remove_blocking(&_rxPktIndexQueue, &lostIdx);
    queue_add_blocking(&_rxPktIndexQueue, &idx);
  }    

}

inline void MainTask::onTx(const CDCItf itf)
{

}

inline
void MainTask::cdcRead(const CDCItf itf, uint8_t *buf, const unsigned len)
{ 

  // Check if there is any data available in the CDC buffer.
  if (tud_cdc_n_available(itf) == 0)
  {
    tud_cdc_n_read_flush(itf);
  }

  tud_cdc_n_read(itf, buf, len);

  // Clear the received FIFO
  tud_cdc_n_read_flush(itf);
  
}

void MainTask::cdcWrite(const CDCItf itf, uint8_t *buf, const unsigned len)
{
  //https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/USBCDC.cpp

  int retry=10;
  size_t to_send = len, so_far = 0;
  while (to_send && retry-- > 0)
  {
    size_t space = tud_cdc_n_write_available(itf);
    if (!space)
    {
      tud_cdc_n_write_flush(itf);
      DBG_MSG("Waiting space on wr fifo itf=%d, retry flush... (%d/10)\n", itf, 10-retry);
      busy_wait_us_32(200);
      continue;
    }
    if (space > to_send)
    {
      space = to_send;
    }
    size_t sent = tud_cdc_n_write(itf, buf + so_far, space);
    if (sent)
    {
      so_far += sent;
      to_send -= sent;
      tud_cdc_n_write_flush(itf);
    }
    else
    {
      break;
    }

  }
}

//================================================================================
// Board
//================================================================================

void Board::init()
{
  board_init();  
  stdio_init_all();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  printf("IoIg Init\n");

#if ENABLE_WD
  // Enable the watchdog, requiring the watchdog to be updated every Xms or the chip will reboot
  // second arg is pause on debug which means the watchdog will pause when stepping through code
  watchdog_enable(2000,true);

  if (watchdog_caused_reboot())
  {
    printf("Rebooted by watchdog!\n");        
  }
  else
  {
    printf("Clean boot\n");
  }
#endif    
  
#if ENABLE_OVERCLOCK
  //FIXME: This freq brakes the UART output 

  // Set the system clock to 200 MHz
	if (!set_sys_clock_khz(200000, false)) 
  {
    DBG_MSG("Warning: Can't set system clock!\n");
  }
#endif   
}


void Board::ledBlink()
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if (board_millis() - start_ms < blinkInterval_ms)
    return; // not enough time
  start_ms += blinkInterval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle  
}


void Board::reset()
{
  watchdog_enable(1, 1); // Enable the watchdog timer 
  while (1) 
  { 
    // Do nothing.
    // force watchdog to reset the system
  }  
}

uint32_t Board::getTotalHeap()
{
   extern char __StackLimit, __bss_end__;
   
   return &__StackLimit  - &__bss_end__;  
}

uint32_t Board::getFreeHeap()
{
  struct mallinfo m = mallinfo();
  return getTotalHeap() - m.uordblks;
}

void Board::onDeviceMounted()
{
  blinkInterval_ms = BlinkPattern::BLINK_MOUNTED;
}

void Board::onDeviceUnMounted()
{
  blinkInterval_ms = BlinkPattern::BLINK_NOT_MOUNTED;
}

void Board::onSuspend()
{
  blinkInterval_ms = BlinkPattern::BLINK_SUSPENDED;
}

void Board::onReseume()
{
  blinkInterval_ms = BlinkPattern::BLINK_MOUNTED;
}



//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+
int main(void)
{
  
  board.init();
  mainTask.init();

  return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  board.onDeviceMounted();  
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  board.onDeviceUnMounted();  
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  board.onSuspend();  
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  board.onReseume();  
}


// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void)rts;
  (void)dtr;
  (void)rts;
}


// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{  
  mainTask.onRx((CDCItf)itf);
}

//This callback is called when the transmission of data on the CDC interface (itf) is complete. 
//use this callback to perform actions after successfully transmitting data.
void tud_cdc_tx_complete_cb(uint8_t itf)
{
  mainTask.onTx((CDCItf)itf);
}

