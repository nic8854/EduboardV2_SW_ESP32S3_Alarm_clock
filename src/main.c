/********************************************************************************************* */
//    Eduboard2 ESP32-S3 Template with BSP
//    Author: Martin Burger
//    Juventus Technikerschule
//    Version: 1.0.0
//    
//    This is the ideal starting point for a new Project. BSP for most of the Eduboard2
//    Hardware is included under components/eduboard2.
//    Hardware support can be activated/deactivated in components/eduboard2/eduboard2_config.h
/********************************************************************************************* */
#include "eduboard2.h"
#include "memon.h"

#include "math.h"

#define TAG "ALARM_CLOCK"

#define UPDATETIME_MS 100

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} clockTime_t;

clockTime_t currentTime;

void timeTask(void* param) {
    currentTime.hours = 0;
    currentTime.minutes = 0;
    currentTime.seconds = 0;
    vTaskDelay(100);
    for(;;) {
        currentTime.seconds += rotary_encoder_get_rotation(true);
        if(currentTime.seconds >= 60) {
            currentTime.seconds = 0;
            currentTime.minutes++;
        }
        if(currentTime.minutes >= 60) {
            currentTime.minutes = 0;
            currentTime.hours++;
        }
        if(currentTime.hours >= 24) {
            currentTime.hours = 0;
        }
        ESP_LOGI(TAG, "current time = %d:%d:%d", currentTime.hours, currentTime.minutes, currentTime.seconds);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void displayTask(void* param) {
    uint16_t xpos = 175;
	uint16_t ypos = 180;
	char displayTime[24];
	uint16_t color = GREEN;
    for(;;) {
        sprintf((char*)displayTime, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
        lcdFillScreen(BLACK);
        lcdDrawRect(xpos-10, ypos+5, xpos+140, ypos-35, BLUE);
        lcdDrawString(fx32L, xpos, ypos, &displayTime[0], color);
        lcdUpdateVScreen();
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

typedef enum {
    SET_HOURS, 
    SET_MINUTES,
    SET_SECONDS
} timesetMode_t;

void buttonTask(void* param) {
    bool_t timeSetState = SET_HOURS;
    timesetMode_t timeSetMode = 0;
    int32_t rotationChange = 0;

    for(;;) {
        // task main loop
        if(rotary_encoder_button_get_state(true) == LONG_PRESSED) {
            timeSetState != timeSetState;
            if(timeSetState) {
                led_set(LED0);
            } else {
                led_reset(LED0);
            }
        }
        if(timeSetState) {
            switch(timeSetMode) {
                case SET_HOURS: {
                    if(rotary_encoder_button_get_state(true) == SHORT_PRESSED) {
                        timeSetMode = SET_MINUTES;
                        ESP_LOGI(TAG, "mode = SET_MINUTES");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(TRUE);
                        ESP_LOGI(TAG, "rotation change --> %d", rotationChange);
                        break;
                    }
                }
                case SET_MINUTES: {
                    if(rotary_encoder_button_get_state(true) == SHORT_PRESSED) {
                        timeSetMode = SET_SECONDS;
                        ESP_LOGI(TAG, "mode = SET_SECONDS");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(TRUE);
                        ESP_LOGI(TAG, "rotation change --> %d", rotationChange);
                        break;
                    }
                }
                case SET_SECONDS: {
                    if(rotary_encoder_button_get_state(true) == SHORT_PRESSED) {
                        timeSetMode = SET_HOURS;
                        ESP_LOGI(TAG, "mode = SET_HOURS");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(TRUE);
                        ESP_LOGI(TAG, "rotation change --> %d", rotationChange);
                        break;
                    }
                }
            }
        } else {
            timeSetMode = SET_HOURS;
        }
        led_toggle(LED7);
        // delay
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}


void app_main()
{
    //Initialize Eduboard2 BSP
    eduboard2_init();
    
    //Create timeTask
    xTaskCreate(timeTask, "testTask", 2*2048, NULL, 10, NULL);
    xTaskCreate(displayTask, "displayTask", 2*2048, NULL, 10, NULL);
    xTaskCreate(buttonTask, "buttonTask", 2*2048, NULL, 10, NULL);
    return;
}