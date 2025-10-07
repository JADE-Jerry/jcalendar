/*
电压 (V)	近似剩余电量	状态说明
4.20	100%	刚刚充满，充电器断开
4.10	~90%	电量非常充足
4.00	~80%	电量充足
3.90	~60%	中等电量
3.80	~50%	中等电量（接近标称电压）
3.75	~40%	电量偏低
3.70	~30%	标称电压点，但电量已不多
3.65	~20%	低电量
3.50	~10%	极低电量，应立即充电
3.30	0%	放电截止电压，继续放电将损坏电池

充电截止电压 4.2
放电截止电压 3.3
 */
#include "battery.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <Arduino.h>

/**
 * 获取电池电压（mV）
 */
int readBatteryVoltage() {
    static const adc1_channel_t channel = ADC1_CHANNEL_4;  // GPIO32
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, ADC_ATTEN_11db);
    adc_power_acquire();
    delay(10);
    int adc_val = adc1_get_raw(channel);
    adc_power_release();

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    int voltage = esp_adc_cal_raw_to_voltage(adc_val, &adc_chars);
    voltage *= 2;

    return voltage;
}