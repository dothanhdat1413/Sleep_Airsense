#include <stdio.h>
// #include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "../component/MHZ14a/mhz14a.h" 

/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "sntp_sync.h"
#include "sdcard.h"

//-------------------------MHZ14A------------------------------
uart_config_t uartMHZ14a ={
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 0, 
    .source_clk = UART_SCLK_DEFAULT
};
uint32_t co2_ppm=0; 
//-------------------------WIFI------------------------------
#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK                  // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.   
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK       // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.   
#define EXAMPLE_H2E_IDENTIFIER ""                          // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.   
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT              // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.   
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT     // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.     
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID       // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.       
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH                         // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.           
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH                // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.               
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID       // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.       
#endif                                                     // Cấu hình chế độ WPA3 SAE (Simultaneous Authentication of Equals) dựa trên cấu hình hệ thống.           

#if CONFIG_ESP_WIFI_AUTH_OPEN                                      //Chọn phương thức xác thực dựa trên cấu hình hệ thống.
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN           //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                    
#elif CONFIG_ESP_WIFI_AUTH_WEP                                     //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                    
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP            //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                        
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK                                 //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                        
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK        //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                                
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK                                //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                    
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK       //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                                
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK                            //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK   //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                            
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK                                //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                            
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK       //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                    
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK                           //Chọn phương thức xác thực dựa trên cấu hình hệ thống.        
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK  //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                            
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK                                //Chọn phương thức xác thực dựa trên cấu hình hệ thống.            
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK       //Chọn phương thức xác thực dựa trên cấu hình hệ thống.                        
#endif                                                             //Chọn phương thức xác thực dựa trên cấu hình hệ thống.        

static EventGroupHandle_t s_wifi_event_group; // Biến kiểu EventGroupHandle_t để theo dõi sự kiện kết nối wifi
#define WIFI_CONNECTED_BIT BIT0     // Bit đánh dấu sự kiện kết nối thành công
#define WIFI_FAIL_BIT      BIT1     // Bit đánh dấu sự kiện kết nối thất bại
static const char *TAG = "wifi station"; // tag để log

static int s_retry_num = 0; // biến đếm số lần thử connect lại wifi

