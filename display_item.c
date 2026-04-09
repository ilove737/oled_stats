/**
 * 显示项配置模块实现
 */

#include "display_item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#define DEFAULT_CONFIG_FILE "/etc/oled_stats.conf"

void config_init(Config* cfg) {
    memset(cfg, 0, sizeof(Config));
    
    strncpy(cfg->i2c_dev, "/dev/i2c-1", sizeof(cfg->i2c_dev) - 1);
    cfg->global_refresh = 25;
    cfg->invert_display = 0;
    cfg->daemon_mode = 0;
    
    // 显示器0配置
    cfg->displays[0].i2c_addr = 0x3C;
    cfg->displays[0].enabled = 1;
    cfg->displays[0].refresh_interval = 25;
    cfg->displays[0].item_count = 0;
    
    // 显示器1配置
    cfg->displays[1].i2c_addr = 0x3D;
    cfg->displays[1].enabled = 1;
    cfg->displays[1].refresh_interval = 25;
    cfg->displays[1].item_count = 0;
}

static char* trim(char* str) {
    while (isspace(*str)) str++;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) *end-- = '\0';
    return str;
}

static char* parse_value(char* line) {
    char* eq = strchr(line, '=');
    if (!eq) return NULL;
    return trim(eq + 1);
}

int config_load_file(Config* cfg, const char* filename) {
    if (!filename) filename = DEFAULT_CONFIG_FILE;
    
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    
    char line[256];
    int current_display = -1;
    int current_item = -1;
    
    while (fgets(line, sizeof(line), fp)) {
        char* s = trim(line);
        if (*s == '\0' || *s == '#' || *s == ';') continue;
        
        // 段落标记
        if (*s == '[') {
            char* end = strchr(s, ']');
            if (!end) continue;
            *end = '\0';
            s++;
            
            if (strncmp(s, "display:", 8) == 0) {
                current_display = atoi(s + 8);
                current_item = -1;
                if (current_display >= 0 && current_display < 2) {
                    cfg->displays[current_display].enabled = 1;
                }
            } else if (strncmp(s, "item:", 5) == 0) {
                // 查找或创建item
                char* name = s + 5;
                if (current_display >= 0 && current_display < 2) {
                    DisplayConfig* disp = &cfg->displays[current_display];
                    for (int i = 0; i < disp->item_count; i++) {
                        if (strcmp(disp->items[i].name, name) == 0) {
                            current_item = i;
                            break;
                        }
                    }
                }
            } else {
                current_display = -1;
                current_item = -1;
            }
            continue;
        }
        
        // 全局配置
        if (strncmp(s, "device", 6) == 0) {
            char* val = parse_value(s);
            if (val) strncpy(cfg->i2c_dev, val, sizeof(cfg->i2c_dev) - 1);
        } else if (strncmp(s, "refresh", 7) == 0) {
            char* val = parse_value(s);
            if (val) cfg->global_refresh = atoi(val);
        } else if (strncmp(s, "invert", 6) == 0) {
            char* val = parse_value(s);
            if (val) cfg->invert_display = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        }
        
        // 显示器配置
        if (current_display >= 0 && current_display < 2) {
            DisplayConfig* disp = &cfg->displays[current_display];
            
            if (strncmp(s, "addr", 4) == 0) {
                char* val = parse_value(s);
                if (val) disp->i2c_addr = strtol(val, NULL, 0);
            } else if (strncmp(s, "enabled", 7) == 0) {
                char* val = parse_value(s);
                if (val) disp->enabled = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
            } else if (strncmp(s, "items", 5) == 0) {
                // 解析items列表，格式: items = sysinfo, time, ...
                char* val = parse_value(s);
                if (val) {
                    char* token = strtok(val, ", ");
                    while (token && disp->item_count < 8) {
                        ItemType type = item_type_from_string(token);
                        if (type != ITEM_NONE) {
                            DisplayItem* item = &disp->items[disp->item_count];
                            memset(item, 0, sizeof(DisplayItem));
                            strncpy(item->name, token, sizeof(item->name) - 1);
                            item->type = type;
                            item->enabled = 1;
                            item->refresh_interval = 0;  // 使用默认值
                            
                            // 设置默认配置
                            if (type == ITEM_SYSINFO) {
                                item->config.sysinfo.show_ip = 1;
                                item->config.sysinfo.show_cpu = 1;
                                item->config.sysinfo.show_mem = 1;
                                item->config.sysinfo.show_disk = 1;
                            } else if (type == ITEM_TIME) {
                                strcpy(item->config.time.format, "%H:%M:%S");
                            }
                            
                            disp->item_count++;
                        }
                        token = strtok(NULL, ", ");
                    }
                }
            } else if (strncmp(s, "refresh", 7) == 0) {
                char* val = parse_value(s);
                if (val) disp->refresh_interval = atoi(val);
            }
        }
        
        // 显示项配置
        if (current_display >= 0 && current_display < 2 && current_item >= 0) {
            DisplayItem* item = &cfg->displays[current_display].items[current_item];
            
            if (strncmp(s, "type", 4) == 0) {
                char* val = parse_value(s);
                if (val) item->type = item_type_from_string(val);
            } else if (strncmp(s, "x", 1) == 0 && s[1] != '\0' && !isalpha(s[1])) {
                char* val = parse_value(s);
                if (val) item->x = atoi(val);
            } else if (strncmp(s, "y", 1) == 0 && s[1] != '\0' && !isalpha(s[1])) {
                char* val = parse_value(s);
                if (val) item->y = atoi(val);
            } else if (strncmp(s, "position", 8) == 0) {
                char* val = parse_value(s);
                if (val) sscanf(val, "%d, %d", &item->x, &item->y);
            } else if (strncmp(s, "refresh", 7) == 0) {
                char* val = parse_value(s);
                if (val) item->refresh_interval = atoi(val);
            } else if (strncmp(s, "enabled", 7) == 0) {
                char* val = parse_value(s);
                if (val) item->enabled = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
            } else if (strncmp(s, "text", 4) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.text.text, val, sizeof(item->config.text.text) - 1);
            } else if (strncmp(s, "url", 3) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.http.url, val, sizeof(item->config.http.url) - 1);
            } else if (strncmp(s, "json_field", 10) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.http.json_field, val, sizeof(item->config.http.json_field) - 1);
            } else if (strncmp(s, "command", 7) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.cmd.command, val, sizeof(item->config.cmd.command) - 1);
            } else if (strncmp(s, "format", 6) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.time.format, val, sizeof(item->config.time.format) - 1);
            } else if (strncmp(s, "vmid", 4) == 0) {
                char* val = parse_value(s);
                if (val) strncpy(item->config.fans.vmid, val, sizeof(item->config.fans.vmid) - 1);
            } else if (strncmp(s, "lines", 5) == 0) {
                // sysinfo的lines配置
                char* val = parse_value(s);
                if (val) {
                    char* token = strtok(val, ", ");
                    while (token) {
                        if (strcmp(token, "ip") == 0) item->config.sysinfo.show_ip = 1;
                        else if (strcmp(token, "cpu") == 0) item->config.sysinfo.show_cpu = 1;
                        else if (strcmp(token, "mem") == 0 || strcmp(token, "memory") == 0) item->config.sysinfo.show_mem = 1;
                        else if (strcmp(token, "disk") == 0) item->config.sysinfo.show_disk = 1;
                        token = strtok(NULL, ", ");
                    }
                }
            }
        }
    }
    
    fclose(fp);
    return 0;
}

