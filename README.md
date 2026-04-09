# OLED 系统状态显示程序

基于 SSD1306 (128x64) OLED 显示屏的系统状态监控程序，支持双屏显示和自定义显示项。

## 功能特性

- 支持双 OLED 显示屏 (I2C地址 0x3C 和 0x3D)
- 6 种显示项类型：`sysinfo`、`time`、`fans`、`text`、`cmd`、`http`
- 支持配置文件和命令行参数两种配置方式
- 自动防止烧屏（画面交替显示）

## 编译安装

```bash
# 编译
make

# 安装到系统
sudo make install

# 运行
stats
```

## 快速开始

```bash
# 默认显示系统信息
./stats

# 显示系统信息 + B站粉丝
./stats --item0 sysinfo --item1 "fans:472537678"

# 显示时间 + 命令输出
./stats --item0 "time,cmd:hostname -I" --item1 "text:Hello World"
```

## 显示项类型

### 1. sysinfo - 系统信息

显示系统状态：IP地址、CPU负载、内存使用、磁盘使用。

**命令行：**
```bash
./stats --item0 sysinfo
```

**配置文件：**
```ini
[display:0]
items = sysinfo

[item:sysinfo]
type = sysinfo
lines = ip, cpu, mem, disk    # 选择要显示的项
```

显示效果：
```
IP: 192.168.1.100
CPU: 0.52
M: 800/4000MB 20.0%
D: 10.5/50.0GB 21.0%
```

---

### 2. time - 时间显示

显示当前时间，可自定义格式。

**命令行：**
```bash
./stats --item0 time
```

**配置文件：**
```ini
[item:time]
type = time
format = %H:%M:%S          # 时间格式
position = 0, 0            # 显示位置
```

**常用时间格式：**
| 格式 | 说明 | 示例 |
|------|------|------|
| `%H:%M:%S` | 时:分:秒 | 14:30:25 |
| `%Y-%m-%d` | 年-月-日 | 2024-04-09 |
| `%m/%d %H:%M` | 月/日 时:分 | 04/09 14:30 |
| `%A %H:%M` | 星期 时:分 | Tuesday 14:30 |

---

### 3. fans - B站粉丝数

显示指定用户的B站粉丝数量。

**命令行：**
```bash
./stats --item1 "fans:472537678"
```

**配置文件：**
```ini
[item:fans]
type = fans
vmid = 472537678          # B站用户ID
position = 0, 20
```

显示效果：
```
Fans: 12345
```

---

### 4. text - 静态文本

显示固定的文本内容。

**命令行：**
```bash
./stats --item1 "text:Hello World"
./stats --item1 "text:服务器状态"
```

**配置文件：**
```ini
[item:label]
type = text
text = Server Online
position = 0, 30
```

显示效果：
```
Hello World
```

---

### 5. cmd - 命令输出

执行 shell 命令并显示输出结果。

**命令行：**
```bash
# 显示IP地址
./stats --item0 "cmd:hostname -I | cut -d' ' -f1"

# 显示温度
./stats --item0 "cmd:vcgencmd measure_temp | cut -d= -f2"

# 显示主机名
./stats --item1 "cmd:hostname"
```

**配置文件：**
```ini
[item:temp]
type = cmd
command = vcgencmd measure_temp | cut -d= -f2
position = 0, 40
refresh = 5              # 每5秒刷新
```

**常用命令示例：**
```bash
# CPU温度 (树莓派)
cmd:vcgencmd measure_temp

# 外网IP
cmd:curl -s ifconfig.me

# 当前用户
cmd:whoami

# 系统运行时间
cmd:uptime -p

# 磁盘使用率
cmd:df -h / | awk 'NR==2{print $5}'
```

---

### 6. http - HTTP 数据

从 HTTP API 获取数据并显示（支持 JSON 解析）。

**配置文件：**
```ini
[item:weather]
type = http
url = https://api.weather.com/current
json_field = temperature   # 提取JSON字段
format = Temp: %s°C
position = 0, 50
```

---

## 配置方式

### 方式一：命令行参数

```bash
# 完整示例
./stats \
  --device /dev/i2c-1 \
  --interval 10 \
  --item0 "sysinfo" \
  --item1 "time,fans:472537678" \
  --invert

# 参数说明
--device, -d    I2C设备路径 (默认: /dev/i2c-1)
--interval, -i  刷新间隔秒数 (默认: 25)
--invert, -n    反转显示颜色
--daemon, -D    以守护进程运行
--item0         显示器0的显示项
--item1         显示器1的显示项
--config, -c    配置文件路径
```

### 方式二：配置文件

配置文件路径：`/etc/oled_stats.conf`

```ini
# 全局配置
[global]
device = /dev/i2c-1
refresh = 25
invert = false

# 显示器0 (I2C地址 0x3C)
[display:0]
addr = 0x3C
enabled = true
items = sysinfo

# 显示器1 (I2C地址 0x3D)
[display:1]
addr = 0x3D
enabled = true
items = time, fans, temp
refresh = 10

# 系统信息配置
[item:sysinfo]
type = sysinfo
lines = ip, cpu, mem, disk

# 时间配置
[item:time]
type = time
format = %H:%M:%S
position = 0, 0

# B站粉丝配置
[item:fans]
type = fans
vmid = 472537678
position = 0, 15
refresh = 60          # 每60秒刷新

# 温度配置 (命令)
[item:temp]
type = cmd
command = vcgencmd measure_temp | cut -d= -f2
position = 0, 30
refresh = 5
```

使用配置文件：
```bash
./stats -c /etc/oled_stats.conf
```

---

## 配置示例

### 示例1：服务器监控

```ini
[global]
refresh = 10

[display:0]
items = sysinfo

[display:1]
items = time, cmd:uptime -p

[item:sysinfo]
lines = ip, cpu, mem
```

### 示例2：树莓派状态

```bash
./stats --item0 "sysinfo" --item1 "time,cmd:vcgencmd measure_temp"
```

### 示例3：B站粉丝监控

```ini
[display:0]
items = text, fans, time

[item:text]
type = text
text = Bilibili Fans

[item:fans]
type = fans
vmid = 472537678

[item:time]
type = time
format = %Y-%m-%d %H:%M
position = 0, 50
```

---

## 项目结构

```
oled_stats/
├── stats.c              # 主程序
├── display_item.h/c     # 配置解析和显示项管理
├── ssd1306.h/c          # SSD1306 OLED驱动
├── sysinfo.h/c          # 系统信息获取
├── http.h/c             # HTTP请求处理
├── Makefile             # 编译脚本
└── oled_stats.conf.example  # 配置文件示例
```

## 依赖

- `libcurl` - HTTP请求支持
- `i2c-dev` - I2C设备接口

```bash
# 安装依赖 (Debian/Ubuntu)
sudo apt-get install libcurl4-openssl-dev i2c-tools
```

## License

MIT License
