#include "weather.h"

#include <_preference.h>

TaskHandle_t WEATHER_HANDLER;

String _qweather_host;
String _qweather_key;
String _qweather_loc;

int8_t _weather_status = -1;
int8_t _weather_type = -1;
Weather _weather_now = {};
DailyWeather dailyWeather = {};
DailyForecast _daily_forecast = {
    .weather = &dailyWeather,
    .length = 1
};

int8_t weather_type() {
    return _weather_type;
}

int8_t weather_status() {
    return _weather_status;
}
Weather* weather_data_now() {
    return &_weather_now;
}
DailyForecast* weather_data_daily() {
    return &_daily_forecast;
}

void task_weather(void* param) {
    Serial.println("[Task] get weather begin...");

    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    _weather_type = pref.getString(PREF_QWEATHER_TYPE).compareTo("1") == 0 ? 1 : 0;
    pref.end();

    Serial.printf("Weather Type: %d\n", _weather_type);

    API<> api;

    // 实时天气
    bool success;
    if (_weather_type == 0) {
        success = api.getForecastDaily(_daily_forecast, _qweather_host.c_str(), _qweather_key.c_str(), _qweather_loc.c_str());
    } else {
        success = api.getWeatherNow(_weather_now, _qweather_host.c_str(), _qweather_key.c_str(), _qweather_loc.c_str());
    }
    if (success) {
        _weather_status = 1;
    } else {
        _weather_status = 2;
    }

    Serial.println("[Task] get weather end...");
    WEATHER_HANDLER = NULL;
    vTaskDelete(NULL);
}

void weather_exec(int status) {
    _weather_status = status;
    if (status > 0) {
        return;
    }

    if (!WiFi.isConnected()) {
        _weather_status = 2;
        return;
    }

    // Preference 获取配置信息。
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    _qweather_host = pref.getString(PREF_QWEATHER_HOST, "api.qweather.com");
    _qweather_key = pref.getString(PREF_QWEATHER_KEY, "");
    _qweather_loc = pref.getString(PREF_QWEATHER_LOC, "");
    pref.end();

    if (_qweather_key.length() == 0 || _qweather_loc.length() == 0) {
        Serial.println("Qweather key/locationID invalid.");
        _weather_status = 3;
        return;
    }

    if (WEATHER_HANDLER != NULL) {
        vTaskDelete(WEATHER_HANDLER);
        WEATHER_HANDLER = NULL;
    }
    xTaskCreate(task_weather, "WeatherData", 1024 * 8, NULL, 2, &WEATHER_HANDLER);
}

void weather_stop() {
    if (WEATHER_HANDLER != NULL) {
        vTaskDelete(WEATHER_HANDLER);
        WEATHER_HANDLER = NULL;
    }
    _weather_status = 2;
}