ItemType item_type_from_string(const char* str) {
    if (strcmp(str, "sysinfo") == 0) return ITEM_SYSINFO;
    if (strcmp(str, "text") == 0) return ITEM_TEXT;
    if (strcmp(str, "http") == 0) return ITEM_HTTP;
    if (strcmp(str, "cmd") == 0 || strcmp(str, "command") == 0) return ITEM_CMD;
    if (strcmp(str, "time") == 0) return ITEM_TIME;
    if (strcmp(str, "fans") == 0 || strcmp(str, "fans") == 0) return ITEM_FANS;
    return ITEM_NONE;
}

const char* item_type_to_string(ItemType type) {
    switch (type) {
        case ITEM_SYSINFO: return "sysinfo";
        case ITEM_TEXT:    return "text";
        case ITEM_HTTP:    return "http";
        case ITEM_CMD:     return "cmd";
        case ITEM_TIME:    return "time";
        case ITEM_FANS:    return "fans";
        default:           return "none";
    }
}

static void print_usage(const char* prog) {
    printf("用法: %s [选项]\n", prog);
    printf("\n选项:\n");
    printf("  -c, --config <file>    配置文件路径 (默认: /etc/oled_stats.conf)\n");
    printf("  -d, --device <dev>     I2C设备路径 (默认: /dev/i2c-1)\n");
    printf("  -i, --interval <sec>   刷新间隔 (默认: 25秒)\n");
    printf("  -n, --invert           反转显示颜色\n");
    printf("  -D, --daemon           以守护进程运行\n");
    printf("\n显示项配置 (命令行):\n");
    printf("  --item0 <type>         显示器0的显示项 (逗号分隔)\n");
    printf("  --item1 <type>         显示器1的显示项 (逗号分隔)\n");
    printf("  --text <text>          添加静态文本项\n");
    printf("  --cmd <command>        添加命令项\n");
    printf("  --fans <vmid>          添加B站粉丝项\n");
    printf("\n显示项类型: sysinfo, text, time, fans, cmd, http\n");
    printf("\n示例:\n");
    printf("  %s --item0 sysinfo --item1 fans:472537678\n", prog);
    printf("  %s --item0 sysinfo --item1 \"cmd:hostname -I\"\n", prog);
    printf("  %s -c /path/to/config.conf\n", prog);
    printf("  -h, --help             显示帮助信息\n");
}

