#include "UartControl.hpp"
#include "drivers/Uart.hpp"
#include "scheduler/Scheduler.hpp"
#include "stm32f4xx_hal.h"

UartControl& UartControl::getInstance() {
    static UartControl instance;
    return instance;
}

bool UartControl::init() {
    // Инициализация состояния
    currentState = UartState::ENABLED;  // По умолчанию UART включен
    indicatorActive = false;
    indicatorStartTime = 0;
    
    // Инициализация кнопки (PC15 - боковая кнопка)
    Button_Init(&sideButton, GPIOC, GPIO_PIN_15, BUTTON_DEBOUNCE_MS, BUTTON_LONG_PRESS_MS);
    
    // Инициализация светодиода (PD14 - желтый/красный индикатор)
    Led_Init(&statusLed, GPIOD, GPIO_PIN_14);
    
    // Установка начального состояния светодиода
    setLedState();
    
    // Отладочный вывод
    Uart::getInstance().printf("UartControl: Initialized (PC15 button, PD14 LED)\n");
    Uart::getInstance().printf("UartControl: UART is %s\n", 
        currentState == UartState::ENABLED ? "ENABLED" : "DISABLED");
    
    return true;
}

void UartControl::update() {
    uint32_t currentTime = HAL_GetTick();
    
    // Обработка кнопки
    Button_Poll(&sideButton);
    
    if (Button_EventReady(&sideButton)) {
        handleButtonEvent();
        Button_EventClear(&sideButton);
    }
    
    // Убрали периодический отладочный вывод для улучшения производительности
    
    // Обновление индикации
    updateIndicator();
}

void UartControl::handleButtonEvent() {
    uint8_t isLongPress = Button_EventIsLong(&sideButton);
    
    if (isLongPress) {
        // Длинное нажатие - переключение состояния UART
        toggleUart();
        
        // Индикация красным светодиодом
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET); // Красный светодиод
        indicatorActive = true;
        indicatorStartTime = HAL_GetTick();
        
        // Отладочный вывод
        Uart::getInstance().printf("UART toggled: %s\n", 
            currentState == UartState::ENABLED ? "ENABLED" : "DISABLED");
    } else {
        // Короткое нажатие - вывод информации о задачах + светодиод
        indicatorActive = true;
        indicatorStartTime = HAL_GetTick();
        
        // Зажигаем светодиод для индикации
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // Желтый светодиод
        
        // Вывод информации о задачах
        Uart::getInstance().printf("\n=== TASKS INFO (Button) ===\n");
        Scheduler::getInstance().printTaskInfo();
        Uart::getInstance().printf("Uptime: %lu ms\n", HAL_GetTick());
        Uart::getInstance().printf("=== END TASKS INFO ===\n");
    }
}

void UartControl::updateIndicator() {
    if (indicatorActive) {
        if ((HAL_GetTick() - indicatorStartTime) >= LED_INDICATOR_MS) {
            // Завершаем индикацию
            indicatorActive = false;
            setLedState();
        }
    }
}

void UartControl::setLedState() {
    if (indicatorActive) {
        // Во время индикации светодиод уже установлен в handleButtonEvent
        return;
    }
    
    // Устанавливаем светодиод в соответствии с состоянием UART
    if (currentState == UartState::ENABLED) {
        // UART включен - желтый светодиод (светится постоянно)
        Led_Set(&statusLed, GPIO_PIN_SET);
    } else {
        // UART выключен - светодиод выключен
        Led_Set(&statusLed, GPIO_PIN_RESET);
    }
}

void UartControl::enableUart() {
    currentState = UartState::ENABLED;
    setLedState();
}

void UartControl::disableUart() {
    currentState = UartState::DISABLED;
    setLedState();
}

void UartControl::toggleUart() {
    if (currentState == UartState::ENABLED) {
        disableUart();
    } else {
        enableUart();
    }
}
