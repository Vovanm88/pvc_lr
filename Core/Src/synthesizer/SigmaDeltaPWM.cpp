#include "synthesizer/SigmaDeltaPWM.hpp"
#include "drivers/Uart.hpp"
#include <string.h>

extern TIM_HandleTypeDef htim1;

SigmaDeltaPWM& SigmaDeltaPWM::getInstance() {
    static SigmaDeltaPWM instance;
    return instance;
}

bool SigmaDeltaPWM::init(uint32_t sampleRate, uint32_t pwmFreq) {
    this->sampleRate = sampleRate;
    this->pwmFreq = pwmFreq;
    this->running = false;
    this->phaseAccumulator = 0;
    this->error = 0.0f;
    this->currentSample = 0.0f;
    this->volume = 1.0f;
    this->bufferIndex = 0;
    
    // Инициализация буфера
    sampleBuffer[0] = 0.0f;
    sampleBuffer[1] = 0.0f;
    
    // Настройка таймера на высокую частоту PWM
    updateTimerSettings();
    
    Uart::getInstance().printf("SigmaDeltaPWM initialized: sampleRate=%lu, pwmFreq=%lu\n",
                              sampleRate, pwmFreq);
    
    return true;
}

void SigmaDeltaPWM::updateTimerSettings() {
    // Останавливаем таймер
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    
    // Рассчитываем параметры для PWM частоты
    // PCLK1 обычно 90MHz для STM32F4
    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    
    // Нам нужна частота обновления pwmFreq (например, 1MHz)
    // ARR должно быть маленьким для максимальной точности
    // Например, ARR = 90 для 1MHz при PCLK = 90MHz
    uint32_t arr = (pclk / pwmFreq) - 1;
    if (arr < 1) arr = 1;
    if (arr > 65535) arr = 65535;
    
    // Prescaler = 0 (деление на 1)
    uint32_t psc = 0;
    
    TIM1->PSC = psc;
    TIM1->ARR = arr;
    TIM1->CCR1 = arr / 2; // Начальная duty cycle 50%
    
    Uart::getInstance().printf("PWM Timer: pclk=%lu, arr=%lu, actual_freq=%lu\n",
                              pclk, arr, pclk / (arr + 1));
}

void SigmaDeltaPWM::start() {
    if (!running) {
        running = true;
        phaseAccumulator = 0;
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        Uart::getInstance().printf("SigmaDeltaPWM started\n");
    }
}

void SigmaDeltaPWM::stop() {
    if (running) {
        running = false;
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        TIM1->CCR1 = 0;
        Uart::getInstance().printf("SigmaDeltaPWM stopped\n");
    }
}

void SigmaDeltaPWM::pushSample(float sample) {
    // Ограничиваем диапазон [-1.0, 1.0]
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    
    // Простое буферизование для сглаживания
    sampleBuffer[bufferIndex] = sample;
    bufferIndex = (bufferIndex + 1) % 2;
}

float SigmaDeltaPWM::interpolateSample() {
    // Линейная интерполяция между двумя последними семплами
    float ratio = (float)(phaseAccumulator % (pwmFreq / sampleRate)) / 
                  (float)(pwmFreq / sampleRate);
    return sampleBuffer[0] + (sampleBuffer[1] - sampleBuffer[0]) * ratio;
}

void SigmaDeltaPWM::updatePWM() {
    if (!running) return;
    
    // Вычисляем текущее значение амплитуды
    float amplitude = interpolateSample();
    
    // Применяем громкость
    amplitude *= volume;
    
    // Конвертируем из [-1, 1] в [0, 1]
    float normalized = (amplitude + 1.0f) / 2.0f;
    
    // Сигма-дельта модуляция
    float temp = normalized + error;
    uint32_t bit = (temp > 0.5f) ? 1 : 0;
    error = temp - (float)bit;
    
    // Обновляем PWM
    uint32_t arr = TIM1->ARR;
    uint32_t dutyCycle = bit ? arr : 0; // 100% или 0% для 1-bit PWM
    
    // Для более гладкого звука можно использовать multi-bit:
    // dutyCycle = (uint32_t)(normalized * arr);
    // Но для чистой сигма-дельта используем только 0/ARR
    
    TIM1->CCR1 = dutyCycle;
    
    // Увеличиваем фазовый аккумулятор для интерполяции
    phaseAccumulator++;
}

void SigmaDeltaPWM::setSampleRate(uint32_t rate) {
    sampleRate = rate;
}

void SigmaDeltaPWM::setPWMFreq(uint32_t freq) {
    pwmFreq = freq;
    updateTimerSettings();
}

void SigmaDeltaPWM::setVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volume = vol;
}

