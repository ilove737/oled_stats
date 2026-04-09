/**
 * SSD1306 OLED Display Driver Implementation
 */

#include "ssd1306.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Adafruit GFX 5x7字体 (ASCII 0x20-0x7E)
// 每个字符5位宽x8位高
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '\''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // '*'
    {0x08, 0x3E, 0x08, 0x3E, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // '['
    {0x02, 0x04, 0x08, 0x10, 0x20}, // '\\'
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ']'
    {0x04, 0x02, 0x01, 0x02, 0x04}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40}, // '_'
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '`'
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a'
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f'
    {0x0C, 0x54, 0x54, 0x54, 0x3C}, // 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j'
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p'
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's'
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 'z'
    {0x00, 0x08, 0x36, 0x41, 0x00}, // '{'
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // '|'
    {0x00, 0x41, 0x36, 0x08, 0x00}, // '}'
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // '~'
};

#define FONT5X7_FIRST_CHAR 32
#define FONT5X7_LAST_CHAR 126
#define FONT5X7_WIDTH 5
#define FONT5X7_HEIGHT 7
#define FONT5X7_GAP 1  // 字符间距

static int i2c_write_command(SSD1306* oled, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};  // 0x00 = command
    if (write(oled->i2c_fd, buf, 2) != 2) {
        return -1;
    }
    return 0;
}

static int i2c_write_data(SSD1306* oled, const uint8_t* data, int len) {
    uint8_t* buf = malloc(len + 1);
    if (!buf) return -1;
    
    buf[0] = 0x40;  // 0x40 = data
    memcpy(buf + 1, data, len);
    
    int ret = write(oled->i2c_fd, buf, len + 1);
    free(buf);
    
    return (ret == len + 1) ? 0 : -1;
}

SSD1306* ssd1306_init(const char* dev, uint8_t addr) {
    SSD1306* oled = calloc(1, sizeof(SSD1306));
    if (!oled) return NULL;
    
    oled->i2c_addr = addr;
    oled->width = SSD1306_WIDTH;
    oled->height = SSD1306_HEIGHT;
    
    // 打开I2C设备
    oled->i2c_fd = open(dev, O_RDWR);
    if (oled->i2c_fd < 0) {
        perror("Failed to open I2C device");
        free(oled);
        return NULL;
    }
    
    // 设置I2C从机地址
    if (ioctl(oled->i2c_fd, I2C_SLAVE, addr) < 0) {
        perror("Failed to set I2C address");
        close(oled->i2c_fd);
        free(oled);
        return NULL;
    }
    
    // 初始化序列
    i2c_write_command(oled, SSD1306_DISPLAYOFF);
    i2c_write_command(oled, SSD1306_SETDISPLAYCLOCKDIV);
    i2c_write_command(oled, 0x80);  // 建议分频比
    i2c_write_command(oled, SSD1306_SETMULTIPLEX);
    i2c_write_command(oled, 0x3F);  // 64路复用
    i2c_write_command(oled, SSD1306_SETSTARTLINE | 0x00);
    i2c_write_command(oled, SSD1306_CHARGEPUMP);
    i2c_write_command(oled, 0x14);  // 启用内部DC-DC
    i2c_write_command(oled, SSD1306_MEMORYMODE);
    i2c_write_command(oled, 0x00);  // 水平寻址模式
    i2c_write_command(oled, SSD1306_SEGREMAP | 0x01);
    i2c_write_command(oled, SSD1306_COMSCANDEC);
    i2c_write_command(oled, SSD1306_SETCOMPAD);
    i2c_write_command(oled, 0x12);
    i2c_write_command(oled, SSD1306_SETCONTRAST);
    i2c_write_command(oled, 0xCF);
    i2c_write_command(oled, SSD1306_SETPRECHARGE);
    i2c_write_command(oled, 0xF1);
    i2c_write_command(oled, SSD1306_SETVCOMDETECT);
    i2c_write_command(oled, 0x40);
    i2c_write_command(oled, SSD1306_DISPLAYALLON_RESUME);
    i2c_write_command(oled, SSD1306_NORMALDISPLAY);
    i2c_write_command(oled, SSD1306_DEACTIVATE_SCROLL);
    i2c_write_command(oled, SSD1306_DISPLAYON);
    
    return oled;
}

void ssd1306_deinit(SSD1306* oled) {
    if (oled) {
        if (oled->i2c_fd >= 0) {
            ssd1306_clear(oled);
            ssd1306_display(oled);
            close(oled->i2c_fd);
        }
        free(oled);
    }
}

void ssd1306_clear(SSD1306* oled) {
    memset(oled->buffer, 0, SSD1306_BUFFER_SIZE);
}

