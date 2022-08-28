

#include "file_server.h"
#include "sdcard_services.h"

static TaskHandle_t file_server_task_h;
static const char *TAG = "file_server";

void preprocess_string(char *str) {
    char *p, *q;

    for (p = q = str; *p != 0; p++) {
        if (*(p) == '%' && *(p + 1) != 0 && *(p + 2) != 0) {
            // quoted hex
            uint8_t a;
            p++;
            if (*p <= '9')
                a = *p - '0';
            else
                a = toupper((unsigned char) *p) - 'A' + 10;
            a <<= 4;
            p++;
            if (*p <= '9')
                a += *p - '0';
            else
                a += toupper((unsigned char) *p) - 'A' + 10;
            *q++ = a;
        } else if (*(p) == '+') {
            *q++ = ' ';
        } else {
            *q++ = *p;
        }
    }
    *q = '\0';
}


static const char *get_path_from_uri(char *dest, const char *base_path, const char *uri2, size_t destsize) {
    char uri[260];
    strcpy(uri, uri2);
    preprocess_string(uri);
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        return NULL;
    }


    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    return dest + base_pathlen;
}



static int sendIndex=0;
#define send_mtu 8192
static char send_buf[send_mtu];

static esp_err_t download_get_handler(httpd_req_t *req) {
    int total=sdcard_get_file_size(fileList[sendIndex]);

    FILE *f =  sdcard_fopen(fileList[sendIndex]);

    sendIndex++;
    if(sendIndex>=fileNum){
        sendIndex=0;
    }


    int index=0;
    int send_len;
    do{
        send_len=fread(send_buf,1,send_mtu,f);
        if(send_len>0){
            if(httpd_resp_send_chunk(req, send_buf,send_mtu)!=ESP_OK){
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
                return ESP_FAIL;
            }
        }else{
            break;
        }
    } while (true);
    httpd_resp_sendstr_chunk(req, NULL);
    fclose(f);
    return ESP_OK;
}



static void file_server_task(void *pvParameters) {

    static struct file_server_data *server_data = NULL;

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
    }

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.uri_match_fn = httpd_uri_match_wildcard;


    if (httpd_start(&server, &config) != ESP_OK) {

    }

    httpd_uri_t file_download = {
            .uri       = "/*",
            .method    = HTTP_GET,
            .handler   = download_get_handler,
            .user_ctx  = server_data
    };
    httpd_register_uri_handler(server, &file_download);

    while (1){
        vTaskDelay(100);
    }
}




esp_err_t start_file_server() {
    xTaskCreatePinnedToCore(file_server_task, "file_server", 4096, NULL, configMAX_PRIORITIES, &file_server_task_h, 1);
    return ESP_OK;
}