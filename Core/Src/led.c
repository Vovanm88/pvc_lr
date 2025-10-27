#include "led.h"

void Led_Init(Led_t *led, GPIO_TypeDef *port, uint16_t pin)
{
    led->port = port;
    led->pin = pin;
}

void Led_Set(Led_t *led, GPIO_PinState state)
{
    HAL_GPIO_WritePin(led->port, led->pin, state);
}

void Led_Toggle(Led_t *led)
{
    HAL_GPIO_TogglePin(led->port, led->pin);
}
