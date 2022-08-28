

#include "file_server.h"
#include "sdcard_services.h"

static TaskHandle_t file_server_task_h;
static const char *TAG = "file_server";
#define SCRATCH_BUFSIZE  2048
#define FILE_PATH_MAX (256)
struct file_server_data {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
};

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





static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath) {


        char entrypath[FILE_PATH_MAX];


        struct dirent *entry;
        struct stat entry_stat;

        DIR *dir = opendir(dirpath);
        const size_t dirpath_len = strlen(dirpath);

        strlcpy(entrypath, dirpath, sizeof(entrypath));

        if (!dir) {
            ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
            return ESP_FAIL;
        }
        cJSON *files = cJSON_CreateArray();
        while ((entry = readdir(dir)) != NULL) {

            if (entry->d_type == DT_DIR) {
                continue;
            }
            strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
            if (stat(entrypath, &entry_stat) == -1) {
                ESP_LOGE(TAG, "Failed to stat: %s",  entry->d_name);
                continue;
            }

            cJSON *item = cJSON_CreateObject();
            cJSON_AddStringToObject(item,"name",entry->d_name);
            cJSON_AddNumberToObject(item,"size",entry_stat.st_size);
            cJSON_AddItemToArray(files, item);
        }

        char * n=cJSON_Print(files);
        httpd_resp_sendstr_chunk(req, n);
        httpd_resp_sendstr_chunk(req, NULL);
        cJSON_Delete(files);
        free(n);
        free(dir);

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}































static int sendIndex=0;
#define send_mtu 8192
static char send_buf[send_mtu];


static esp_err_t download_get_handler(httpd_req_t *req) {
    char filepath[FILE_PATH_MAX];
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *) req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }
    if (filename[strlen(filename) - 1] == '/') {
        return http_resp_dir_html(req, filepath);
    }



    FILE *f = fopen(filepath,"rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }


    httpd_resp_set_type(req, "image/jpeg");
    int index=0;
    int send_len;
    do{
        send_len=fread(send_buf,1,send_mtu,f);
        if(send_len>0){
            if(httpd_resp_send_chunk(req, send_buf,send_len)!=ESP_OK){
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

    const char *base_path = "/sdcard";
    static struct file_server_data *server_data = NULL;


    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
    }

    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

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
