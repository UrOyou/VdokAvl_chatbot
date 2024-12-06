#include <stdio.h>
#include <esp_log.h>
#include <cstring>
#include "tts_ws.h"


// 初始化============================================================================================
void init(KeyDATA* ketdata, const char* APPID, const char* APIKey, const char* APISecret, const char* Text) {
    ketdata->APPID = strdup(APPID);
    ketdata->APIKey = strdup(APIKey);
    ketdata->APISecret = strdup(APISecret);
    ketdata->Text = strdup(Text);

    ketdata->common_args.app_id = strdup(ketdata->APPID);

    ketdata->business_args.aue = strdup("raw");
    ketdata->business_args.auf = strdup("audio/L16;rate=16000");
    ketdata->business_args.vcn = strdup("xiaoyan");
    ketdata->business_args.tte = strdup("utf8");

    // 使用UTF-8编码
    size_t text_encoded_length = 4 * strlen(Text); // 假设最坏情况下每个字符编码后长度为4
    unsigned char* text_encoded = (unsigned char*)malloc(text_encoded_length);
    base64_encode((const unsigned char*)Text, strlen(Text), text_encoded, text_encoded_length);

    ketdata->data.status = 2;
    ketdata->data.text = (char*)malloc(text_encoded_length + 1);
    memcpy(ketdata->data.text, text_encoded, text_encoded_length);
    ketdata->data.text[text_encoded_length] = '\0';
    free(text_encoded);

    // 如果需要使用UTF-16LE编码，可以在这里添加相应的代码
}

// 主函数============================================================================================
void create_url() {
    // 示例API密钥和密钥
    const char* APISecret = "your_api_secret";
    const char* APIKey = "your_api_key";

    // 创建URL
    char* url = "wss://tts-api.xfyun.cn/v2/tts";

    // 生成RFC1123格式的时间戳
    time_t now = time(NULL);
    char* date = format_date_time(now);

    // 拼接字符串
    char* signature_origin = "host: ws-api.xfyun.cn\n";
    signature_origin = (char*)realloc(signature_origin, strlen(signature_origin) + strlen(date) + 6);
    strcat(signature_origin, "date: ");
    strcat(signature_origin, date);
    strcat(signature_origin, "\nGET /v2/tts HTTP/1.1");

    // 进行HMAC-SHA256加密
    size_t signature_len;
    unsigned char* signature_sha = hmac_sha256((const unsigned char*)APISecret, (const unsigned char*)signature_origin, strlen(signature_origin), &signature_len);

    // Base64编码
    char* signature_sha_b64 = base64_encode(signature_sha, signature_len);
    free(signature_sha);

    // 构造authorization字符串
    char* authorization_origin = "api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\"";
    size_t auth_origin_len = snprintf(NULL, 0, authorization_origin, APIKey, "hmac-sha256", "host date request-line", signature_sha_b64);
    char* authorization = (char*)malloc(auth_origin_len + 1);
    snprintf(authorization, auth_origin_len + 1, authorization_origin, APIKey, "hmac-sha256", "host date request-line", signature_sha_b64);

    // Base64编码authorization
    char* authorization_b64 = base64_encode((const unsigned char*)authorization, strlen(authorization));

    // 将请求的鉴权参数组合为字典并拼接成URL
    char* v = "authorization=%s&date=%s&host=ws-api.xfyun.cn";
    size_t v_len = snprintf(NULL, 0, v, authorization_b64, date, "ws-api.xfyun.cn");
    char* url_with_auth = (char*)malloc(strlen(url) + v_len + 2);
    snprintf(url_with_auth, strlen(url) + v_len + 2, "%s?%s", url, v);

    // 打印结果进行对比
    printf("date: %s\n", date);
    printf("v: %s\n", v);
    printf("WebSocket URL: %s\n", url_with_auth);

}

// 函数定义
char* format_date_time(time_t now) {
    struct tm* tm_info = gmtime(&now);
    char* date = (char*)malloc(30);
    strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
    return date;
}

unsigned char* hmac_sha256(const unsigned char* key, const unsigned char* data, size_t data_len, size_t* out_len) {
    unsigned char* digest = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    unsigned int len = 0;
    HMAC(EVP_sha256(), key, strlen((const char*)key), data, data_len, digest, &len);
    *out_len = len;
    return digest;
}

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *b64, *bmem;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, buffer, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, (char**)&buffer);
    BIO_set_close(b64, BIO_NOCLOSE);
    size_t out_len = strlen((char*)buffer);
    char* result = (char*)malloc(out_len + 1);
    strncpy(result, (char*)buffer, out_len);
    result[out_len] = '\0';
    BIO_free_all(b64);
    return result;
}

