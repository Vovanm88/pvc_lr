#ifndef TASK_HPP
#define TASK_HPP

#include <stdint.h>

enum class TaskState {
    READY,
    BLOCKED,
    SLEEPING
};

class Task {
public:
    Task(uint8_t priority);
    virtual ~Task() = default;
    
    // Виртуальные методы для переопределения
    virtual void onInit() {}
    virtual void update() = 0;
    virtual void onSuspend() {}
    
    // Методы управления состоянием
    void delay(uint32_t ms);
    void block();
    void unblock();
    void sleep(uint32_t ms);
    
    // Геттеры
    uint8_t getPriority() const { return priority; }
    TaskState getState() const { return state; }
    uint32_t getWakeTime() const { return wakeTime; }
    
    // Проверка готовности к выполнению
    bool isReady() const;
    
protected:
    uint8_t priority;      // 0-255, меньше = выше приоритет
    TaskState state;
    uint32_t wakeTime;      // Время пробуждения для SLEEPING задач
    
    friend class Scheduler;
};

#endif // TASK_HPP
