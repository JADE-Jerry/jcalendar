#define BUSY_PIN GPIO_NUM_4
#define RES_PIN  GPIO_NUM_16
#define DC_PIN   GPIO_NUM_17
#define CS_PIN   GPIO_NUM_5
#define SCLK_PIN GPIO_NUM_18
#define MOSI_PIN GPIO_NUM_23

int si_calendar_status();
void si_calendar();

int si_wifi_status();
void si_wifi();

int si_weather_status();
void si_weather();

int si_screen_status();
void si_screen();

void print_status();
