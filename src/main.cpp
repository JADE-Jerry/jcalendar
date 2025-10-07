#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>

#include "esp_sleep.h"

#include <wiring.h>

#include "battery.h"

#include "led.h"
#include "_sntp.h"
#include "weather.h"
#include "screen_ink.h"
#include "_preference.h"

#include "version.h"

#include "OneButton.h"
OneButton button(KEY_M, true);

void IRAM_ATTR checkTicks() {
    button.tick();
}

WiFiManager wm;
WiFiManagerParameter para_qweather_host("qweather_host", "和风天气Host", "", 64); //     和风天气key
WiFiManagerParameter para_qweather_key("qweather_key", "和风天气API Key", "", 32); //     和风天气key
// const char* test_html = "<br/><label for='test'>天气模式</label><br/><input type='radio' name='test' value='0' checked> 每日天气test </input><input type='radio' name='test' value='1'> 实时天气test</input>";
// WiFiManagerParameter para_test(test_html);
WiFiManagerParameter para_qweather_type("qweather_type", "天气类型（0:每日天气，1:实时天气）", "0", 2, "pattern='\\[0-1]{1}'"); //     城市code
WiFiManagerParameter para_qweather_location("qweather_loc", "位置ID", "", 64); //     城市code
WiFiManagerParameter para_cd_day_label("cd_day_label", "倒数日（4字以内）", "", 10); //     倒数日
WiFiManagerParameter para_cd_day_date("cd_day_date", "日期（yyyyMMdd）", "", 8, "pattern='\\d{8}'"); //     城市code
WiFiManagerParameter para_tag_days("tag_days", "日期Tag（yyyyMMddx，详见README）", "", 30); //     日期Tag
WiFiManagerParameter para_si_week_1st("si_week_1st", "每周起始（0:周日，1:周一）", "0", 2, "pattern='\\[0-1]{1}'"); //     每周第一天
WiFiManagerParameter para_study_schedule("study_schedule", "课程表", "0", 4000, "pattern='\\[0-9]{3}[;]$'"); //     每周第一天

void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        uint64_t status = esp_sleep_get_ext1_wakeup_status();
        if (status == 0) {
            Serial.println(" *None of the configured pins woke us up");
        } else {
            Serial.print(" *Wakeup pin mask: ");
            Serial.printf("0x%016llX\r\n", status);
            for (int i = 0; i < 64; i++) {
                if ((status >> i) & 0x1) {
                    Serial.printf("  - GPIO%d\r\n", i);
                }
            }
        }
        break;
    }
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep.\r\n");
    }
}

void buttonClick(void* oneButton);
void buttonDoubleClick(void* oneButton);
void buttonLongPressStop(void* oneButton);
void go_sleep();

unsigned long _idle_millis;
unsigned long TIME_TO_SLEEP = 180 * 1000;

bool _wifi_flag = false;
unsigned long _wifi_failed_millis;
void setup() {
    delay(10);
    Serial.begin(115200);
    Serial.println(".");
    print_wakeup_reason();
    Serial.println("\r\n\r\n");
    delay(10);


    Serial.printf("***********************\r\n");
    Serial.printf("      J-Calendar\r\n");
    Serial.printf("    version: %s\r\n", J_VERSION);
    Serial.printf("***********************\r\n\r\n");
    Serial.printf("Copyright © 2022-2025 JADE Software Co., Ltd. All Rights Reserved.\r\n\r\n");

    led_init();
    led_on();
    delay(1000);
    int voltage = readBatteryVoltage();
    Serial.printf("Battery: %d mV\r\n", voltage);
    if(voltage < 2500) {
        Serial.println("[INFO]电池损坏或无ADC电路。");
    } else if(voltage < 3000) {
        Serial.println("[WARN]电量低于3v，系统休眠。");
        go_sleep();
    } else if (voltage < 3300) {
        // 低于3.3v，电池电量用尽，屏幕给警告，然后关机。 
        Serial.println("[WARN]电量低于3.3v，警告并系统休眠。");
        si_warning("电量不足，请充电！");
        go_sleep();
    } else if (voltage > 4400) {
        Serial.println("[INFO]未接电池。");
    }

    button.setClickMs(300);
    button.setPressMs(3000); // 设置长按的时长
    button.attachClick(buttonClick, &button);
    button.attachDoubleClick(buttonDoubleClick, &button);
    // button.attachMultiClick()
    button.attachLongPressStop(buttonLongPressStop, &button);
    attachInterrupt(digitalPinToInterrupt(KEY_M), checkTicks, CHANGE);

    Serial.println("Wm begin...");
    led_fast();
    wm.setHostname("J-Calendar");
    wm.setEnableConfigPortal(false);
    wm.setConnectTimeout(10);
    if (wm.autoConnect()) {
        Serial.println("Connect OK.");
        led_on();
        _wifi_flag = true;
    } else {
        Serial.println("Connect failed.");
        _wifi_flag = false;
        _wifi_failed_millis = millis();
        led_slow();
        _sntp_exec(2);
        weather_exec(2);
        WiFi.mode(WIFI_OFF); // 提前关闭WIFI，省电
        Serial.println("Wifi closed.");
    }
}

