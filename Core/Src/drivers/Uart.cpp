#include "drivers/Uart.hpp"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>

// Внешние переменные из HAL
extern UART_HandleTypeDef huart6;

Uart& Uart::getInstance() {
    static Uart instance;
    return instance;
}

bool Uart::init() {
    txInProgress = false;
    lastTxTime = 0;
    txBuffer.clear();
    rxBuffer.clear();
    
    return true;
}

bool Uart::send(uint8_t data) {
    if (txBuffer.isFull()) {
        return false;
    }
    
    txBuffer.push(data);
    
    // Не пытаемся отправить сразу - пусть process() управляет этим
    return true;
}

bool Uart::send(const uint8_t* data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        if (!send(data[i])) {
            return false;
        }
    }
    return true;
}

bool Uart::send(const char* str) {
    while (*str) {
        if (!send(static_cast<uint8_t>(*str))) {
            return false;
        }
        str++;
    }
    return true;
}

void Uart::printf(const char* format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (len > 0) {
        send(reinterpret_cast<const uint8_t*>(buffer), len);
    }
}

bool Uart::receive(uint8_t& data) {
    return rxBuffer.pop(data);
}

bool Uart::receive(uint8_t* data, uint16_t& size) {
    uint16_t received = 0;
    uint8_t byte;
    
    while (received < size && rxBuffer.pop(byte)) {
        data[received] = byte;
        received++;
    }
    
    size = received;
    return received > 0;
}

uint16_t Uart::available() const {
    return rxBuffer.getCount();
}

void Uart::flush() {
    rxBuffer.clear();
}

bool Uart::isTxBusy() const {
    return txInProgress;
}

bool Uart::isRxDataAvailable() const {
    return !rxBuffer.isEmpty();
}

void Uart::onReceiveISR() {
    // Данные уже получены HAL'ом, добавляем их в буфер
    // HAL автоматически помещает данные в huart6.pRxBuffPtr
    if (huart6.pRxBuffPtr != nullptr) {
        uint8_t data = *huart6.pRxBuffPtr;
        rxBuffer.push(data);
    }
}

bool Uart::startTransmission() {
    if (txBuffer.isEmpty()) {
        return false;
    }
    
    // Проверяем, что UART действительно готов к передаче
    if (huart6.gState != HAL_UART_STATE_READY) {
        return false;
    }
    
    // Проверяем, что предыдущая передача завершена
    if (huart6.TxXferCount != 0) {
        return false;
    }
    
    // Извлекаем символ из буфера
    uint8_t data;
    if (txBuffer.pop(data)) {
        // Неблокирующая отправка
        HAL_UART_Transmit(&huart6, &data, 1, 0);
        // Считаем, что символ отправлен
        return true;
    }
    return false;
}

void Uart::processTxBuffer() {
    if (txBuffer.isEmpty()) {
        txInProgress = false;
        return;
    }
    
    uint8_t data;
    if (txBuffer.pop(data)) {
        HAL_UART_Transmit_IT(&huart6, &data, 1);
    } else {
        txInProgress = false;
    }
}

void Uart::process() {
    // Проверяем, что буфер не пуст
    if (!txBuffer.isEmpty()) {
        // Добавляем задержку между попытками (50 мс)
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastTxTime >= 2) {
            if (startTransmission()) {
                // Символ отправлен успешно
                lastTxTime = currentTime;
            }
            // Если не удалось отправить, просто ждем следующего цикла
        }
    }
}
