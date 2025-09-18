#include "led.h"

#include <Arduino.h>
#include "wiring.h"

TaskHandle_t LED_HANDLER;
int8_t BLINK_TYPE;

void led_init()
{
    pinMode(PIN_LED, OUTPUT);
}

void task_led(void *param)
{
    while(1)
    {
        switch(BLINK_TYPE)
        {
            case 0:
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(1000));
            break;
            case 1:
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(1000));
            break;
            case 2:
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(1000));
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(1000));
            break;
            case 3:
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(200));
            break;
            case 4:
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, LOW); // On
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(1000));
            break;
            default:
                digitalWrite(PIN_LED, HIGH); // Off
                vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void led_fast()
{
    BLINK_TYPE = 3;
    if (LED_HANDLER != NULL)
    {
        vTaskDelete(LED_HANDLER);
    }
    xTaskCreate(task_led, "TASK_LED", 2048, NULL, 5, &LED_HANDLER);
}

void led_slow()
{
    BLINK_TYPE = 2;
    if (LED_HANDLER != NULL)
    {
        vTaskDelete(LED_HANDLER);
    }
    xTaskCreate(task_led, "TASK_LED", 2048, NULL, 5, &LED_HANDLER);
}

void led_config()
{
    BLINK_TYPE = 4;
    if (LED_HANDLER != NULL)
    {
        vTaskDelete(LED_HANDLER);
    }
    xTaskCreate(task_led, "TASK_LED", 2048, NULL, 5, &LED_HANDLER);
}

void led_on()
{
    BLINK_TYPE = 1;
    if (LED_HANDLER != NULL)
    {
        vTaskDelete(LED_HANDLER);
    }
    xTaskCreate(task_led, "TASK_LED", 2048, NULL, 5, &LED_HANDLER);
}

void led_off()
{
    BLINK_TYPE = 0;
    if (LED_HANDLER != NULL)
    {
        vTaskDelete(LED_HANDLER);
    }
    xTaskCreate(task_led, "TASK_LED", 2048, NULL, 5, &LED_HANDLER);
}

