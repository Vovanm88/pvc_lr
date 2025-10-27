#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <stdint.h>
#include <stdbool.h>

// События клавиатуры
enum class KeyEvent {
    PRESS,
    RELEASE,
    LONG_PRESS
};

// Callback функция для событий клавиатуры
typedef void (*KeyboardCallback)(uint8_t keyCode, KeyEvent event);

class Keyboard {
public:
    static Keyboard& getInstance();
    
    // Инициализация
    bool init();
    
    // Сканирование клавиатуры (вызывается из задачи)
    void scan();
    
    // Установка callback функции
    void setCallback(KeyboardCallback callback);
    
    // Получение состояния кнопки
    bool isPressed(uint8_t keyCode) const;
    bool wasPressed(uint8_t keyCode) const;
    
    // Коды кнопок (1-12)
    // Матрица: 1 2 3
    //          4 5 6  
    //          7 8 9
    //          11 10 12
    static constexpr uint8_t KEY_1 = 1;
    static constexpr uint8_t KEY_2 = 2;
    static constexpr uint8_t KEY_3 = 3;
    static constexpr uint8_t KEY_4 = 4;
    static constexpr uint8_t KEY_5 = 5;
    static constexpr uint8_t KEY_6 = 6;
    static constexpr uint8_t KEY_7 = 7;
    static constexpr uint8_t KEY_8 = 8;
    static constexpr uint8_t KEY_9 = 9;
    static constexpr uint8_t KEY_10 = 10;
    static constexpr uint8_t KEY_11 = 11;
    static constexpr uint8_t KEY_12 = 12;
    
private:
    Keyboard() = default;
    ~Keyboard() = default;
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    
    // Константы для PCA9538
    static constexpr uint16_t KBRD_ADDR = 0xE2;
    static constexpr uint8_t ROW1 = 0xFE;
    static constexpr uint8_t ROW2 = 0xFD;
    static constexpr uint8_t ROW3 = 0xFB;
    static constexpr uint8_t ROW4 = 0xF7;
    
    // Параметры подавления дребезга
    static constexpr uint32_t DEBOUNCE_TIME_MS = 20;  // Уменьшено для быстрой реакции
    static constexpr uint32_t LONG_PRESS_TIME_MS = 800;  // Увеличено для более комфортного использования
    
    // Состояние клавиатуры
    bool keyStates[13];        // Текущее состояние (индекс 0 не используется)
    bool prevKeyStates[13];    // Предыдущее состояние
    uint32_t keyPressTime[13]; // Время нажатия для каждой клавиши
    uint32_t lastScanTime;     // Время последнего сканирования
    
    KeyboardCallback callback;
    
    // Внутренние методы
    bool checkRow(uint8_t row);
    uint8_t getKeyCode(uint8_t row, uint8_t col);
    void processKeyEvent(uint8_t keyCode, bool pressed);
};

#endif // KEYBOARD_HPP
