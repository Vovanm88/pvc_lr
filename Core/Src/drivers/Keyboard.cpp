#include "drivers/Keyboard.hpp"
#include "drivers/Uart.hpp"
#include "pca9538.h"
#include "i2c.h"
#include "usart.h"

// Внешние переменные из HAL
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart6;

Keyboard& Keyboard::getInstance() {
    static Keyboard instance;
    return instance;
}

bool Keyboard::init() {
    // Инициализация состояний
    for (int i = 0; i < 13; i++) {
        keyStates[i] = false;
        prevKeyStates[i] = false;
        keyPressTime[i] = 0;
    }
    lastScanTime = 0;
    callback = nullptr;
    
    // Инициализация PCA9538
    uint8_t buf = 0;
    
    // Настройка полярности (не инвертировать)
    if (PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf) != HAL_OK) {
        return false;
    }
    
    // Настройка выходного порта (все выходы в 0)
    if (PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf) != HAL_OK) {
        return false;
    }
    
    // Настройка конфигурации портов (все как входы)
    uint8_t config = 0xFF; // Все пины как входы
    if (PCA9538_Write_Register(KBRD_ADDR, CONFIG, &config) != HAL_OK) {
        return false;
    }
    
    // Небольшая задержка для стабилизации
    HAL_Delay(10);
    return true;
}

