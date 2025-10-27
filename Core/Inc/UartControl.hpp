#ifndef UARTCONTROL_HPP
#define UARTCONTROL_HPP

#include <stdint.h>
#include <stdbool.h>
#include "button.h"
#include "led.h"
#include "drivers/Uart.hpp"

// Состояния UART
enum class UartState {
    ENABLED,    // UART включен
    DISABLED    // UART выключен
};

// Класс для управления UART через боковую кнопку
class UartControl {
public:
    static UartControl& getInstance();
    
    // Инициализация
    bool init();
    
    // Обновление (вызывается из задачи)
    void update();
    
    // Получение состояния
    UartState getState() const { return currentState; }
    bool isEnabled() const { return currentState == UartState::ENABLED; }
    
    // Управление UART
    void enableUart();
    void disableUart();
    void toggleUart();
    
    // Константы
    static constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
    static constexpr uint32_t BUTTON_LONG_PRESS_MS = 500;
    static constexpr uint32_t LED_INDICATOR_MS = 200;  // Время индикации светодиода
    
private:
    UartControl() = default;
    ~UartControl() = default;
    UartControl(const UartControl&) = delete;
    UartControl& operator=(const UartControl&) = delete;
    
    // Состояние
    UartState currentState;
    Button_t sideButton;
    Led_t statusLed;
    
    // Индикация
    uint32_t indicatorStartTime;
    bool indicatorActive;
    
    // Внутренние методы
    void handleButtonEvent();
    void updateIndicator();
    void setLedState();
};

#endif // UARTCONTROL_HPP
