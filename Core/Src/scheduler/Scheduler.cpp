#include "scheduler/Scheduler.hpp"
#include "drivers/Uart.hpp"
#include <algorithm>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "usart.h"

Scheduler& Scheduler::getInstance() {
    static Scheduler instance;
    return instance;
}

void Scheduler::init() {
    taskCount = 0;
    
    for (uint8_t i = 0; i < MAX_TASKS; i++) {
        tasks[i] = nullptr;
    }
}

void Scheduler::addTask(Task* task) {
    if (taskCount < MAX_TASKS && task != nullptr) {
        tasks[taskCount] = task;
        taskCount++;
        sortTasksByPriority();
    }
}

void Scheduler::removeTask(Task* task) {
    for (uint8_t i = 0; i < taskCount; i++) {
        if (tasks[i] == task) {
            // Сдвигаем оставшиеся задачи
            for (uint8_t j = i; j < taskCount - 1; j++) {
                tasks[j] = tasks[j + 1];
            }
            taskCount--;
            break;
        }
    }
}

void Scheduler::run() {
    static bool initialized = false;
    
    // Инициализация всех задач (только один раз)
    if (!initialized) {
        for (uint8_t i = 0; i < taskCount; i++) {
            tasks[i]->onInit();
        }
        initialized = true;
    }
    
    // Обновляем состояния всех задач
    updateTaskStates();
    
    // Ищем следующую готовую задачу
    Task* nextTask = findNextReadyTask();
    
    // Отладочный вывод каждые 2000мс
    static uint32_t lastSchedulerDebug = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastSchedulerDebug >= 2000) {
        Uart::getInstance().printf("Scheduler::run: taskCount=%d, nextTask=%p\n", 
                                  taskCount, nextTask);
        lastSchedulerDebug = currentTime;
    }
    
    if (nextTask != nullptr) {
        // Отладочный вывод для каждой выполняемой задачи
        static uint32_t lastTaskNameDebug = 0;
        if (currentTime - lastTaskNameDebug >= 1000) {
            Uart::getInstance().printf("Executing task at %p\n", nextTask);
            lastTaskNameDebug = currentTime;
        }
        nextTask->update();
    }
}

uint32_t Scheduler::getCurrentTime() const {
    return HAL_GetTick();
}

void Scheduler::delayTask(Task* task, uint32_t ms) {
    task->state = TaskState::SLEEPING;
    task->wakeTime = getCurrentTime() + ms;
}

void Scheduler::sleepTask(Task* task, uint32_t ms) {
    delayTask(task, ms);
}

void Scheduler::blockTask(Task* task) {
    task->state = TaskState::BLOCKED;
}

void Scheduler::unblockTask(Task* task) {
    if (task->state == TaskState::BLOCKED) {
        task->state = TaskState::READY;
    }
}

void Scheduler::sortTasksByPriority() {
    // Простая сортировка пузырьком по приоритету
    for (uint8_t i = 0; i < taskCount - 1; i++) {
        for (uint8_t j = 0; j < taskCount - i - 1; j++) {
            if (tasks[j]->getPriority() > tasks[j + 1]->getPriority()) {
                Task* temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }
}

Task* Scheduler::findNextReadyTask() {
    static uint8_t lastTaskIndex = 0;
    
    // Начинаем поиск со следующей задачи после последней выполненной
    for (uint8_t i = 0; i < taskCount; i++) {
        uint8_t taskIndex = (lastTaskIndex + 1 + i) % taskCount;
        if (tasks[taskIndex]->isReady()) {
            lastTaskIndex = taskIndex;
            return tasks[taskIndex];
        }
    }
    return nullptr;
}

void Scheduler::updateTaskStates() {
    uint32_t currentTime = getCurrentTime();
    
    for (uint8_t i = 0; i < taskCount; i++) {
        Task* task = tasks[i];
        
        // Проверяем спящие задачи
        if (task->state == TaskState::SLEEPING) {
            if (currentTime >= task->wakeTime) {
                task->state = TaskState::READY;
            }
        }
    }
}
void Scheduler::printTaskInfo() const {
    // Используем printf через UART драйвер
    Uart& uart = Uart::getInstance();
    
    uart.printf("Tasks: %d/%d\n", taskCount, MAX_TASKS);
    
    // Считаем задачи по состояниям
    uint8_t readyCount = 0, blockedCount = 0, sleepingCount = 0;
    for (uint8_t i = 0; i < taskCount; i++) {
        switch (tasks[i]->getState()) {
            case TaskState::READY: readyCount++; break;
            case TaskState::BLOCKED: blockedCount++; break;
            case TaskState::SLEEPING: sleepingCount++; break;
        }
    }
    
    uart.printf("States: R=%d, B=%d, S=%d\n", readyCount, blockedCount, sleepingCount);
}