char* urlencode(const char* str) {
    // 实现URL编码的函数，这里省略具体实现？
    return strdup(str);
}
void on_message() {
    // 示例代码，实际中需要替换为实际的WebSocket消息和连接处理
    const char *message = "{\"code\":0,\"sid\":\"12345\",\"data\":{\"audio\":\"BASE64_ENCODED_AUDIO\",\"status\":2},\"message\":\"\"}";
    WebSocket ws; // 假设这是有效的WebSocket连接

    // 解析消息
    if (parse_message(message, &ws)) {
        // 根据消息处理逻辑
        // ...
    }

    return 0;
}

int parse_message(const char *message, WebSocket *ws) {
    cJSON *json = cJSON_Parse(message);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return 1; // 解析失败
    }

    // 获取code, sid, audio, status
    int code = cJSON_GetObjectItem(json, "code")->valueint;
    const char *sid = cJSON_GetObjectItem(json, "sid")->valuestring;
    cJSON *data = cJSON_GetObjectItem(json, "data");
    const char *encoded_audio = cJSON_GetObjectItem(data, "audio")->valuestring;
    int status = cJSON_GetObjectItem(data, "status")->valueint;
    const char *message_str = cJSON_GetObjectItem(json, "message")->valuestring;

    // 打印整个消息
    printf("Received message: %s\n", message);

    // 处理WebSocket关闭
    if (status == 2) {
        printf("ws is closed\n");
        close_websocket(ws);
        cJSON_Delete(json);
        return 0;
    }

    // 处理错误码
    if (code != 0) {
        printf("sid:%s call error:%s code is:%d\n", sid, message_str, code);
        cJSON_Delete(json);
        return 0;
    } else {
        // Base64解码audio数据
        int audio_len = base64_decode_expected_len(encoded_audio, strlen(encoded_audio));
        unsigned char *audio = (unsigned char *)malloc(audio_len + 1);
        base64_decode(encoded_audio, strlen(encoded_audio), (char *)audio);
        audio[audio_len] = '\0'; // 确保字符串以NULL结尾

        // 写入文件
        handle_audio((const char *)audio, audio_len);
        free(audio);
    }
    cJSON_Delete(json);
    return 0;
}

void handle_audio(const char *audio, size_t audio_len) {
    FILE *f = fopen("./demo.pcm", "ab");
    if (f != NULL) {
        fwrite(audio, 1, audio_len, f);
        fclose(f);
    } else {
        perror("Failed to open file");
    }
}

void close_websocket(WebSocket *ws) {
    // 关闭WebSocket连接的逻辑
    // ...
}

void on_error(WebSocket *ws, const char *error) {
    fprintf(stderr, "### error: %s\n", error);
    // 根据实际需要处理错误
}

void on_close(WebSocket *ws) {
    printf("### closed ###\n");
    // 根据实际需要处理关闭
}

void on_open(WebSocket *ws, WsParam *wsParam) {
    pthread_t thread_id;
    int result = pthread_create(&thread_id, NULL, run, (void *)wsParam);
    if (result != 0) {
        fprintf(stderr, "Error creating thread\n");
        return;
    }
    pthread_detach(thread_id); // 分离线程，自动回收资源
}

void *run(void *arg) {
    WsParam *wsParam = (WsParam *)arg;
    cJSON *d = cJSON_CreateObject();
    cJSON_AddItemToObject(d, "common", wsParam->CommonArgs);
    cJSON_AddItemToObject(d, "business", wsParam->BusinessArgs);
    cJSON_AddItemToObject(d, "data", wsParam->Data);
    char *d_str = cJSON_Print(d);
    printf("------>开始发送文本数据\n");
    // 假设ws有一个send函数
    // ws.send(d_str);
    free(d_str);
    cJSON_Delete(d);

    // 假设有一个函数来检查文件是否存在并删除
    // if (path_exists("./demo.pcm")) {
    //     remove_file("./demo.pcm");
    // }
    return NULL;
}

// 以下是辅助函数的示例实现
int path_exists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void remove_file(const char *path) {
    if (remove(path) != 0) {
        perror("Failed to remove file");
    }
}


void free_memo(void){
    //init释放内存
    free(ketdata.APPID);
    free(ketdata.APIKey);
    free(ketdata.APISecret);
    free(ketdata.Text);
    free(ketdata.common_args.app_id);
    free(ketdata.business_args.aue);
    free(ketdata.business_args.auf);
    free(ketdata.business_args.vcn);
    free(ketdata.business_args.tte);
    free(ketdata.data.text);

    //createurl释放内存
    free(date);
    free(signature_origin);
    free(signature_sha_b64);
    free(authorization);
    free(authorization_b64);
    free(url_with_auth);
}


int main() {

    // 示例代码，实际中需要替换为实际的WebSocket连接和参数处理
    WebSocket ws; // 假设这是有效的WebSocket连接
    WsParam wsParam; // 假设这是有效的参数
    // 注册事件处理函数
    // ...

    // 模拟WebSocket连接建立
    on_open(&ws, &wsParam);
    // 示例使用
    KeyDATA ketdata;
    init_KeyDATA(&ketdata, "your_APPID", "your_APIKey", "your_APISecret", "Hello, World!");

    return 0;
}

