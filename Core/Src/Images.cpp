#include "Images.hpp"
#include "drivers/Display.hpp"

// Заглушки для изображений (в реальном проекте будут заменены на сгенерированные)
const uint8_t splash_image_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... остальные данные будут сгенерированы image_converter.py
};

const uint8_t help_image_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... остальные данные будут сгенерированы image_converter.py
};

Images& Images::getInstance() {
    static Images instance;
    return instance;
}

bool Images::init() {
    return true;
}

void Images::drawSplashScreen() {
    display.clear(DisplayColor::BLACK);
    drawImage(splash_image_data, SPLASH_WIDTH, SPLASH_HEIGHT, 0, 0);
    display.update();
}

void Images::drawHelpScreen() {
    display.clear(DisplayColor::BLACK);
    drawImage(help_image_data, HELP_WIDTH, HELP_HEIGHT, 0, 0);
    display.update();
}

void Images::drawImage(const uint8_t* imageData, uint16_t width, uint16_t height, uint8_t x, uint8_t y) {
    // Простая реализация отрисовки изображения
    // В реальной реализации здесь будет более сложная логика
    display.setCursor(x, y);
    display.print("IMAGE");
}