void ssd1306_display(SSD1306* oled) {
    i2c_write_command(oled, SSD1306_COLUMNADDR);
    i2c_write_command(oled, 0);
    i2c_write_command(oled, SSD1306_WIDTH - 1);
    i2c_write_command(oled, SSD1306_PAGEADDR);
    i2c_write_command(oled, 0);
    i2c_write_command(oled, 7);
    
    // 分批发送数据，避免I2C缓冲区溢出
    for (int i = 0; i < SSD1306_BUFFER_SIZE; i += 16) {
        int len = (i + 16 < SSD1306_BUFFER_SIZE) ? 16 : SSD1306_BUFFER_SIZE - i;
        i2c_write_data(oled, oled->buffer + i, len);
    }
}

void ssd1306_set_pixel(SSD1306* oled, int x, int y, int color) {
    if (x < 0 || x >= oled->width || y < 0 || y >= oled->height) return;
    
    if (color) {
        oled->buffer[(y / 8) * oled->width + x] |= (1 << (y & 7));
    } else {
        oled->buffer[(y / 8) * oled->width + x] &= ~(1 << (y & 7));
    }
}

int ssd1306_draw_char(SSD1306* oled, int x, int y, char c, int color) {
    if (c < FONT5X7_FIRST_CHAR || c > FONT5X7_LAST_CHAR) {
        c = '?';
    }
    
    int idx = c - FONT5X7_FIRST_CHAR;
    
    // 绘制字符 (列方向)
    for (int col = 0; col < FONT5X7_WIDTH; col++) {
        uint8_t line = font5x7[idx][col];
        for (int row = 0; row < FONT5X7_HEIGHT; row++) {
            int pixel = (line >> row) & 1;
            ssd1306_set_pixel(oled, x + col, y + row, color ? pixel : !pixel);
        }
    }
    
    return FONT5X7_WIDTH + FONT5X7_GAP;
}

int ssd1306_draw_string(SSD1306* oled, int x, int y, const char* str, int color) {
    int orig_x = x;
    while (*str) {
        if (*str == '\n') {
            x = orig_x;
            y += FONT5X7_HEIGHT + 1;
        } else {
            x += ssd1306_draw_char(oled, x, y, *str, color);
        }
        str++;
    }
    return x - orig_x;
}

int ssd1306_draw_big_num(SSD1306* oled, int x, int y, const char* num, int color) {
    // 使用普通字符显示
    return ssd1306_draw_string(oled, x, y, num, color);
}

void ssd1306_draw_hline(SSD1306* oled, int x, int y, int w, int color) {
    for (int i = 0; i < w; i++) {
        ssd1306_set_pixel(oled, x + i, y, color);
    }
}

void ssd1306_draw_vline(SSD1306* oled, int x, int y, int h, int color) {
    for (int i = 0; i < h; i++) {
        ssd1306_set_pixel(oled, x, y + i, color);
    }
}

void ssd1306_draw_rect(SSD1306* oled, int x, int y, int w, int h, int color) {
    ssd1306_draw_hline(oled, x, y, w, color);
    ssd1306_draw_hline(oled, x, y + h - 1, w, color);
    ssd1306_draw_vline(oled, x, y, h, color);
    ssd1306_draw_vline(oled, x + w - 1, y, h, color);
}

void ssd1306_fill_rect(SSD1306* oled, int x, int y, int w, int h, int color) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            ssd1306_set_pixel(oled, x + i, y + j, color);
        }
    }
}

int ssd1306_draw_ppm(SSD1306* oled, int x, int y, const char* ppm_file) {
    FILE* fp = fopen(ppm_file, "rb");
    if (!fp) return -1;
    
    char magic[3];
    int width, height, maxval;
    
    if (fscanf(fp, "%2s", magic) != 1 || strcmp(magic, "P6") != 0) {
        fclose(fp);
        return -1;
    }
    
    // 跳过注释
    int c;
    while ((c = fgetc(fp)) == '#') {
        while ((c = fgetc(fp)) != '\n' && c != EOF);
    }
    ungetc(c, fp);
    
    if (fscanf(fp, "%d %d %d", &width, &height, &maxval) != 3) {
        fclose(fp);
        return -1;
    }
    
    fgetc(fp);  // 跳过空白字符
    
    // 读取像素数据
    for (int j = 0; j < height && j + y < oled->height; j++) {
        for (int i = 0; i < width && i + x < oled->width; i++) {
            unsigned char rgb[3];
            if (fread(rgb, 1, 3, fp) != 3) break;
            
            // 转换为灰度并二值化
            int gray = (rgb[0] * 299 + rgb[1] * 587 + rgb[2] * 114) / 1000;
            ssd1306_set_pixel(oled, x + i, y + j, gray > 128 ? 1 : 0);
        }
    }
    
    fclose(fp);
    return 0;
}

void ssd1306_invert(SSD1306* oled, int invert) {
    i2c_write_command(oled, invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}