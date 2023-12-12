#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../component/MHZ14a/mhz14a.h" 

uart_config_t uartMHZ14a ={
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 0, 
    .source_clk = UART_SCLK_DEFAULT
};
// 
uint32_t co2_ppm=0; 

void app_main(void)
{
    ESP_LOGI("MHZ14A","hhhh");
    ESP_ERROR_CHECK_WITHOUT_ABORT(mhz14a_initUART(&uartMHZ14a));
    ESP_LOGI("MHZ14A","hhhh");
    while(1){
        ESP_ERROR_CHECK_WITHOUT_ABORT(mhz14a_getDataFromSensorViaUART(&co2_ppm));
        ESP_LOGI("MHZ14A", "CO2: %lu", co2_ppm);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}   
