#ifndef LED_H
#define LED_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} Led_t;

/* Инициализация (GPIO должен быть настроен как OUTPUT заранее или в MX_GPIO_Init) */
void Led_Init(Led_t *led, GPIO_TypeDef *port, uint16_t pin);
void Led_Set(Led_t *led, GPIO_PinState state);
void Led_Toggle(Led_t *led);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
