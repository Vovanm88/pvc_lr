#include "drivers/Display.hpp"
#include "i2c.h"
#include "fonts.h"
#include <stdarg.h>
#include <stdio.h>

// Внешние переменные из HAL
extern I2C_HandleTypeDef hi2c1;

Display& Display::getInstance() {
    static Display instance;
    return instance;
}

bool Display::init() {
    // Инициализация состояния
    currentX = 0;
    currentY = 0;
    inverted = false;
    initialized = false;
    
    // Очистка буфера
    clear(DisplayColor::BLACK);
    
    // Последовательность инициализации OLED SSD1306
    HAL_Delay(100);
    
    writeCommand(0xAE); // Display OFF
    writeCommand(0x20); // Set Memory Addressing Mode
    writeCommand(0x10); // Page addressing mode
    writeCommand(0xB0); // Set Page Start Address
    writeCommand(0xC8); // Set COM Output Scan Direction
    writeCommand(0x00); // Set Low Column Start Address
    writeCommand(0x10); // Set High Column Start Address
    writeCommand(0x40); // Set Start Line Address
    writeCommand(0x81); // Set Contrast Control
    writeCommand(0xFF); // Contrast value
    writeCommand(0xA1); // Set Segment Re-map
    writeCommand(0xA6); // Set Normal Display
    writeCommand(0xA8); // Set Multiplex Ratio
    writeCommand(0x3F); // 1/64 duty
    writeCommand(0xA4); // Set Entire Display ON
    writeCommand(0xD3); // Set Display Offset
    writeCommand(0x00); // No offset
    writeCommand(0xD5); // Set Display Clock Divide Ratio
    writeCommand(0xF0); // Divide ratio
    writeCommand(0xD9); // Set Pre-charge Period
    writeCommand(0x22); // Pre-charge period
    writeCommand(0xDA); // Set COM Hardware Configuration
    writeCommand(0x12); // COM configuration
    writeCommand(0xDB); // Set VCOMH Deselect Level
    writeCommand(0x20); // VCOMH level
    writeCommand(0x8D); // Charge Pump Setting
    writeCommand(0x14); // Enable charge pump
    writeCommand(0xAF); // Display ON
    
    // Обновляем экран
    update();
    
    initialized = true;
    return true;
}

void Display::clear(DisplayColor color) {
    uint8_t fillValue = (color == DisplayColor::BLACK) ? 0x00 : 0xFF;
    
    for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = fillValue;
    }
}

void Display::update() {
    if (!initialized) return;
    
    for (uint8_t page = 0; page < 8; page++) {
        writeCommand(0xB0 + page); // Set page address
        writeCommand(0x00);        // Set low column address
        writeCommand(0x10);        // Set high column address
        
        // Отправляем данные страницы
        uint8_t* pageData = &buffer[WIDTH * page];
        writeData(pageData, WIDTH);
    }
}

void Display::setInverted(bool inverted) {
    this->inverted = inverted;
}

void Display::setCursor(uint8_t x, uint8_t y) {
    currentX = x;
    currentY = y;
}

void Display::drawPixel(uint8_t x, uint8_t y, DisplayColor color) {
    if (!isPixelInBounds(x, y)) return;
    
    DisplayColor actualColor = inverted ? (color == DisplayColor::BLACK ? DisplayColor::WHITE : DisplayColor::BLACK) : color;
    setPixelInBuffer(x, y, actualColor);
}

DisplayColor Display::getPixel(uint8_t x, uint8_t y) const {
    if (!isPixelInBounds(x, y)) return DisplayColor::BLACK;
    
    DisplayColor bufferColor = getPixelFromBuffer(x, y);
    return inverted ? (bufferColor == DisplayColor::BLACK ? DisplayColor::WHITE : DisplayColor::BLACK) : bufferColor;
}

