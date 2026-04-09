/**
 * 系统信息获取模块
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdint.h>

// 系统信息结构体
typedef struct {
    char ip[64];          // IP地址
    double cpu_load;      // CPU负载 (1分钟平均)
    uint64_t mem_used;    // 已使用内存 (KB)
    uint64_t mem_total;   // 总内存 (KB)
    uint64_t disk_used;   // 已使用磁盘 (KB)
    uint64_t disk_total;  // 总磁盘 (KB)
    double disk_percent;  // 磁盘使用百分比
} SysInfo;

/**
 * 获取系统信息
 * @param info 系统信息结构体指针
 * @return 0成功，-1失败
 */
int sysinfo_get(SysInfo* info);

/**
 * 获取IP地址
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @return 0成功，-1失败
 */
int sysinfo_get_ip(char* buf, int len);

/**
 * 获取CPU负载
 * @return CPU负载 (0.0-1.0)
 */
double sysinfo_get_cpu_load(void);

/**
 * 获取内存信息
 * @param used 已使用内存 (KB)
 * @param total 总内存 (KB)
 * @return 0成功，-1失败
 */
int sysinfo_get_memory(uint64_t* used, uint64_t* total);

/**
 * 获取磁盘信息
 * @param used 已使用磁盘 (KB)
 * @param total 总磁盘 (KB)
 * @param percent 使用百分比
 * @return 0成功，-1失败
 */
int sysinfo_get_disk(uint64_t* used, uint64_t* total, double* percent);

/**
 * 格式化内存信息字符串
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @param used 已使用内存 (KB)
 * @param total 总内存 (KB)
 */
void sysinfo_format_memory(char* buf, int len, uint64_t used, uint64_t total);

/**
 * 格式化磁盘信息字符串
 * @param buf 缓冲区
 * @param len 缓冲区长度
 * @param used 已使用磁盘 (KB)
 * @param total 总磁盘 (KB)
 * @param percent 使用百分比
 */
void sysinfo_format_disk(char* buf, int len, uint64_t used, uint64_t total, double percent);

#endif // SYSINFO_H
