

#include "holiday.h"

#include "Arduino.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 获取当月假期信息。数组中数字为当月日期，正为休假，负为补工作日
// https://timor.tech/api/holiday/
bool getHolidays(Holiday& result, int year, int month) {
    String req = "https://timor.tech/api/holiday/year/" + String(year) + "-" + String(month);

    HTTPClient http;
    http.setTimeout(10 * 1000);
    http.begin(req);
    Serial.printf("Request: %s\n", req.c_str());
    int httpCode = http.GET();
    if (httpCode != 200) {
        http.end();
        Serial.println(HTTPClient::errorToString(httpCode));
        Serial.println("假日数据获取失败");
        return false;
    }
    
    String resp = http.getString();
    // Serial.printf("Response: %s\n", resp.c_str());
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, resp);
    if (error) {
        http.end();
        Serial.print(F("Parse holiday data failed: "));
        Serial.println(error.f_str());
        return false;
    }
    http.end();

    int status = doc["code"].as<int>();
    if (status != 0) {
        Serial.println("Get holidays error.");
        return false;
    }
    result.year = year;
    result.month = month;
    JsonObject oHoliday = doc["holiday"].as<JsonObject>();
    int i = 0;
    Serial.printf("Holiday: ");
    for (JsonPair kv : oHoliday) {
        String key = String(kv.key().c_str());
        JsonObject value = kv.value();
        bool isHoliday = value["holiday"].as<bool>();

        int day = key.substring(3, 5).toInt();
        result.holidays[i] = day * (isHoliday ? 1 : -1); // 假期为正，补工作日为负
        Serial.printf("%d ", result.holidays[i]);
        i++;
        if (i >= 50) break;
    }
    Serial.println();
    result.length = i;
    return true;
}
