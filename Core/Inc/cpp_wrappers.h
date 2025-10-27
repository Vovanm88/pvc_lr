#ifndef CPP_WRAPPERS_H
#define CPP_WRAPPERS_H

#ifdef __cplusplus
extern "C" {
#endif

// C wrapper functions for C++ classes
// These functions provide C-compatible interfaces to C++ objects

// Scheduler wrapper functions - removed scheduler_tick as it's no longer needed

// Uart wrapper functions  
void uart_on_receive_isr(void);
void uart_process_tx_buffer(void);

#ifdef __cplusplus
}
#endif

#endif // CPP_WRAPPERS_H
