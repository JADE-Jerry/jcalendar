
#ifndef __WIRING_H__
#define __WIRING_H__

#define LOLIN32_LITE



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
#define PIN_BUTTON GPIO_NUM_14 // 注意：由于此按键负责唤醒，因此需要选择支持RTC唤醒的PIN脚。
#define PIN_LED GPIO_NUM_22
#endif

#endif
