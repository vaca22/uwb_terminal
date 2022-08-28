#include <stdio.h>
#include "sdcard_services.h"
#include <sys/dirent.h>
#include <esp_log.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "file_server.h"
#include "wifi_services.h"


void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();
    sdcard_init();
    sdcard_search_filepath("/sdcard");

    start_file_server();

//    ESP_LOGE("gaga","%d",fileNum);
//
//    for(int k=0;k<fileNum;k++){
//        ESP_LOGE("dddd","%d", sdcard_get_file_size(fileList[k]));
//    }
}
