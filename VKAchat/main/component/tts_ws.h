#ifndef __TTS_WS__
#define __TTS_WS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include <time.h>

#include <stdint.h>
#include <stddef.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include <pthread.h>

#include <base64.h> // 需要一个base64编码的库，例如OpenSSL
//----------

// #include <openssl/hmac.h>
// #include <openssl/sha.h>
// #include <openssl/bio.h>
// #include <openssl/evp.h>
//----------
#include <cjson/cJSON.h>
#include <libb64/decode.h>


#define STATUS_FIRST_FRAME = 0  // 第一帧的标识
#define STATUS_CONTINUE_FRAME = 1  // 中间帧标识
#define STATUS_LAST_FRAME = 2  // 最后一帧的标识

void init(APPID,APIKey,APISecret,Text);
void create_url();
void on_message(ws,mesage);
void on_error(error);
void on_close(void);
void on_open(void);
void test_progress(void);

// 函数声明
char* format_date_time(time_t now);
unsigned char* hmac_sha256(const unsigned char* key, const unsigned char* data, size_t data_len, size_t* out_len);
char* base64_encode(const unsigned char* buffer, size_t length);
char* urlencode(const char* str);

// 函数声明
int parse_message(const char *message, WebSocket *ws);
void handle_audio(const char *audio, size_t audio_len);
void close_websocket(WebSocket *ws);

// 函数声明
void on_error(WebSocket *ws, const char *error);
void on_close(WebSocket *ws);
void on_open(WebSocket *ws, WsParam *wsParam);
void *run(void *arg);



//存储公共参数
typedef struct {
    char* app_id;
} CommonArgs;

//存储业务参数
typedef struct {
    char* aue;
    char* auf;
    char* vcn;
    char* tte;
} BusinessArgs;

//存储数据
typedef struct {
    int status;
    char* text;
} Data;

typedef struct {
    char* APPID;
    char* APIKey;
    char* APISecret;
    char* Text;
    CommonArgs common_args;
    BusinessArgs business_args;
    Data data;
} KeyDATA;

// 假设有一个结构体表示WebSocket连接
typedef struct {
    // WebSocket连接相关的字段
} WebSocket;

// 假设有参数结构体
typedef struct {
    cJSON *CommonArgs;
    cJSON *BusinessArgs;
    cJSON *Data;
} WsParam;



#endif