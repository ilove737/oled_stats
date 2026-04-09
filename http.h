/**
 * HTTP请求模块
 */

#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>

/**
 * 获取B站粉丝数
 * @param vmid B站用户ID
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @return 0成功，-1失败
 */
int http_get_bili_fans(const char* vmid, char* buf, int len);

/**
 * 获取BTC价格 (人民币)
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @return 0成功，-1失败
 */
int http_get_btc_price(char* buf, int len);

/**
 * HTTP GET请求
 * @param url URL
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @return 实际读取的字节数，-1失败
 */
int http_get(const char* url, char* buf, int len);

#endif // HTTP_H
