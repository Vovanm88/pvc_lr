#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_DEBOUNCE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DEBOUNCE
} button_internal_state_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t debounce_ms;
    uint32_t long_press_ms;
    /* internal */
    button_internal_state_t state;
    uint32_t last_change_tick;
    uint32_t press_tick;
    uint8_t event_ready;
    uint8_t last_event_is_long;
} Button_t;

void Button_Init(Button_t *b, GPIO_TypeDef *port, uint16_t pin, uint32_t debounce_ms, uint32_t long_press_ms);
void Button_Poll(Button_t *b);
int Button_EventReady(Button_t *b);
uint8_t Button_EventIsLong(Button_t *b);
void Button_EventClear(Button_t *b);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H */
