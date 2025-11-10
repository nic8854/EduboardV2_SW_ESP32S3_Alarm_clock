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

#define SET_TIME_BIT        (1 << 0)  // bit 0
#define SET_ALARM_BIT       (1 << 1)  // bit 1
#define ALARM_ACTIVE_BIT    (1 << 2)  // bit 2
#define SET_HOURS_BIT       (1 << 5)  // bit 5
#define SET_MINUTES_BIT     (1 << 6)  // bit 6
#define SET_SECONDS_BIT     (1 << 7)  // bit 7

EventGroupHandle_t alarmClockEventGroup;



void timeTask(void* param) {
    currentTime.hours = 0;
    currentTime.minutes = 0;
    currentTime.seconds = 0;
    vTaskDelay(100);
    for(;;) {
        EventBits_t currentBits = xEventGroupGetBits(alarmClockEventGroup);
        if((currentBits & SET_TIME_BIT) == 0) {
            currentTime.seconds++;
        }
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

void inputTask(void* param) {
    bool timeSetState = SET_HOURS;
    timesetMode_t timeSetMode = 0;
    int32_t rotationChange = 0;

    for(;;) {
        // task main loop
        if(rotary_encoder_button_get_state(false) == LONG_PRESSED) {
            timeSetState = !timeSetState;
            if(timeSetState) {
                xEventGroupSetBits(alarmClockEventGroup, SET_TIME_BIT);
                led_set(LED0, 1);
            } else {
                xEventGroupClearBits(alarmClockEventGroup, SET_TIME_BIT);
                led_set(LED0, 0);
            }
        }
        if(timeSetState) {
            switch(timeSetMode) {
                case SET_HOURS: {
                    if(rotary_encoder_button_get_state(false) == SHORT_PRESSED) {
                        timeSetMode = SET_MINUTES;
                        ESP_LOGI(TAG, "mode = SET_MINUTES");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(true);
                        ESP_LOGI(TAG, "SET_HOURS change --> %d", rotationChange);
                        break;
                    }
                }
                case SET_MINUTES: {
                    if(rotary_encoder_button_get_state(false) == SHORT_PRESSED) {
                        timeSetMode = SET_SECONDS;
                        ESP_LOGI(TAG, "mode = SET_SECONDS");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(true);
                        ESP_LOGI(TAG, "SET_MINUTES change --> %d", rotationChange);
                        break;
                    }
                }
                case SET_SECONDS: {
                    if(rotary_encoder_button_get_state(false) == SHORT_PRESSED) {
                        timeSetMode = SET_HOURS;
                        ESP_LOGI(TAG, "mode = SET_HOURS");
                        rotationChange = 0;
                        break;
                    } else {
                        rotationChange += rotary_encoder_get_rotation(true);
                        ESP_LOGI(TAG, "SET_SECONDS change --> %d", rotationChange);
                        break;
                    }
                }
            }
        } else {
            timeSetMode = SET_HOURS;
        }
        rotary_encoder_button_get_state(true);
        // delay
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}


void app_main()
{
    //Initialize Eduboard2 BSP
    eduboard2_init();

    alarmClockEventGroup = xEventGroupCreate();
    
    //Create timeTask
    xTaskCreate(timeTask, "timeTask", 2*2048, NULL, 10, NULL);
    xTaskCreate(displayTask, "displayTask", 2*2048, NULL, 10, NULL);
    xTaskCreate(inputTask, "inputTask", 2*2048, NULL, 10, NULL);
    return;
}
