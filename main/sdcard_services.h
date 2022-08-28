//
// Created by vaca on 8/27/22.
//



#ifndef UWB_TERMINAL_SDCARD_SERVICES_H
#define UWB_TERMINAL_SDCARD_SERVICES_H
#include <stdio.h>


//extern char **fileList;
//extern int fileNum;
void sdcard_init();
void sdcard_search_filepath(const char *path);
int sdcard_get_file_size(char* path);
FILE* sdcard_fopen(char *my_path);
#endif //UWB_TERMINAL_SDCARD_SERVICES_H