static void parse_item_spec(DisplayConfig* disp, const char* spec) {
    char buf[256];
    strncpy(buf, spec, sizeof(buf) - 1);
    
    char* token = strtok(buf, ", ");
    while (token && disp->item_count < 8) {
        char* colon = strchr(token, ':');
        char type_str[32] = {0};
        char param[64] = {0};
        
        if (colon) {
            strncpy(type_str, token, colon - token);
            strncpy(param, colon + 1, sizeof(param) - 1);
        } else {
            strncpy(type_str, token, sizeof(type_str) - 1);
        }
        
        ItemType type = item_type_from_string(type_str);
        if (type != ITEM_NONE) {
            DisplayItem* item = &disp->items[disp->item_count];
            memset(item, 0, sizeof(DisplayItem));
            strncpy(item->name, type_str, sizeof(item->name) - 1);
            item->type = type;
            item->enabled = 1;
            item->x = 0;
            item->y = disp->item_count * 12;  // 默认每行12像素
            
            // 设置默认配置
            if (type == ITEM_SYSINFO) {
                item->config.sysinfo.show_ip = 1;
                item->config.sysinfo.show_cpu = 1;
                item->config.sysinfo.show_mem = 1;
                item->config.sysinfo.show_disk = 1;
            } else if (type == ITEM_TIME) {
                strcpy(item->config.time.format, "%H:%M:%S");
            } else if (type == ITEM_FANS && param[0]) {
                strncpy(item->config.fans.vmid, param, sizeof(item->config.fans.vmid) - 1);
            } else if (type == ITEM_TEXT && param[0]) {
                strncpy(item->config.text.text, param, sizeof(item->config.text.text) - 1);
            } else if (type == ITEM_CMD && param[0]) {
                strncpy(item->config.cmd.command, param, sizeof(item->config.cmd.command) - 1);
            }
            
            disp->item_count++;
        }
        
        token = strtok(NULL, ", ");
    }
}

