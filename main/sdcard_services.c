//
// Created by vaca on 8/27/22.
//

#include "sdcard_services.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"


static const char *TAG = "sdcard";

#define MOUNT_POINT "/sdcard"


#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13


#define SPI_DMA_CHAN    1

char **fileList=NULL;
int fileNum=0;

void sdcard_init(){
    esp_err_t ret;


    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
            .mosi_io_num = PIN_NUM_MOSI,
            .miso_io_num = PIN_NUM_MISO,
            .sclk_io_num = PIN_NUM_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");


    sdmmc_card_print_info(stdout, card);

}


static void clean_file_list(){
    if(fileList){
        if(fileNum>0){
            for(int k=0;k<fileNum;k++){
                free(fileList[k]);
            }
        }
        free(fileList);
    }
    fileList=NULL;
    fileNum=0;
}


static void sdcard_search_fileNum(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            fileNum++;
        }
    }
    fileList= (char **)malloc(fileNum*sizeof(char*));
    closedir(dir);
}

void sdcard_search_filepath(const char *path) {
    clean_file_list();
    sdcard_search_fileNum(path);
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (!dir) {
        return;
    }
    int index=0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            fileList[index]= malloc(strlen(entry->d_name)+1);
            memcpy(fileList[index],entry->d_name,strlen(entry->d_name)+1);
            index++;
        }
    }
    closedir(dir);
}




int sdcard_get_file_size(char* my_path){
    char path[50]="/sdcard/";
    memcpy(path+ strlen(path),my_path, strlen(my_path)+1);
    struct stat entry_stat;
    if (stat(path, &entry_stat) == -1) {
        return -1;
    }
    ESP_LOGI("file", "  %ld", entry_stat.st_size);
    return entry_stat.st_size;
}


FILE* sdcard_fopen(char *my_path){
    char path[50]="/sdcard/";
    memcpy(path+ strlen(path),my_path, strlen(my_path)+1);
    return fopen(path, "rb");
}