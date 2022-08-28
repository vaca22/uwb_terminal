#include <stdio.h>
#include "sdcard_services.h"
#include <sys/dirent.h>
#include <esp_log.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"


void sdcard_search_filepath(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            char *x=entry->d_name;
            ESP_LOGE("gaga","%s",x);
        }
    }
    closedir(dir);
}


void app_main(void)
{
    sdcard_init();
    sdcard_search_filepath("/sdcard");
    ESP_LOGE("gaga","gaga");
    sdcard_search_filepath("/sdcard");
}
