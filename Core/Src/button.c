#include "button.h"

/* считаем кнопку active-low (нажатие -> 0). При необходимости поменяйте phys_pressed */
static inline uint8_t phys_pressed(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) ? 1 : 0;
}

void Button_Init(Button_t *b, GPIO_TypeDef *port, uint16_t pin, uint32_t debounce_ms, uint32_t long_press_ms)
{
    b->port = port;
    b->pin = pin;
    b->debounce_ms = debounce_ms;
    b->long_press_ms = long_press_ms;
    b->state = BUTTON_STATE_IDLE;
    b->last_change_tick = HAL_GetTick();
    b->press_tick = 0;
    b->event_ready = 0;
    b->last_event_is_long = 0;
}

void Button_Poll(Button_t *b)
{
    uint32_t now = HAL_GetTick();
    uint8_t pressed = phys_pressed(b->port, b->pin);

    switch (b->state) {
    case BUTTON_STATE_IDLE:
        if (pressed) {
            b->state = BUTTON_STATE_DEBOUNCE;
            b->last_change_tick = now;
        }
        break;
    case BUTTON_STATE_DEBOUNCE:
        if ((now - b->last_change_tick) >= b->debounce_ms) {
            if (pressed) {
                b->state = BUTTON_STATE_PRESSED;
                b->press_tick = now;
            } else {
                b->state = BUTTON_STATE_IDLE;
            }
        }
        break;
    case BUTTON_STATE_PRESSED:
        if (!pressed) {
            b->state = BUTTON_STATE_RELEASE_DEBOUNCE;
            b->last_change_tick = now;
        }
        break;
    case BUTTON_STATE_RELEASE_DEBOUNCE:
        if ((now - b->last_change_tick) >= b->debounce_ms) {
            if (!pressed) {
                uint32_t duration = now - b->press_tick;
                b->last_event_is_long = (duration >= b->long_press_ms) ? 1 : 0;
                b->event_ready = 1;
                b->state = BUTTON_STATE_IDLE;
            } else {
                b->state = BUTTON_STATE_PRESSED;
            }
        }
        break;
    default:
        b->state = BUTTON_STATE_IDLE;
        break;
    }
}

int Button_EventReady(Button_t *b) { return b->event_ready; }
uint8_t Button_EventIsLong(Button_t *b) { return b->last_event_is_long; }
void Button_EventClear(Button_t *b) { b->event_ready = 0; }