void Display::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color) {
    // Алгоритм Брезенхема для рисования линий
    int16_t dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int16_t dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy;
    
    int16_t x = x1, y = y1;
    
    while (true) {
        drawPixel(x, y, color);
        
        if (x == x2 && y == y2) break;
        
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void Display::drawHLine(uint8_t x1, uint8_t x2, uint8_t y, DisplayColor color) {
    if (x1 > x2) {
        uint8_t temp = x1;
        x1 = x2;
        x2 = temp;
    }
    
    for (uint8_t x = x1; x <= x2; x++) {
        drawPixel(x, y, color);
    }
}

void Display::drawVLine(uint8_t y1, uint8_t y2, uint8_t x, DisplayColor color) {
    if (y1 > y2) {
        uint8_t temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    for (uint8_t y = y1; y <= y2; y++) {
        drawPixel(x, y, color);
    }
}

void Display::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, DisplayColor color) {
    drawHLine(x, x + width - 1, y, color);                    // Верхняя линия
    drawHLine(x, x + width - 1, y + height - 1, color);      // Нижняя линия
    drawVLine(y, y + height - 1, x, color);                  // Левая линия
    drawVLine(y, y + height - 1, x + width - 1, color);      // Правая линия
}

void Display::drawFilledRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, DisplayColor color) {
    for (uint8_t i = 0; i < height; i++) {
        drawHLine(x, x + width - 1, y + i, color);
    }
}

void Display::drawCircle(uint8_t x, uint8_t y, uint8_t radius, DisplayColor color) {
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t px = 0;
    int16_t py = radius;
    
    drawPixel(x, y + radius, color);
    drawPixel(x, y - radius, color);
    drawPixel(x + radius, y, color);
    drawPixel(x - radius, y, color);
    
    while (px < py) {
        if (f >= 0) {
            py--;
            ddF_y += 2;
            f += ddF_y;
        }
        px++;
        ddF_x += 2;
        f += ddF_x;
        
        drawPixel(x + px, y + py, color);
        drawPixel(x - px, y + py, color);
        drawPixel(x + px, y - py, color);
        drawPixel(x - px, y - py, color);
        drawPixel(x + py, y + px, color);
        drawPixel(x - py, y + px, color);
        drawPixel(x + py, y - px, color);
        drawPixel(x - py, y - px, color);
    }
}

void Display::drawFilledCircle(uint8_t x, uint8_t y, uint8_t radius, DisplayColor color) {
    for (uint8_t i = 0; i <= radius; i++) {
        drawCircle(x, y, i, color);
    }
}

void Display::drawChar(char ch, uint8_t x, uint8_t y, DisplayColor color) {
    if (ch < 32 || ch > 126) return; // Только печатные символы
    
    // Используем шрифт по умолчанию (Font_7x10)
    FontDef font = Font_7x10;
    
    for (uint8_t i = 0; i < font.FontHeight; i++) {
        uint16_t b = font.data[(ch - 32) * font.FontHeight + i];
        for (uint8_t j = 0; j < font.FontWidth; j++) {
            if ((b << j) & 0x8000) {
                drawPixel(x + j, y + i, color);
            } else {
                drawPixel(x + j, y + i, (color == DisplayColor::BLACK) ? DisplayColor::WHITE : DisplayColor::BLACK);
            }
        }
    }
}

void Display::drawString(const char* str, uint8_t x, uint8_t y, DisplayColor color) {
    uint8_t posX = x;
    while (*str) {
        drawChar(*str, posX, y, color);
        posX += 7; // Ширина символа Font_7x10
        str++;
    }
}

void Display::print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            currentX = 0;
            currentY += 10; // Высота шрифта
        } else {
            drawChar(*str, currentX, currentY, DisplayColor::WHITE);
            currentX += 7;
        }
        str++;
    }
}

void Display::print(char ch) {
    if (ch == '\n') {
        currentX = 0;
        currentY += 10;
    } else {
        drawChar(ch, currentX, currentY, DisplayColor::WHITE);
        currentX += 7;
    }
}

void Display::printf(const char* format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    print(buffer);
}

void Display::writeCommand(uint8_t command) {
    HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x00, 1, &command, 1, 10);
}

void Display::writeData(uint8_t* data, uint16_t size) {
    HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x40, 1, data, size, 100);
}

bool Display::isPixelInBounds(uint8_t x, uint8_t y) const {
    return x < WIDTH && y < HEIGHT;
}

void Display::setPixelInBuffer(uint8_t x, uint8_t y, DisplayColor color) {
    if (!isPixelInBounds(x, y)) return;
    
    uint16_t index = x + (y / 8) * WIDTH;
    uint8_t bit = y % 8;
    
    if (color == DisplayColor::WHITE) {
        buffer[index] |= (1 << bit);
    } else {
        buffer[index] &= ~(1 << bit);
    }
}

DisplayColor Display::getPixelFromBuffer(uint8_t x, uint8_t y) const {
    if (!isPixelInBounds(x, y)) return DisplayColor::BLACK;
    
    uint16_t index = x + (y / 8) * WIDTH;
    uint8_t bit = y % 8;
    
    return (buffer[index] & (1 << bit)) ? DisplayColor::WHITE : DisplayColor::BLACK;
}