int config_load_args(Config* cfg, int argc, char* argv[]) {
    static struct option long_options[] = {
        {"config",   required_argument, 0, 'c'},
        {"device",   required_argument, 0, 'd'},
        {"interval", required_argument, 0, 'i'},
        {"invert",   no_argument,       0, 'n'},
        {"daemon",   no_argument,       0, 'D'},
        {"item0",    required_argument, 0, '0'},
        {"item1",    required_argument, 0, '1'},
        {"text",     required_argument, 0, 't'},
        {"cmd",      required_argument, 0, 'C'},
        {"fans",     required_argument, 0, 'f'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    char* config_file = NULL;
    int cmdline_items = 0;  // 是否有命令行指定的items
    
    // 先解析非配置文件的参数
    optind = 1;
    while ((opt = getopt_long(argc, argv, "c:d:i:nDh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                config_file = optarg;
                break;
            case 'd':
                strncpy(cfg->i2c_dev, optarg, sizeof(cfg->i2c_dev) - 1);
                break;
            case 'i':
                cfg->global_refresh = atoi(optarg);
                break;
            case 'n':
                cfg->invert_display = 1;
                break;
            case 'D':
                cfg->daemon_mode = 1;
                break;
            case '0':
                parse_item_spec(&cfg->displays[0], optarg);
                cmdline_items = 1;
                break;
            case '1':
                parse_item_spec(&cfg->displays[1], optarg);
                cmdline_items = 1;
                break;
            case 't':
                // 添加文本项
                {
                    DisplayConfig* disp = &cfg->displays[0];
                    DisplayItem* item = &disp->items[disp->item_count];
                    memset(item, 0, sizeof(DisplayItem));
                    item->type = ITEM_TEXT;
                    item->enabled = 1;
                    strncpy(item->config.text.text, optarg, sizeof(item->config.text.text) - 1);
                    disp->item_count++;
                    cmdline_items = 1;
                }
                break;
            case 'C':
                // 添加命令项
                {
                    DisplayConfig* disp = &cfg->displays[0];
                    DisplayItem* item = &disp->items[disp->item_count];
                    memset(item, 0, sizeof(DisplayItem));
                    item->type = ITEM_CMD;
                    item->enabled = 1;
                    strncpy(item->config.cmd.command, optarg, sizeof(item->config.cmd.command) - 1);
                    disp->item_count++;
                    cmdline_items = 1;
                }
                break;
            case 'f':
                // 添加粉丝项
                {
                    DisplayConfig* disp = &cfg->displays[1];
                    DisplayItem* item = &disp->items[disp->item_count];
                    memset(item, 0, sizeof(DisplayItem));
                    item->type = ITEM_FANS;
                    item->enabled = 1;
                    strncpy(item->config.fans.vmid, optarg, sizeof(item->config.fans.vmid) - 1);
                    disp->item_count++;
                    cmdline_items = 1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                return 1;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    // 如果没有命令行指定的items，尝试加载配置文件
    if (!cmdline_items && config_file) {
        config_load_file(cfg, config_file);
    }
    
    // 如果仍然没有items，使用默认配置
    if (cfg->displays[0].item_count == 0) {
        DisplayItem* item = &cfg->displays[0].items[0];
        item->type = ITEM_SYSINFO;
        item->enabled = 1;
        item->config.sysinfo.show_ip = 1;
        item->config.sysinfo.show_cpu = 1;
        item->config.sysinfo.show_mem = 1;
        item->config.sysinfo.show_disk = 1;
        cfg->displays[0].item_count = 1;
    }
    
    return 0;
}

DisplayItem* display_item_create(DisplayConfig* disp, ItemType type, const char* name) {
    if (disp->item_count >= 8) return NULL;
    
    DisplayItem* item = &disp->items[disp->item_count];
    memset(item, 0, sizeof(DisplayItem));
    
    if (name) strncpy(item->name, name, sizeof(item->name) - 1);
    item->type = type;
    item->enabled = 1;
    
    disp->item_count++;
    return item;
}

void display_item_set_sysinfo(DisplayItem* item, int show_ip, int show_cpu, int show_mem, int show_disk) {
    item->type = ITEM_SYSINFO;
    item->config.sysinfo.show_ip = show_ip;
    item->config.sysinfo.show_cpu = show_cpu;
    item->config.sysinfo.show_mem = show_mem;
    item->config.sysinfo.show_disk = show_disk;
}

void display_item_set_text(DisplayItem* item, const char* text) {
    item->type = ITEM_TEXT;
    strncpy(item->config.text.text, text, sizeof(item->config.text.text) - 1);
}

void display_item_set_http(DisplayItem* item, const char* url, const char* json_field) {
    item->type = ITEM_HTTP;
    strncpy(item->config.http.url, url, sizeof(item->config.http.url) - 1);
    if (json_field) {
        strncpy(item->config.http.json_field, json_field, sizeof(item->config.http.json_field) - 1);
    }
}

void display_item_set_cmd(DisplayItem* item, const char* command) {
    item->type = ITEM_CMD;
    strncpy(item->config.cmd.command, command, sizeof(item->config.cmd.command) - 1);
}

void display_item_set_time(DisplayItem* item, const char* format) {
    item->type = ITEM_TIME;
    strncpy(item->config.time.format, format, sizeof(item->config.time.format) - 1);
}

void display_item_set_fans(DisplayItem* item, const char* vmid) {
    item->type = ITEM_FANS;
    strncpy(item->config.fans.vmid, vmid, sizeof(item->config.fans.vmid) - 1);
}
