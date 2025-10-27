#include "cpp_wrappers.h"
#include "scheduler/Scheduler.hpp"
#include "drivers/Uart.hpp"

// C wrapper functions for C++ classes
// These functions provide C-compatible interfaces to C++ objects

extern "C" {

// Scheduler wrapper functions - removed scheduler_tick as it's no longer needed

// Uart wrapper functions  
void uart_on_receive_isr(void) {
    Uart::getInstance().onReceiveISR();
}

void uart_process_tx_buffer(void) {
    Uart::getInstance().processTxBuffer();
}

} // extern "C"
