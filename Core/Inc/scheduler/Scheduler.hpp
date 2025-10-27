#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "Task.hpp"
#include <stdint.h>

class Scheduler {
public:
    static Scheduler& getInstance();
    
    // Управление задачами
    void addTask(Task* task);
    void removeTask(Task* task);
    
    // Основной цикл планировщика
    void run();
    
    // Управление временем (polling-based)
    uint32_t getCurrentTime() const;
    
    // Управление задачами
    void delayTask(Task* task, uint32_t ms);
    void sleepTask(Task* task, uint32_t ms);
    void blockTask(Task* task);
    void unblockTask(Task* task);
    
    // Отладочная информация
    uint8_t getTaskCount() const { return taskCount; }
    void printTaskInfo() const;
    
    // Инициализация
    void init();
    
private:
    Scheduler() = default;
    ~Scheduler() = default;
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    
    static constexpr uint8_t MAX_TASKS = 16;
    Task* tasks[MAX_TASKS];
    uint8_t taskCount;
    
    void sortTasksByPriority();
    Task* findNextReadyTask();
    void updateTaskStates();
};

#endif // SCHEDULER_HPP
