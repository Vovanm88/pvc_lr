#ifndef UART_HPP
#define UART_HPP

#include <stdint.h>
#include <stdbool.h>
#include "utils/RingBuffer.hpp"

class Uart {
public:
    static Uart& getInstance();
    
    // Инициализация
    bool init();
    
    // Отправка данных
    bool send(uint8_t data);
    bool send(const uint8_t* data, uint16_t size);
    bool send(const char* str);
    void printf(const char* format, ...);
    
    // Прием данных
    bool receive(uint8_t& data);
    bool receive(uint8_t* data, uint16_t& size);
    uint16_t available() const;
    void flush();
    
    // Проверка состояния
    bool isTxBusy() const;
    bool isRxDataAvailable() const;
    
    // Обработчик прерывания (вызывается из HAL)
    void onReceiveISR();
    
    // Обработчик завершения передачи (вызывается из HAL)
    void processTxBuffer();
    
    // Обработка буфера передачи (вызывается из планировщика)
    void process();
    
    // Размеры буферов
    static constexpr uint16_t TX_BUFFER_SIZE = 4096;
    static constexpr uint16_t RX_BUFFER_SIZE = 1024;
    
private:
    Uart() = default;
    ~Uart() = default;
    Uart(const Uart&) = delete;
    Uart& operator=(const Uart&) = delete;
    
    // Кольцевые буферы
    RingBuffer<TX_BUFFER_SIZE> txBuffer;
    RingBuffer<RX_BUFFER_SIZE> rxBuffer;
    
    // Состояние передачи
    bool txInProgress;
    uint32_t lastTxTime;
    
    // Внутренние методы
    bool startTransmission();
};

#endif // UART_HPP