void Keyboard::scan() {
    uint32_t currentTime = HAL_GetTick();
    
    // Проверяем, прошло ли достаточно времени с последнего сканирования
    if (currentTime - lastScanTime < DEBOUNCE_TIME_MS) {
        return;
    }
    
    lastScanTime = currentTime;
    
    // Сканируем все строки
    for (uint8_t row = 0; row < 4; row++) {
        uint8_t rowMask;
        switch (row) {
            case 0: rowMask = ROW1; break;
            case 1: rowMask = ROW2; break;
            case 2: rowMask = ROW3; break;
            case 3: rowMask = ROW4; break;
            default: continue;
        }
        
        // Настраиваем строку как выход
        if (PCA9538_Write_Register(KBRD_ADDR, CONFIG, &rowMask) != HAL_OK) {
            continue;
        }
        
        // Небольшая задержка для стабилизации
        HAL_Delay(1);
        
        // Читаем состояние столбцов
        uint8_t colData;
        if (PCA9538_Read_Inputs(KBRD_ADDR, &colData) != HAL_OK) {
            continue;
        }
        
        // Проверяем каждый столбец
        uint8_t kbd_in = colData & 0x70;
        if (kbd_in != 0x70) {
            // Определяем нажатые клавиши
            for (uint8_t col = 0; col < 3; col++) {
                uint8_t colMask = 0x10 << col;
                if (!(kbd_in & colMask)) {
                    uint8_t keyCode = getKeyCode(row, col);
                    if (keyCode > 0 && keyCode <= 12) {
                        processKeyEvent(keyCode, true);
                    }
                }
            }
        }
    }
    
    // Проверяем отпущенные клавиши
    for (uint8_t i = 1; i <= 12; i++) {
        if (prevKeyStates[i] && !keyStates[i]) {
            processKeyEvent(i, false);
        }
    }
    
    // Обновляем предыдущие состояния
    for (uint8_t i = 1; i <= 12; i++) {
        prevKeyStates[i] = keyStates[i];
    }
    
    // Дополнительная проверка: если клавиша была нажата, но сейчас не нажата - отпускаем
    for (uint8_t i = 1; i <= 12; i++) {
        if (keyStates[i]) {
            // Проверяем, действительно ли клавиша все еще нажата
            bool stillPressed = false;
            for (uint8_t row = 0; row < 4; row++) {
                uint8_t rowMask;
                switch (row) {
                    case 0: rowMask = ROW1; break;
                    case 1: rowMask = ROW2; break;
                    case 2: rowMask = ROW3; break;
                    case 3: rowMask = ROW4; break;
                    default: continue;
                }
                
                if (PCA9538_Write_Register(KBRD_ADDR, CONFIG, &rowMask) == HAL_OK) {
                    HAL_Delay(1);
                    uint8_t colData;
                    if (PCA9538_Read_Inputs(KBRD_ADDR, &colData) == HAL_OK) {
                        uint8_t kbd_in = colData & 0x70;
                        if (kbd_in != 0x70) {
                            for (uint8_t col = 0; col < 3; col++) {
                                uint8_t colMask = 0x10 << col;
                                if (!(kbd_in & colMask)) {
                                    uint8_t keyCode = getKeyCode(row, col);
                                    if (keyCode == i) {
                                        stillPressed = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                if (stillPressed) break;
            }
            
            if (!stillPressed) {
                // Клавиша больше не нажата - отпускаем
                keyStates[i] = false;
                processKeyEvent(i, false);
            }
        }
    }
}

void Keyboard::setCallback(KeyboardCallback callback) {
    this->callback = callback;
}

bool Keyboard::isPressed(uint8_t keyCode) const {
    if (keyCode > 0 && keyCode <= 12) {
        return keyStates[keyCode];
    }
    return false;
}

bool Keyboard::wasPressed(uint8_t keyCode) const {
    if (keyCode > 0 && keyCode <= 12) {
        return prevKeyStates[keyCode];
    }
    return false;
}

bool Keyboard::checkRow(uint8_t row) {
    uint8_t rowMask;
    switch (row) {
        case 0: rowMask = ROW1; break;
        case 1: rowMask = ROW2; break;
        case 2: rowMask = ROW3; break;
        case 3: rowMask = ROW4; break;
        default: return false;
    }
    
    if (PCA9538_Write_Register(KBRD_ADDR, CONFIG, &rowMask) != HAL_OK) {
        return false;
    }
    
    uint8_t colData;
    if (PCA9538_Read_Inputs(KBRD_ADDR, &colData) != HAL_OK) {
        return false;
    }
    
    return (colData & 0x70) != 0x70;
}

uint8_t Keyboard::getKeyCode(uint8_t row, uint8_t col) {
    // Матрица клавиш 4x3
    // Строка 0: 1, 2, 3
    // Строка 1: 4, 5, 6  
    // Строка 2: 7, 8, 9
    // Строка 3: 11, 10, 12
    
    if (row >= 4 || col >= 3) {
        return 0;
    }
    
    // Обычные кнопки для первых трех строк
    if (row < 3) {
        return row * 3 + col + 1;
    }
    
    // Специальные кнопки в последней строке
    if (row == 3) {
        switch (col) {
            case 0: return KEY_11;     // 11
            case 1: return KEY_10;     // 10  
            case 2: return KEY_12;     // 12
        }
    }
    
    return 0;
}

void Keyboard::processKeyEvent(uint8_t keyCode, bool pressed) {
    uint32_t currentTime = HAL_GetTick();
    
    if (pressed) {
        if (!keyStates[keyCode]) {
            // Клавиша только что нажата
            keyStates[keyCode] = true;
            keyPressTime[keyCode] = currentTime;
            
            if (callback) {
                callback(keyCode, KeyEvent::PRESS);
            }
        } else {
            // Клавиша уже нажата - проверяем длительное нажатие
            // Проверяем, что LONG_PRESS еще не был отправлен для этой клавиши
            if (currentTime - keyPressTime[keyCode] >= LONG_PRESS_TIME_MS && 
                keyPressTime[keyCode] != 0xFFFFFFFF) { // 0xFFFFFFFF = флаг "LONG_PRESS уже отправлен"
                if (callback) {
                    callback(keyCode, KeyEvent::LONG_PRESS);
                }
                // Устанавливаем флаг, что LONG_PRESS уже отправлен
                keyPressTime[keyCode] = 0xFFFFFFFF;
            }
        }
    } else {
        // Клавиша отпущена
        keyStates[keyCode] = false;
        keyPressTime[keyCode] = 0; // Сбрасываем флаг
        
        if (callback) {
            callback(keyCode, KeyEvent::RELEASE);
        }
    }
}
