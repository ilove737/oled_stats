/**
 * HTTP请求模块实现 - 使用libcurl
 */

#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// CURL写回调函数
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct {
        char* data;
        int len;
        int pos;
    }* buf = userp;
    
    if (buf->pos + realsize >= buf->len - 1) {
        realsize = buf->len - buf->pos - 1;
    }
    
    memcpy(buf->data + buf->pos, contents, realsize);
    buf->pos += realsize;
    buf->data[buf->pos] = '\0';
    
    return realsize;
}

int http_get(const char* url, char* buf, int len) {
    CURL* curl;
    CURLcode res;
    
    struct {
        char* data;
        int len;
        int pos;
    } write_buf = {buf, len, 0};
    
    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }
    
    // 设置请求头，模拟浏览器
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, 
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return -1;
    }
    
    return write_buf.pos;
}

// 简单的JSON解析，查找follower字段
static long parse_bili_fans(const char* json) {
    // 查找 "follower": 后面的数字
    const char* p = strstr(json, "\"follower\"");
    if (!p) return -1;
    
    p = strchr(p, ':');
    if (!p) return -1;
    p++;
    
    while (*p == ' ' || *p == '\t') p++;
    
    return atol(p);
}

int http_get_bili_fans(const char* vmid, char* buf, int len) {
    char url[256];
    char response[4096];
    
    snprintf(url, sizeof(url), 
             "https://api.bilibili.com/x/relation/stat?vmid=%s&jsonp=jsonp", vmid);
    
    int ret = http_get(url, response, sizeof(response));
    if (ret < 0) {
        strncpy(buf, "N/A", len);
        return -1;
    }
    
    long fans = parse_bili_fans(response);
    if (fans < 0) {
        strncpy(buf, "N/A", len);
        return -1;
    }
    
    snprintf(buf, len, "%ld", fans);
    return 0;
}

// 解析BTC价格
static double parse_btc_price(const char* json) {
    // 查找 "price_cny" 字段
    const char* p = strstr(json, "\"price_cny\"");
    if (!p) return -1;
    
    p = strchr(p, ':');
    if (!p) return -1;
    p++;
    
    while (*p == ' ' || *p == '\t' || *p == '"') p++;
    
    return atof(p);
}

int http_get_btc_price(char* buf, int len) {
    const char* url = "https://dncapi.bostonteapartyevent.com/api/v4/reducehalf/info?coincode=bitcoin&webp=1";
    char response[8192];
    
    int ret = http_get(url, response, sizeof(response));
    if (ret < 0) {
        strncpy(buf, "N/A", len);
        return -1;
    }
    
    double price = parse_btc_price(response);
    if (price <= 0) {
        strncpy(buf, "N/A", len);
        return -1;
    }
    
    snprintf(buf, len, "%.2f", price);
    return 0;
}
