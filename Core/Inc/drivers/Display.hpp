#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <stdint.h>
#include <stdbool.h>

// Цвета для OLED
enum class DisplayColor {
    BLACK = 0x00,
    WHITE = 0x01
};

// Структура точки
struct Point {
    uint8_t x, y;
    Point(uint8_t x = 0, uint8_t y = 0) : x(x), y(y) {}
};

// Структура прямоугольника
struct Rect {
    uint8_t x, y, width, height;
    Rect(uint8_t x = 0, uint8_t y = 0, uint8_t w = 0, uint8_t h = 0) 
        : x(x), y(y), width(w), height(h) {}
};

class Display {
public:
    static Display& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление экраном
    void clear(DisplayColor color = DisplayColor::BLACK);
    void update();
    void setInverted(bool inverted);
    
    // Установка курсора для текста
    void setCursor(uint8_t x, uint8_t y);
    Point getCursor() const { return Point(currentX, currentY); }
    
    // Рисование пикселей
    void drawPixel(uint8_t x, uint8_t y, DisplayColor color);
    DisplayColor getPixel(uint8_t x, uint8_t y) const;
    
    // Рисование линий
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, DisplayColor color);
    void drawHLine(uint8_t x1, uint8_t x2, uint8_t y, DisplayColor color);
    void drawVLine(uint8_t y1, uint8_t y2, uint8_t x, DisplayColor color);
    
    // Рисование фигур
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, DisplayColor color);
    void drawFilledRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, DisplayColor color);
    void drawCircle(uint8_t x, uint8_t y, uint8_t radius, DisplayColor color);
    void drawFilledCircle(uint8_t x, uint8_t y, uint8_t radius, DisplayColor color);
    
    // Работа с текстом
    void drawChar(char ch, uint8_t x, uint8_t y, DisplayColor color);
    void drawString(const char* str, uint8_t x, uint8_t y, DisplayColor color);
    void print(const char* str);
    void print(char ch);
    void printf(const char* format, ...);
    
    // Размеры экрана
    static constexpr uint8_t WIDTH = 128;
    static constexpr uint8_t HEIGHT = 64;
    
private:
    Display() = default;
    ~Display() = default;
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    
    // Константы для OLED
    static constexpr uint8_t OLED_I2C_ADDR = 0x78;
    static constexpr uint16_t BUFFER_SIZE = 1024; // 128 * 64 / 8
    
    // Состояние дисплея
    uint8_t buffer[BUFFER_SIZE];
    uint8_t currentX, currentY;
    bool inverted;
    bool initialized;
    
    // Внутренние методы
    void writeCommand(uint8_t command);
    void writeData(uint8_t* data, uint16_t size);
    bool isPixelInBounds(uint8_t x, uint8_t y) const;
    void setPixelInBuffer(uint8_t x, uint8_t y, DisplayColor color);
    DisplayColor getPixelFromBuffer(uint8_t x, uint8_t y) const;
};

#endif // DISPLAY_HPP
