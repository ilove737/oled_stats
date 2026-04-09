/**
 * 系统信息获取模块实现
 */

#include "sysinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>

int sysinfo_get_ip(char* buf, int len) {
    FILE* fp = popen("hostname -I 2>/dev/null | awk '{print $1}'", "r");
    if (!fp) {
        strncpy(buf, "N/A", len);
        return -1;
    }
    
    if (!fgets(buf, len, fp)) {
        strncpy(buf, "N/A", len);
        pclose(fp);
        return -1;
    }
    
    // 移除末尾的换行符
    buf[strcspn(buf, "\n")] = '\0';
    pclose(fp);
    return 0;
}

double sysinfo_get_cpu_load(void) {
    FILE* fp = fopen("/proc/loadavg", "r");
    if (!fp) return 0.0;
    
    double load1, load5, load15;
    if (fscanf(fp, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
        fclose(fp);
        return 0.0;
    }
    
    fclose(fp);
    return load1;
}

int sysinfo_get_memory(uint64_t* used, uint64_t* total) {
    FILE* fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;
    
    uint64_t mem_free = 0, mem_available = 0, mem_buffers = 0, mem_cached = 0;
    *total = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lu kB", (unsigned long*)total);
        } else if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %lu kB", (unsigned long*)&mem_free);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %lu kB", (unsigned long*)&mem_available);
        } else if (strncmp(line, "Buffers:", 8) == 0) {
            sscanf(line, "Buffers: %lu kB", (unsigned long*)&mem_buffers);
        } else if (strncmp(line, "Cached:", 7) == 0) {
            sscanf(line, "Cached: %lu kB", (unsigned long*)&mem_cached);
        }
    }
    
    fclose(fp);
    
    // 计算已使用内存
    if (mem_available > 0) {
        *used = *total - mem_available;
    } else {
        *used = *total - mem_free - mem_buffers - mem_cached;
    }
    
    return 0;
}

int sysinfo_get_disk(uint64_t* used, uint64_t* total, double* percent) {
    struct statvfs vfs;
    if (statvfs("/", &vfs) != 0) {
        return -1;
    }
    
    *total = (uint64_t)vfs.f_blocks * vfs.f_frsize / 1024;
    *used = (uint64_t)(vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize / 1024;
    *percent = (double)*used / *total * 100.0;
    
    return 0;
}

int sysinfo_get(SysInfo* info) {
    memset(info, 0, sizeof(SysInfo));
    
    sysinfo_get_ip(info->ip, sizeof(info->ip));
    info->cpu_load = sysinfo_get_cpu_load();
    sysinfo_get_memory(&info->mem_used, &info->mem_total);
    sysinfo_get_disk(&info->disk_used, &info->disk_total, &info->disk_percent);
    
    return 0;
}

void sysinfo_format_memory(char* buf, int len, uint64_t used, uint64_t total) {
    double used_mb = used / 1024.0;
    double total_mb = total / 1024.0;
    double percent = (double)used / total * 100.0;
    snprintf(buf, len, "M: %.0f/%.0fMB %.1f%%", used_mb, total_mb, percent);
}

void sysinfo_format_disk(char* buf, int len, uint64_t used, uint64_t total, double percent) {
    double used_gb = used / 1024.0 / 1024.0;
    double total_gb = total / 1024.0 / 1024.0;
    snprintf(buf, len, "D: %.1f/%.1fGB %.1f%%", used_gb, total_gb, percent);
}