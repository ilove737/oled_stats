/**
 * 显示项配置模块
 */

#ifndef DISPLAY_ITEM_H
#define DISPLAY_ITEM_H

#include <stdint.h>
#include <time.h>

// 显示项类型
typedef enum {
    ITEM_NONE = 0,
    ITEM_SYSINFO,
    ITEM_TEXT,
    ITEM_HTTP,
    ITEM_CMD,
    ITEM_TIME,
    ITEM_FANS,
} ItemType;

// 系统信息选项
typedef struct {
    int show_ip;
    int show_cpu;
    int show_mem;
    int show_disk;
} SysInfoConfig;

// 文本配置
typedef struct {
    char text[128];
} TextConfig;

// HTTP配置
typedef struct {
    char url[256];
    char json_field[64];
    char format[64];
} HttpConfig;

// 命令配置
typedef struct {
    char command[256];
} CmdConfig;

// 时间配置
typedef struct {
    char format[32];
} TimeConfig;

// 粉丝配置
typedef struct {
    char vmid[32];
} FansConfig;

// 显示项
typedef struct DisplayItem {
    char name[32];              // 项名称
    ItemType type;              // 类型
    int x, y;                   // 位置
    int refresh_interval;       // 刷新间隔(秒)，0=跟随全局
    time_t last_update;         // 上次更新时间
    int enabled;                // 是否启用
    
    // 缓存的显示数据
    char cached_data[256];
    int cache_valid;
    
    // 类型特定配置
    union {
        SysInfoConfig sysinfo;
        TextConfig text;
        HttpConfig http;
        CmdConfig cmd;
        TimeConfig time;
        FansConfig fans;
    } config;
} DisplayItem;

// 显示器配置
typedef struct {
    uint8_t i2c_addr;           // I2C地址
    int enabled;                // 是否启用
    DisplayItem items[8];       // 显示项
    int item_count;             // 显示项数量
    int refresh_interval;       // 默认刷新间隔
} DisplayConfig;

// 全局配置
typedef struct {
    char i2c_dev[64];           // I2C设备路径
    DisplayConfig displays[2];  // 两个显示器配置
    int global_refresh;         // 全局刷新间隔
    int invert_display;         // 是否反转显示
    int daemon_mode;            // 守护进程模式
} Config;

/**
 * 初始化默认配置
 */
void config_init(Config* cfg);

/**
 * 从配置文件加载
 * @param cfg 配置结构体
 * @param filename 配置文件路径，NULL使用默认路径
 * @return 0成功，-1失败
 */
int config_load_file(Config* cfg, const char* filename);

/**
 * 从命令行参数加载
 * @param cfg 配置结构体
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 0成功，继续运行；1显示帮助后退出；-1失败
 */
int config_load_args(Config* cfg, int argc, char* argv[]);

/**
 * 创建显示项
 */
DisplayItem* display_item_create(DisplayConfig* disp, ItemType type, const char* name);

/**
 * 设置系统信息项
 */
void display_item_set_sysinfo(DisplayItem* item, int show_ip, int show_cpu, int show_mem, int show_disk);

/**
 * 设置文本项
 */
void display_item_set_text(DisplayItem* item, const char* text);

/**
 * 设置HTTP项
 */
void display_item_set_http(DisplayItem* item, const char* url, const char* json_field);

/**
 * 设置命令项
 */
void display_item_set_cmd(DisplayItem* item, const char* command);

/**
 * 设置时间项
 */
void display_item_set_time(DisplayItem* item, const char* format);

/**
 * 设置粉丝项
 */
void display_item_set_fans(DisplayItem* item, const char* vmid);

/**
 * 解析类型字符串
 */
ItemType item_type_from_string(const char* str);

/**
 * 类型转字符串
 */
const char* item_type_to_string(ItemType type);

#endif // DISPLAY_ITEM_H
