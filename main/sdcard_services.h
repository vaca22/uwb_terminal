//
// Created by vaca on 8/27/22.
//

#ifndef UWB_TERMINAL_SDCARD_SERVICES_H
#define UWB_TERMINAL_SDCARD_SERVICES_H

extern char **fileList;
extern int fileNum;
void sdcard_init();
void sdcard_search_filepath(const char *path);
#endif //UWB_TERMINAL_SDCARD_SERVICES_H
