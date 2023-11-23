#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void)
{
ESP_LOGW("LOG", "This is a warning");
ESP_LOGI("LOG", "This is an info");
ESP_LOGD("LOG", "This is a debug");
ESP_LOGV("LOG", "This is a verbose");
ESP_LOGE("LOG","This is an error");
}   
