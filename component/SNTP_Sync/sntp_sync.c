#include "sntp_sync.h"


void sntp_init_func()
{
    ESP_LOGI(__func__, "Initializing SNTP.");   
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL); // Thiết lập chế độ hoạt động của SNTP là "polling" để tự động đồng bộ thời gian.
    esp_sntp_setservername(0, "pool.ntp.org"); // Thiết lập địa chỉ máy chủ NTP mà thiết bị sẽ truy vấn.
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED); // Thiết lập chế độ đồng bộ ngay lập tức.
    esp_sntp_init(); // Khởi tạo SNTP
}

esp_err_t sntp_setTime(struct tm *timeInfo, time_t *timeNow, char *timeString)
{
    for (size_t i = 0; (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (i < RETRY_SET_TIME_SYSTEM); i++) // Kiểm tra cho đến khi hệ thống đã đồng bộ thời gian 
    {
        ESP_LOGI(__func__, "Waiting for system time to be set...");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    time(timeNow); // Lấy giá trị thời gian hiện tại và lưu vào biến timeNow
    
    //localtime_r(timeNow, timeInfo); // Chuyển đổi thời gian epoch (số giây kể từ 00:00:00 UTC, 1 tháng 1 năm 1970) thành cấu trúc struct tm để có thể truy xuất thông tin chi tiết về thời gian.

    setenv("TZ", "GMT-07", 1);  // Thiết lập múi giờ Việt Nam
    tzset(); // Cập nhật thông tin múi giờ
    localtime_r(timeNow, timeInfo); // Chuyển đổi thời gian epoch (số giây kể từ 00:00:00 UTC, 1 tháng 1 năm 1970) thành cấu trúc struct tm để có thể truy xuất thông tin chi tiết về thời gian.

    strftime(timeString,30, "%c", timeInfo); // Chuyển đổi thời gian từ cấu trúc struct tm thành chuỗi ký tự theo định dạng "Day Month Year 00:00:00 1970". Chuỗi kết quả được lưu vào timeString.
    // ESP_LOGI(__func__, "The current date/time in Viet Nam is: %s ", timeString);
    return ESP_OK;
}
