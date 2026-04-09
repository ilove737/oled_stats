# OLED系统状态显示程序 Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I.
LDFLAGS = -lcurl

# 目标程序
TARGET = stats

# 源文件
SRCS = stats.c ssd1306.c sysinfo.c http.c display_item.c
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 编译
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译对象文件
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 安装到系统
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	install -m 644 oled_stats.conf.example /etc/oled_stats.conf 2>/dev/null || true

# 卸载
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -f /etc/oled_stats.conf

# 交叉编译 (树莓派)
rpi: CC = arm-linux-gnueabihf-gcc
rpi: $(TARGET)

# 调试版本
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# 显示帮助
help:
	@echo "可用目标:"
	@echo "  all      - 编译程序"
	@echo "  clean    - 清理编译文件"
	@echo "  install  - 安装到系统"
	@echo "  uninstall- 从系统卸载"
	@echo "  rpi      - 交叉编译 (树莓派)"
	@echo "  debug    - 编译调试版本"
	@echo "  help     - 显示帮助"

.PHONY: all clean install uninstall rpi debug help