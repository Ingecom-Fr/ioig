#include <hardware/pwm.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <hardware/adc.h>

#include "fw/tasks/analog.h"
#include "fw/main.h"


AnalogTask &analogTask = AnalogTask::instance();


void AnalogTask::init()
{
    setState(Task::State::RUNNING);    
}

void AnalogTask::reset()
{
    auto prevState = getState();
    setState(Task::State::STOPPED);
    sleep_ms(2);
    setState(prevState);    
}


inline void AnalogTask::processInit(Packet & rxPkt, Packet & txPkt)
{
    
    auto pin=rxPkt.getPayloadItem8(0);    
    auto mode=rxPkt.getPayloadItem8(1);  
    
    //send back the received parameters to host
    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(mode);     


    if (mode==0) // AnalogIn
    {        
        adc_init();        


        adc_gpio_init(pin);    

        /* Check if the ADC channel we just configured belongs to the
         * temperature sensor. If that's the case, enable the temperature
         * sensor.
         */
        if (pin == ADC_TEMP)
        {
            adc_set_temp_sensor_enabled(true);            
        }
        
    }else      // AnalogOut
    {
        /* Populate PWM object with default values. */
        auto slice        = pwm_gpio_to_slice_num(pin);
        auto channel      = pwm_gpio_to_channel(pin);
        auto count_top    = rxPkt.getPayloadItem32(2);
        auto period_us    = rxPkt.getPayloadItem64(6);        

        auto cfg = pwm_get_default_config();
        pwm_config_set_wrap(&cfg, count_top);        

        // min_period should be 8us
        uint32_t min_period = 1'000'000 * count_top / clock_get_hz(clk_sys);
        pwm_config_set_clkdiv(&cfg, (float)period_us / (float)min_period);

        pwm_init(slice, &cfg, false);
        gpio_set_function(pin, GPIO_FUNC_PWM);           

        txPkt.addPayloadItem32(count_top);
        txPkt.addPayloadItem64(period_us);
        txPkt.addPayloadItem8(slice);
        txPkt.addPayloadItem8(channel);
    }

    

}

inline void AnalogTask::processDeInit(Packet & rxPkt, Packet & txPkt)
{    
    auto pin=rxPkt.getPayloadItem8(0);
    auto mode=rxPkt.getPayloadItem8(1);   

    //send back the received parameters to host
    txPkt.addPayloadItem8(pin);
    txPkt.addPayloadItem8(mode);     
    
    if (mode==1)   // AnalogOut
    {
        auto slice=rxPkt.getPayloadItem8(2);
        pwm_set_enabled(slice, false);
        gpio_deinit(pin);
        txPkt.addPayloadItem8(slice);     
    }
    gpio_deinit(pin); 

}

inline void AnalogTask::processWrite(Packet & rxPkt, Packet & txPkt)
{    
    auto pin          = rxPkt.getPayloadItem8(0); 
    auto slice        = rxPkt.getPayloadItem32(1);       
    auto val          = rxPkt.getPayloadItem16(5);
    auto count_top    = rxPkt.getPayloadItem32(7);
    auto wrResolution = rxPkt.getPayloadItem8(11);      
    

    float percent = (float)val/(float)((1 << wrResolution)-1);

    if (percent < 0) 
    {
        pwm_set_enabled(slice, false); //disable pwm
    }else {        
        pwm_set_gpio_level(pin, percent * (count_top + 1));
        pwm_set_enabled(slice, true);        
    }

    txPkt.addPayloadItemFloat(percent);
}

inline void AnalogTask::processRead(Packet & rxPkt, Packet & txPkt)
{    
    auto channel=rxPkt.getPayloadItem8(0);    
    auto readResolution=rxPkt.getPayloadItem8(1);  

    /* Select the desired ADC input channel. */
    adc_select_input(channel);
    
    /* Read the 16-Bit ADC value. */
    int adcRead16 =  adc_read() << (16 - ADC_RESOLUTION_BITS);

    uint16_t result = (adcRead16 >> (16 - readResolution));

    txPkt.addPayloadItem8(channel);
    txPkt.addPayloadItem16(result);

}


inline void AnalogTask::processReadTemp(Packet & rxPkt, Packet & txPkt)
{    
    adc_select_input(4);

    float adc = (float)adc_read() * ADC_CONVERSION_FACTOR;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;    
    
    txPkt.addPayloadItemFloat(tempC);
}



void AnalogTask::process(Packet &rxPkt,Packet &txPkt)
{     

    CHECK_STATE();  

    auto rxPktType = rxPkt.getType();   

    switch (rxPktType)
    {
    case Packet::Type::ANALOG_INIT:        
        processInit(rxPkt,txPkt);
    break;
    case Packet::Type::ANALOG_DEINIT:        
        processDeInit(rxPkt,txPkt);
    break;
    case Packet::Type::ANALOG_READ:
        processRead(rxPkt,txPkt);
    break;
    case Packet::Type::ANALOG_READ_TEMP:
        processReadTemp(rxPkt,txPkt);
    break;    
    case Packet::Type::ANALOG_WRITE:
        processWrite(rxPkt,txPkt);
    break;

    default:                   
        break;
    } 
                       
}

