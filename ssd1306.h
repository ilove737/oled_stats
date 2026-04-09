/**
 * SSD1306 OLED Display Driver
 * 支持 128x64 分辨率，I2C接口
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64
#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

// I2C地址
#define SSD1306_I2C_ADDR_0 0x3C
#define SSD1306_I2C_ADDR_1 0x3D

// SSD1306命令
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETLOWCOL           0x00
#define SSD1306_SETHIGHCOL          0x10
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_EXTERNALVCC         0x01
#define SSD1306_SWITCHCAPVCC        0x02
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DEACTIVATE_SCROLL   0x2E
#define SSD1306_SETCOMPAD           0xDA

// SSD1306结构体
typedef struct {
    int i2c_fd;                         // I2C文件描述符
    uint8_t i2c_addr;                   // I2C地址
    uint8_t buffer[SSD1306_BUFFER_SIZE]; // 显示缓冲区
    int width;
    int height;
} SSD1306;

/**
 * 初始化SSD1306显示屏
 * @param dev 设备路径 (如 "/dev/i2c-1")
 * @param addr I2C地址 (0x3C 或 0x3D)
 * @return SSD1306指针，失败返回NULL
 */
SSD1306* ssd1306_init(const char* dev, uint8_t addr);

/**
 * 释放SSD1306资源
 */
void ssd1306_deinit(SSD1306* oled);

/**
 * 清空显示缓冲区
 */
void ssd1306_clear(SSD1306* oled);

/**
 * 将缓冲区内容显示到屏幕
 */
void ssd1306_display(SSD1306* oled);

/**
 * 设置像素点
 * @param x X坐标 (0-127)
 * @param y Y坐标 (0-63)
 * @param color 0=黑色(关闭), 1=白色(点亮)
 */
void ssd1306_set_pixel(SSD1306* oled, int x, int y, int color);

/**
 * 绘制字符 (8x8字体)
 * @param x X坐标
 * @param y Y坐标
 * @param c ASCII字符
 * @param color 颜色
 * @return 字符宽度
 */
int ssd1306_draw_char(SSD1306* oled, int x, int y, char c, int color);

/**
 * 绘制字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 字符串
 * @param color 颜色
 * @return 字符串总宽度
 */
int ssd1306_draw_string(SSD1306* oled, int x, int y, const char* str, int color);

/**
 * 绘制大号数字 (16x24)
 */
int ssd1306_draw_big_num(SSD1306* oled, int x, int y, const char* num, int color);

/**
 * 绘制水平线
 */
void ssd1306_draw_hline(SSD1306* oled, int x, int y, int w, int color);

/**
 * 绘制垂直线
 */
void ssd1306_draw_vline(SSD1306* oled, int x, int y, int h, int color);

/**
 * 绘制矩形
 */
void ssd1306_draw_rect(SSD1306* oled, int x, int y, int w, int h, int color);

/**
 * 绘制填充矩形
 */
void ssd1306_fill_rect(SSD1306* oled, int x, int y, int w, int h, int color);

/**
 * 从PPM文件绘制图像
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param ppm_file PPM文件路径
 * @return 0成功，-1失败
 */
int ssd1306_draw_ppm(SSD1306* oled, int x, int y, const char* ppm_file);

/**
 * 反转显示颜色
 */
void ssd1306_invert(SSD1306* oled, int invert);

#endif // SSD1306_H
