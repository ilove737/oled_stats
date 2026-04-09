/**
 * OLED系统状态显示程序
 * 
 * 支持自定义显示项：sysinfo, text, http, cmd, time, fans
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <curl/curl.h>

#include "ssd1306.h"
#include "sysinfo.h"
#include "http.h"
#include "display_item.h"

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// 更新显示项数据
static void update_item(DisplayItem* item) {
    time_t now = time(NULL);
    item->last_update = now;
    
    switch (item->type) {
        case ITEM_SYSINFO: {
            SysInfo info;
            sysinfo_get(&info);
            
            // sysinfo需要特殊处理，多行显示
            int y = item->y;
            if (item->config.sysinfo.show_ip) {
                snprintf(item->cached_data, sizeof(item->cached_data), "IP: %s", info.ip);
                // 标记为多行数据
                item->cache_valid = 1;
            }
            break;
        }
        
        case ITEM_TEXT:
            strncpy(item->cached_data, item->config.text.text, sizeof(item->cached_data) - 1);
            item->cache_valid = 1;
            break;
            
        case ITEM_TIME: {
            time_t t = time(NULL);
            struct tm* tm = localtime(&t);
            strftime(item->cached_data, sizeof(item->cached_data), 
                     item->config.time.format, tm);
            item->cache_valid = 1;
            break;
        }
        
        case ITEM_FANS:
            if (item->config.fans.vmid[0]) {
                char fans[32];
                if (http_get_bili_fans(item->config.fans.vmid, fans, sizeof(fans)) == 0) {
                    snprintf(item->cached_data, sizeof(item->cached_data), "Fans: %s", fans);
                } else {
                    strncpy(item->cached_data, "Fans: N/A", sizeof(item->cached_data) - 1);
                }
            } else {
                strncpy(item->cached_data, "Fans: --", sizeof(item->cached_data) - 1);
            }
            item->cache_valid = 1;
            break;
            
        case ITEM_CMD: {
            FILE* fp = popen(item->config.cmd.command, "r");
            if (fp) {
                if (fgets(item->cached_data, sizeof(item->cached_data), fp)) {
                    // 移除换行符
                    item->cached_data[strcspn(item->cached_data, "\n")] = '\0';
                    item->cache_valid = 1;
                } else {
                    strncpy(item->cached_data, "N/A", sizeof(item->cached_data) - 1);
                    item->cache_valid = 1;
                }
                pclose(fp);
            } else {
                strncpy(item->cached_data, "Error", sizeof(item->cached_data) - 1);
                item->cache_valid = 1;
            }
            break;
        }
        
        case ITEM_HTTP:
            // HTTP请求需要单独实现
            strncpy(item->cached_data, "HTTP", sizeof(item->cached_data) - 1);
            item->cache_valid = 1;
            break;
            
        default:
            item->cache_valid = 0;
            break;
    }
}

// 渲染显示项到OLED
static void render_item(SSD1306* oled, DisplayItem* item) {
    if (!item->enabled || !item->cache_valid) return;
    
    int y = item->y;
    
    switch (item->type) {
        case ITEM_SYSINFO: {
            SysInfo info;
            sysinfo_get(&info);
            char buf[64];
            
            if (item->config.sysinfo.show_ip) {
                snprintf(buf, sizeof(buf), "IP: %s", info.ip);
                ssd1306_draw_string(oled, item->x, y, buf, 1);
                y += 10;
            }
            if (item->config.sysinfo.show_cpu) {
                snprintf(buf, sizeof(buf), "CPU: %.2f", info.cpu_load);
                ssd1306_draw_string(oled, item->x, y, buf, 1);
                y += 10;
            }
            if (item->config.sysinfo.show_mem) {
                sysinfo_format_memory(buf, sizeof(buf), info.mem_used, info.mem_total);
                ssd1306_draw_string(oled, item->x, y, buf, 1);
                y += 10;
            }
            if (item->config.sysinfo.show_disk) {
                sysinfo_format_disk(buf, sizeof(buf), info.disk_used, info.disk_total, info.disk_percent);
                ssd1306_draw_string(oled, item->x, y, buf, 1);
            }
            break;
        }
        
        default:
            ssd1306_draw_string(oled, item->x, item->y, item->cached_data, 1);
            break;
    }
}

// 渲染整个显示器
static void render_display(SSD1306* oled, DisplayConfig* disp, int global_refresh) {
    ssd1306_clear(oled);
    
    for (int i = 0; i < disp->item_count; i++) {
        DisplayItem* item = &disp->items[i];
        
        // 检查是否需要更新
        time_t now = time(NULL);
        int interval = item->refresh_interval > 0 ? item->refresh_interval : global_refresh;
        
        if (now - item->last_update >= interval || !item->cache_valid) {
            update_item(item);
        }
        
        render_item(oled, item);
    }
    
    ssd1306_display(oled);
}

// 打印配置信息
static void print_config(Config* cfg) {
    printf("OLED显示程序启动\n");
    printf("I2C设备: %s\n", cfg->i2c_dev);
    printf("刷新间隔: %d秒\n", cfg->global_refresh);
    printf("反转显示: %s\n", cfg->invert_display ? "是" : "否");
    
    for (int d = 0; d < 2; d++) {
        DisplayConfig* disp = &cfg->displays[d];
        if (!disp->enabled || disp->item_count == 0) continue;
        
        printf("\n显示器%d (0x%02X):\n", d, disp->i2c_addr);
        for (int i = 0; i < disp->item_count; i++) {
            DisplayItem* item = &disp->items[i];
            printf("  [%d] %s @ (%d,%d)\n", i, item_type_to_string(item->type), item->x, item->y);
        }
    }
}

int main(int argc, char* argv[]) {
    Config cfg;
    config_init(&cfg);
    
    // 加载配置
    int ret = config_load_args(&cfg, argc, argv);
    if (ret != 0) {
        return ret > 0 ? 0 : 1;
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 初始化curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // 初始化OLED
    SSD1306* oled[2] = {NULL, NULL};
    
    for (int d = 0; d < 2; d++) {
        DisplayConfig* disp = &cfg.displays[d];
        if (disp->enabled && disp->item_count > 0) {
            oled[d] = ssd1306_init(cfg.i2c_dev, disp->i2c_addr);
            if (oled[d] && cfg.invert_display) {
                ssd1306_invert(oled[d], 1);
            }
        }
    }
    
    print_config(&cfg);
    
    int loop_count = 0;
    
    while (running) {
        // 渲染每个显示器
        for (int d = 0; d < 2; d++) {
            if (oled[d] && cfg.displays[d].enabled) {
                render_display(oled[d], &cfg.displays[d], cfg.global_refresh);
            }
        }
        
        loop_count++;
        
        // 等待下一次刷新
        for (int i = 0; i < cfg.global_refresh && running; i++) {
            sleep(1);
        }
    }
    
    // 清理
    printf("\n正在退出...\n");
    
    for (int d = 0; d < 2; d++) {
        if (oled[d]) {
            ssd1306_clear(oled[d]);
            ssd1306_display(oled[d]);
            ssd1306_deinit(oled[d]);
        }
    }
    
    curl_global_cleanup();
    
    return 0;
}