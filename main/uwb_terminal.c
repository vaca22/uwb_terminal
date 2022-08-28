#include <stdio.h>
#include "sdcard_services.h"
#include <sys/dirent.h>
#include <esp_log.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"





void app_main(void)
{
    sdcard_init();
    sdcard_search_filepath("/sdcard");

//    ESP_LOGE("gaga","%d",fileNum);
//
//    for(int k=0;k<fileNum;k++){
//        ESP_LOGE("dddd","%s",fileList[k]);
//    }
}
