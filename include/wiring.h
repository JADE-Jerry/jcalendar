
#ifndef __WIRING_H__
#define __WIRING_H__

// #define J_INKSCREEN_PRO
#define LOLIN32_LITE

#ifdef J_INKSCREEN_PRO
// Output PIN
#define SPI_MOSI GPIO_NUM_23
#define SPI_MISO GPIO_NUM_19
#define SPI_SCK GPIO_NUM_18
#define SPI_CS GPIO_NUM_5
#define SPI_DC GPIO_NUM_17
#define SPI_RST GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4
// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
// Other PIN
#define KEY_M GPIO_NUM_14 // 注意：由于此按键负责唤醒，因此需要选择支持RTC唤醒的PIN脚。
#define KEY_L GPIO_NUM_35
#define KEY_R GPIO_NUM_13

#define PIN_LED_R GPIO_NUM_33
#define PIN_LED_B GPIO_NUM_25
#define PIN_LED_G GPIO_NUM_26

#define PIN_ADC GPIO_NUM_32 // ADC
#endif


#ifdef LOLIN32_LITE    
// Output PIN
#define SPI_MOSI GPIO_NUM_23
#define SPI_MISO GPIO_NUM_19
#define SPI_SCK GPIO_NUM_18

#define SPI_CS GPIO_NUM_5
#define SPI_DC GPIO_NUM_17
#define SPI_RST GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4
// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
// Other PIN
#define KEY_M GPIO_NUM_14 // 注意：由于此按键负责唤醒，因此需要选择支持RTC唤醒的PIN脚。
#define PIN_LED_R GPIO_NUM_22

#define PIN_ADC GPIO_NUM_32 // ADC
#endif

#endif