/**
 * @brief Hàm xử lý sự kiện kết nối Wifi
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) { // Sự kiện bắt đầu kết nối wifi     
        esp_wifi_connect();                                             // Sự kiện bắt đầu kết nối wifi                
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {   // Sự kiện bị ngắt kết nối wifi
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {                                   // Sự kiện bị ngắt kết nối wifi            
            esp_wifi_connect();                                                         // Sự kiện bị ngắt kết nối wifi        
            s_retry_num++;                                                              // Sự kiện bị ngắt kết nối wifi            
            ESP_LOGI(TAG, "retry to connect to the AP");                                // Sự kiện bị ngắt kết nối wifi khi kết nối bị ngắt, ESP32 thử kết nối lại (nếu chưa vượt quá số lần thử) hoặc đặt cờ WIFI_FAIL_BIT để báo kết nối thất bại.                
        } else {                                                                        // Sự kiện bị ngắt kết nối wifi khi kết nối bị ngắt, ESP32 thử kết nối lại (nếu chưa vượt quá số lần thử) hoặc đặt cờ WIFI_FAIL_BIT để báo kết nối thất bại.                
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);                      // Sự kiện bị ngắt kết nối wifi khi kết nối bị ngắt, ESP32 thử kết nối lại (nếu chưa vượt quá số lần thử) hoặc đặt cờ WIFI_FAIL_BIT để báo kết nối thất bại.                                            
        }                                                                               // Sự kiện bị ngắt kết nối wifi khi kết nối bị ngắt, ESP32 thử kết nối lại (nếu chưa vượt quá số lần thử) hoặc đặt cờ WIFI_FAIL_BIT để báo kết nối thất bại.        
        ESP_LOGI(TAG,"connect to the AP fail");                                         // Sự kiện bị ngắt kết nối wifi khi kết nối bị ngắt, ESP32 thử kết nối lại (nếu chưa vượt quá số lần thử) hoặc đặt cờ WIFI_FAIL_BIT để báo kết nối thất bại.                
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {  // Sự kiện nhận được địa chỉ IP, đặt biến đếm số lần thử connect lại =0, đặt cờ WIFI_CONNECTED_BIT để báo hiệu kết nối thành công.                                    
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;          // Sự kiện nhận được địa chỉ IP, đặt biến đếm số lần thử connect lại =0, đặt cờ WIFI_CONNECTED_BIT để báo hiệu kết nối thành công.           
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));          // Sự kiện nhận được địa chỉ IP, đặt biến đếm số lần thử connect lại =0, đặt cờ WIFI_CONNECTED_BIT để báo hiệu kết nối thành công.               
        s_retry_num = 0;                                                     // Sự kiện nhận được địa chỉ IP, đặt biến đếm số lần thử connect lại =0, đặt cờ WIFI_CONNECTED_BIT để báo hiệu kết nối thành công.               
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);          // Sự kiện nhận được địa chỉ IP, đặt biến đếm số lần thử connect lại =0, đặt cờ WIFI_CONNECTED_BIT để báo hiệu kết nối thành công.                   
    }           
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate(); // Tạo một nhóm sự kiện FreeRTOS để theo dõi trạng thái kết nối Wi-Fi.

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init()); //Khởi tạo thành phần mạng để cấu hình kết nối Wi-Fi.

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default()); // Tạo vòng lặp sự kiện mặc định để xử lý các sự kiện Wi-Fi và IP.
    esp_netif_create_default_wifi_sta(); // Tạo thành phần mạng Wi-Fi cho chế độ Station.

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  //Khởi tạo cấu hình Wi-Fi với các giá trị mặc định.          
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&cfg));                 //Khởi tạo cấu hình Wi-Fi với các giá trị mặc định.      

    esp_event_handler_instance_t instance_any_id;                            //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.          
    esp_event_handler_instance_t instance_got_ip;                            //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.             
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_register(WIFI_EVENT,          //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                                
                                                        ESP_EVENT_ANY_ID,    //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                                
                                                        &event_handler,      //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                               
                                                        NULL,                //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                                 
                                                        &instance_any_id));  //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                                                     
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_register(IP_EVENT,            //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                                             
                                                        IP_EVENT_STA_GOT_IP, //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                             
                                                        &event_handler,      //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                
                                                        NULL,                //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                    
                                                        &instance_got_ip));  //Đăng ký trình xử lý sự kiện cho các sự kiện Wi-Fi và IP để xử lý các sự kiện liên quan đến kết nối Wi-Fi.                                      

    wifi_config_t wifi_config = {                                       // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                                                        
        .sta = {                                                        // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                
            .ssid = CONFIG_ESP_WIFI_SSID,                               // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                                                                                                            
            .password = CONFIG_ESP_WIFI_PASSWORD,                       // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                                 
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,    // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                     
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,                           // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                                                 
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,               // Thiết lập cấu hình wifi bằng SSID và Password từ menuconfig                                                                          
        },
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA) );                 // Thiết lập chế độ và cấu hình Wi-Fi, sau đó khởi động Wi-Fi để bắt đầu quá trình kết nối.            
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );   // Thiết lập chế độ và cấu hình Wi-Fi, sau đó khởi động Wi-Fi để bắt đầu quá trình kết nối.            
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start() );                                 // Thiết lập chế độ và cấu hình Wi-Fi, sau đó khởi động Wi-Fi để bắt đầu quá trình kết nối.                                
    ESP_LOGI(TAG, "wifi_init_sta finished.");                           // Thiết lập chế độ và cấu hình Wi-Fi, sau đó khởi động Wi-Fi để bắt đầu quá trình kết nối.        

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,  // Sử dụng xEventGroupWaitBits để đợi cho đến khi kết nối được thiết lập hoặc kết nối thất bại sau một số lần thử lại.                
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,                 // Sử dụng xEventGroupWaitBits để đợi cho đến khi kết nối được thiết lập hoặc kết nối thất bại sau một số lần thử lại.            
            pdFALSE,                                            // Sử dụng xEventGroupWaitBits để đợi cho đến khi kết nối được thiết lập hoặc kết nối thất bại sau một số lần thử lại.                                    
            pdFALSE,                                            // Sử dụng xEventGroupWaitBits để đợi cho đến khi kết nối được thiết lập hoặc kết nối thất bại sau một số lần thử lại.            
            portMAX_DELAY);                                     // Sử dụng xEventGroupWaitBits để đợi cho đến khi kết nối được thiết lập hoặc kết nối thất bại sau một số lần thử lại.                

    if (bits & WIFI_CONNECTED_BIT) {                               // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.         
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",       // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.                         
                CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);   // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.                                 
    } else if (bits & WIFI_FAIL_BIT) {                             // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.                 
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.             
                CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);   // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.                                             
    } else {                                                       // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.         
        ESP_LOGE(TAG, "UNEXPECTED EVENT");                         // Dựa vào các bit được đặt trong nhóm sự kiện, ghi log thông báo tương ứng về kết nối thành công hoặc thất bại.                     
    }
}


time_t timeNow = 0;
struct tm timeInfo = {0};
char timeString[30];
char fileName[30];
/**
 * @brief Sửa biến lưu thời gian phù hợp với định dạng tên file
 * 
 */
