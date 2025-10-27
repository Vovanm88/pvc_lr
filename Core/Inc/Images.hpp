#ifndef IMAGES_HPP
#define IMAGES_HPP

#include <stdint.h>
#include "drivers/Display.hpp"

// Класс для работы с изображениями
class Images {
public:
    static Images& getInstance();
    
    // Инициализация
    bool init();
    
    // Отрисовка изображений
    void drawSplashScreen();
    void drawHelpScreen();
    
    // Получение размеров изображений
    static constexpr uint16_t SPLASH_WIDTH = 128;
    static constexpr uint16_t SPLASH_HEIGHT = 64;
    static constexpr uint16_t HELP_WIDTH = 128;
    static constexpr uint16_t HELP_HEIGHT = 64;
    
private:
    Images() : display(Display::getInstance()) {}
    ~Images() = default;
    Images(const Images&) = delete;
    Images& operator=(const Images&) = delete;
    
    Display& display;
    
    // Внутренние методы
    void drawImage(const uint8_t* imageData, uint16_t width, uint16_t height, uint8_t x, uint8_t y);
    void drawPixelFromByte(uint8_t byte, uint8_t x, uint8_t y, uint8_t bitOffset);
};

// Данные изображений (заглушки)
extern const uint8_t splash_image_data[];
extern const uint8_t help_image_data[];

#endif // IMAGES_HPP
