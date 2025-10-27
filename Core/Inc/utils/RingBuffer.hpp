#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <stdint.h>
#include <stdbool.h>

template<uint16_t Size>
class RingBuffer {
public:
    RingBuffer() : head(0), tail(0), count(0) {}
    
    // Добавление данных
    bool push(uint8_t data) {
        if (isFull()) return false;
        
        buffer[head] = data;
        head = (head + 1) % Size;
        count++;
        return true;
    }
    
    // Добавление данных в начало (для возврата символа)
    bool pushFront(uint8_t data) {
        if (isFull()) return false;
        
        tail = (tail - 1 + Size) % Size;
        buffer[tail] = data;
        count++;
        return true;
    }
    
    // Извлечение данных
    bool pop(uint8_t& data) {
        if (isEmpty()) return false;
        
        data = buffer[tail];
        tail = (tail + 1) % Size;
        count--;
        return true;
    }
    
    // Проверка состояния
    bool isEmpty() const { return count == 0; }
    bool isFull() const { return count == Size; }
    uint16_t getCount() const { return count; }
    uint16_t getFreeSpace() const { return Size - count; }
    
    // Очистка буфера
    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
    
    // Просмотр данных без извлечения
    bool peek(uint8_t& data, uint16_t offset = 0) const {
        if (offset >= count) return false;
        
        uint16_t index = (tail + offset) % Size;
        data = buffer[index];
        return true;
    }
    
private:
    uint8_t buffer[Size];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
};

#endif // RINGBUFFER_HPP
