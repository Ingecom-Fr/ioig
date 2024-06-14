#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GP0  = 0,
    GP1  = 1,
    GP2  = 2,
    GP3  = 3,
    GP4  = 4,
    GP5  = 5,
    GP6  = 6,
    GP7  = 7,
    GP8  = 8,
    GP9  = 9,
    GP10 = 10,
    GP11 = 11,
    GP12 = 12,
    GP13 = 13,
    GP14 = 14,
    GP15 = 15,
    GP16 = 16,
    GP17 = 17,
    GP18 = 18,
    GP19 = 19,
    GP20 = 20,
    GP21 = 21,
    GP22 = 22,
    GP23 = 23,
    GP24 = 24,
    GP25 = 25,
    GP26 = 26,
    GP27 = 27,
    GP28 = 28,
    GP29 = 29,

    // ADC internal channels
    ADC_TEMP = 0xF0,
    ADC_VREF = 0xF1,

    A0  = GP26,
    A1  = GP27,
    A2  = GP28,
    A3  = GP29,
    ANALOGIN0 = A0,
    ANALOGIN1 = A1,
    ANALOGIN2 = A2,
    ANALOGIN3 = A3,

    PWM0_A = GP0,
    PWM0_B = GP1,
    PWM1_A = GP2,
    PWM1_B = GP3,
    PWM2_A = GP4,
    PWM2_B = GP5,
    PWM3_A = GP6,
    PWM3_B = GP7,
    PWM4_A = GP8,
    PWM4_B = GP9,
    PWM5_A = GP10,
    PWM5_B = GP11,
    PWM6_A = GP12,
    PWM6_B = GP13,
    PWM7_A = GP14,
    PWM7_B = GP15,

    LED1 = GP25,
    LED2 = GP25,
    LED3 = GP25,
    LED4 = GP25,

    // Not connected
    NC = (int)0xFFFFFFFF
} Pin;


typedef enum {
    UART_0 = 0,
    UART_1,
} UART_Instance;

typedef enum {
    ADC0 = 0,
} ADC_Instance;

typedef enum {
    SPI_0 = 0,
    SPI_1
} SPI_Instance;

typedef enum {
    I2C_0 = 0,
    I2C_1
} I2C_Instance;

typedef enum {
    PWM_0 = 0,
    PWM_1,
    PWM_2,
    PWM_3,
    PWM_4,
    PWM_5,
    PWM_6,
    PWM_7
} PWM_Instance;


#define UART0_PINOUT0 GP0, GP1
#define UART0_PINOUT1 GP12, GP13
#define UART0_PINOUT2 GP16, GP17

#define UART1_PINOUT0 GP4, GP5
#define UART1_PINOUT1 GP8, GP9
#define UART1_PINOUT2 GP14, GP15

#define UART_INSTANCES 2

#define SPI0_PINOUT0 GP18,GP19,GP16,GP17
#define SPI0_PINOUT1 GP2,GP3,GP4,GP5

#define SPI1_PINOUT0 GP10,GP11,GP12,GP13
#define SPI1_PINOUT1 GP6,GP7,GP8,GP9

#define SPI_INSTANCES 2

#define I2C0_PINOUT0 GP4,GP5
#define I2C0_PINOUT1 GP0,GP1
#define I2C0_PINOUT2 GP20,GP21

#define I2C1_PINOUT0 GP18,GP19
#define I2C1_PINOUT1 GP10,GP11
#define I2C1_PINOUT2 GP6,GP7

#define I2C_INSTANCES 2

#define TARGET_PINS_COUNT 30

#ifdef __cplusplus
}
#endif