/**
 * 处理各个任务
 * 1. sntp同步
 *      前置条件：Wifi已连接
 * 2. 刷新日历
 *      前置条件：sntp同步完成（无论成功或失败）
 * 3. 刷新天气信息
 *      前置条件：wifi已连接
 * 4. 系统配置
 *      前置条件：无
 * 5. 休眠
 *      前置条件：所有任务都完成或失败，
 */
void loop() {
    button.tick(); // 单击，刷新页面；双击，打开配置；长按，重启
    wm.process();
    // 前置任务：wifi已连接
    // sntp同步
    if (_sntp_status() == -1) {
        _sntp_exec();
    }
    // 如果是定时器唤醒，并且接近午夜（23:50之后），则直接休眠
    if (_sntp_status() == SYNC_STATUS_TOO_LATE) {
        go_sleep();
    }
    // 前置任务：wifi已连接
    // 获取Weather信息
    if (weather_status() == -1) {
        weather_exec();
    }

    // 刷新日历
    // 前置任务：sntp、weather
    // 执行条件：屏幕状态为待处理
    if (_sntp_status() > 0 && weather_status() > 0 && si_screen_status() == -1) {
        // 数据获取完毕后，关闭Wifi，省电
        if (!wm.getConfigPortalActive()) {
            WiFi.mode(WIFI_OFF);
        }
        Serial.println("Wifi closed after data fetch.");

        si_screen();
    }

    // 休眠
    // 前置条件：屏幕刷新完成（或成功）

    // 未在配置状态，且屏幕刷新完成，进入休眠
    if (!wm.getConfigPortalActive() && si_screen_status() > 0) {
        if (_wifi_flag) {
            go_sleep();
        }
        if (!_wifi_flag && millis() - _wifi_failed_millis > 10 * 1000) { // 如果wifi连接不成功，等待10秒休眠
            go_sleep();
        }
    }
    // 配置状态下，
    if (wm.getConfigPortalActive() && millis() - _idle_millis > TIME_TO_SLEEP) {
        go_sleep();
    }

    delay(10);
}


// 刷新页面
void buttonClick(void* oneButton) {
    Serial.println("Button click.");
    if (wm.getConfigPortalActive()) {
        Serial.println("In config status.");
    } else {
        Serial.println("Refresh screen manually.");
        Preferences pref;
        pref.begin(PREF_NAMESPACE);
        int _si_type = pref.getInt(PREF_SI_TYPE);
        pref.putInt(PREF_SI_TYPE, _si_type == 0 ? 1 : 0);
        pref.end();
        si_screen();
    }
}

