#include "scheduler/Task.hpp"
#include "scheduler/Scheduler.hpp"

Task::Task(uint8_t priority) 
    : priority(priority), state(TaskState::READY), wakeTime(0) {
}

void Task::delay(uint32_t ms) {
    Scheduler::getInstance().delayTask(this, ms);
}

void Task::block() {
    state = TaskState::BLOCKED;
}

void Task::unblock() {
    if (state == TaskState::BLOCKED) {
        state = TaskState::READY;
    }
}

void Task::sleep(uint32_t ms) {
    Scheduler::getInstance().sleepTask(this, ms);
}

bool Task::isReady() const {
    if (state == TaskState::READY) {
        return true;
    }
    
    if (state == TaskState::SLEEPING) {
        return Scheduler::getInstance().getCurrentTime() >= wakeTime;
    }
    
    return false;
}