void timestamp_toFileName(){
	int i=0;
	for(i=0;i<30;i++){
		if((timeString[i]==' ')||(timeString[i]==':')||(timeString[i]=='\0')) fileName[i]='o';
        else fileName[i]=timeString[i]; // kí tự không bên cạnh 1 số
        // if(i==29) fileName[i]='t';
	}
}

void app_main(void)
{   
    //_________________________Initialize NVS, Wifi
    esp_err_t ret = nvs_flash_init();    // khởi tạo 1 vùng nhớ không bay hơi -NVS
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {  // Nếu việc khởi tạo gặp lỗi vì không còn trang trống hoặc có phiên bản mới, mã lỗi ESP_ERR_NVS_NO_FREE_PAGES hoặc ESP_ERR_NVS_NEW_VERSION_FOUND sẽ được trả về.
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase()); // khi lỗi, hàm nvs_flash_erase sẽ chọn xoá dữ liệu NVS, sau đó khởi tạo lại.
    ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    sntp_init_func();
    //Done initialize NVS, Wifi
    //_________________________Initialize MHZ14a_UART
    ESP_LOGI("MHZ14A","init MHZ14A");
    ESP_ERROR_CHECK_WITHOUT_ABORT(mhz14a_initUART(&uartMHZ14a));
    // Done initialize MHZ14a_UART

    // _______________________Initialize SD card

    ESP_LOGI(__func__, "Initialize SD card with SPI interface.");       //Config SPI    
    esp_vfs_fat_mount_config_t mount_config_t = MOUNT_CONFIG_DEFAULT(); //Config SPI            
    spi_bus_config_t spi_bus_config_t = SPI_BUS_CONFIG_DEFAULT();       //Config SPI            
    sdmmc_host_t host_t = SDSPI_HOST_DEFAULT();                         //Config SPI                    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();  //Config SPI                
    slot_config.gpio_cs = CONFIG_PIN_NUM_CS;                            //Config SPI            
    slot_config.host_id = host_t.slot;                                  //Config SPI        

    sdmmc_card_t SDCARD;
    ESP_ERROR_CHECK_WITHOUT_ABORT(sdcard_initialize(&mount_config_t, &SDCARD, &host_t, &spi_bus_config_t, &slot_config));
    // _______________________Done Initialized SD card


    while(1){
        // ESP_LOGI("MHZ14A","Start take time");
        // sntp_setTime(&timeInfo,&timeNow,&timeString);
        // ESP_LOGI("MHZ14A","Take time done");
        

        // ESP_ERROR_CHECK_WITHOUT_ABORT(mhz14a_getDataFromSensorViaUART(&co2_ppm));
        // ESP_LOGI("MHZ14A","Get data done");
        // ESP_LOGI("MHZ14A","CO2 at %s: %ld ppm",timeString, co2_ppm);
        // timestamp_toFileName();
        // ESP_LOGI("Test","File name: %s", fileName);
        // ESP_ERROR_CHECK_WITHOUT_ABORT(sdcard_writeDataToFile(fileName,"%ld", co2_ppm));
        
        ESP_ERROR_CHECK_WITHOUT_ABORT(sdcard_writeDataToFile("Sat","%ld", co2_ppm));
         ESP_ERROR_CHECK_WITHOUT_ABORT(sdcard_writeDataToFile("Sat123","%ld", co2_ppm));
          ESP_ERROR_CHECK_WITHOUT_ABORT(sdcard_writeDataToFile("Satnnnnnnnnnnnnnnnnnnnn","%ld", co2_ppm));

                

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