void saveParamsCallback() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.putString(PREF_QWEATHER_HOST, para_qweather_host.getValue());
    pref.putString(PREF_QWEATHER_KEY, para_qweather_key.getValue());
    pref.putString(PREF_QWEATHER_TYPE, strcmp(para_qweather_type.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_QWEATHER_LOC, para_qweather_location.getValue());
    pref.putString(PREF_CD_DAY_LABLE, para_cd_day_label.getValue());
    pref.putString(PREF_CD_DAY_DATE, para_cd_day_date.getValue());
    pref.putString(PREF_TAG_DAYS, para_tag_days.getValue());
    pref.putString(PREF_SI_WEEK_1ST, strcmp(para_si_week_1st.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_STUDY_SCHEDULE, para_study_schedule.getValue());
    pref.end();

    Serial.println("Params saved.");

    _idle_millis = millis(); // 刷新无操作时间点

    ESP.restart();
}

void preSaveParamsCallback() {
}

// 双击打开配置页面
void buttonDoubleClick(void* oneButton) {
    Serial.println("Button double click.");
    if (wm.getConfigPortalActive()) {
        ESP.restart();
        return;
    }

    if (weather_status == 0) {
        weather_stop();
    }

    // 设置配置页面
    // 根据配置信息设置默认值
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String qHost = pref.getString(PREF_QWEATHER_HOST);
    String qToken = pref.getString(PREF_QWEATHER_KEY);
    String qType = pref.getString(PREF_QWEATHER_TYPE, "0");
    String qLoc = pref.getString(PREF_QWEATHER_LOC);
    String cddLabel = pref.getString(PREF_CD_DAY_LABLE);
    String cddDate = pref.getString(PREF_CD_DAY_DATE);
    String tagDays = pref.getString(PREF_TAG_DAYS);
    String week1st = pref.getString(PREF_SI_WEEK_1ST, "0");
    String studySchedule = pref.getString(PREF_STUDY_SCHEDULE);
    pref.end();

    para_qweather_host.setValue(qHost.c_str(), 64);
    para_qweather_key.setValue(qToken.c_str(), 32);
    para_qweather_location.setValue(qLoc.c_str(), 64);
    para_qweather_type.setValue(qType.c_str(), 1);
    para_cd_day_label.setValue(cddLabel.c_str(), 16);
    para_cd_day_date.setValue(cddDate.c_str(), 8);
    para_tag_days.setValue(tagDays.c_str(), 30);
    para_si_week_1st.setValue(week1st.c_str(), 1);
    para_study_schedule.setValue(studySchedule.c_str(), 4000);

    wm.setTitle("J-Calendar");
    wm.addParameter(&para_si_week_1st);
    wm.addParameter(&para_qweather_host);
    wm.addParameter(&para_qweather_key);
    wm.addParameter(&para_qweather_type);
    wm.addParameter(&para_qweather_location);
    wm.addParameter(&para_cd_day_label);
    wm.addParameter(&para_cd_day_date);
    wm.addParameter(&para_tag_days);
    wm.addParameter(&para_study_schedule);
    // std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};
    std::vector<const char*> menu = { "wifi","param","update","sep","info","restart","exit" };
    wm.setMenu(menu); // custom menu, pass vector
    wm.setConfigPortalBlocking(false);
    wm.setBreakAfterConfig(true);
    wm.setPreSaveParamsCallback(preSaveParamsCallback);
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setSaveConnect(false); // 保存完wifi信息后是否自动连接，设置为否，以便于用户继续配置param。
    wm.startConfigPortal("J-Calendar", "password");

    led_config(); // LED 进入三快闪状态

    // 控制配置超时180秒后休眠
    _idle_millis = millis();
}

// 重置系统，并重启
void buttonLongPressStop(void* oneButton) {
    Serial.println("Button long press.");

    // 删除Preferences，namespace下所有健值对。
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.clear();
    pref.end();

    ESP.restart();
}

#define uS_TO_S_FACTOR 1000000
#define TIMEOUT_TO_SLEEP  10 // seconds
time_t blankTime = 0;
void go_sleep() {
    uint64_t p;
    // 根据配置情况来刷新，如果未配置qweather信息，则24小时刷新，否则每2小时刷新
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String _qweather_key = pref.getString(PREF_QWEATHER_KEY, "");
    pref.end();

    time_t now;
    time(&now);
    struct tm local;
    localtime_r(&now, &local);
    if (_qweather_key.length() == 0 || weather_type() == 0) { // 没有配置天气或者使用按日天气，则第二天刷新。
        // Sleep to next day
        int secondsToNextDay = (24 - local.tm_hour) * 3600 - local.tm_min * 60 - local.tm_sec;
        Serial.printf("Seconds to next day: %d seconds.\n", secondsToNextDay);
        p = (uint64_t)(secondsToNextDay);
        p = p < 0 ? 3600 * 24 : (p + 30); // 额外增加30秒，避免过早唤醒
    } else {
        // Sleep to next even hour.
        int secondsToNextHour = (60 - local.tm_min) * 60 - local.tm_sec;
        if ((local.tm_hour % 2) == 0) { // 如果是奇数点，则多睡1小时
            secondsToNextHour += 3600;
        }
        Serial.printf("Seconds to next even hour: %d seconds.\n", secondsToNextHour);
        p = (uint64_t)(secondsToNextHour);
        p = p < 0 ? 3600 : (p + 10); // 额外增加10秒，避免过早唤醒
    }

    esp_sleep_enable_timer_wakeup(p * (uint64_t)uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(KEY_M, LOW);

    // 省电考虑，关闭RTC外设和存储器
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // RTC IO, sensors and ULP, 注意：由于需要按键唤醒，所以不能关闭，否则会导致RTC_IO唤醒(ext0)失败
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF); // 
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);

    gpio_deep_sleep_hold_dis(); // 解除所有引脚的保持状态
    
    // 省电考虑，重置gpio，平均每针脚能省8ua。
    // gpio_reset_pin(PIN_LED_R); // 减小deep-sleep电流
    gpio_reset_pin(SPI_CS); // 减小deep-sleep电流
    gpio_reset_pin(SPI_DC); // 减小deep-sleep电流
    gpio_reset_pin(SPI_RST); // 减小deep-sleep电流
    gpio_reset_pin(SPI_BUSY); // 减小deep-sleep电流`
    gpio_reset_pin(SPI_MOSI); // 减小deep-sleep电流
    gpio_reset_pin(SPI_MISO); // 减小deep-sleep电流
    gpio_reset_pin(SPI_SCK); // 减小deep-sleep电流
    // gpio_reset_pin(PIN_ADC); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SDA); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SCL); // 减小deep-sleep电流

    delay(10);
    Serial.println("Deep sleep...");
    Serial.flush();
    esp_deep_sleep_start();
}