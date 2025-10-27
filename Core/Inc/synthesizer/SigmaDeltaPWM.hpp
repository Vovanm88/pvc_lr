 #ifndef SIGMA_DELTA_PWM_HPP
#define SIGMA_DELTA_PWM_HPP

#include <stdint.h>
#include <stdbool.h>
#include "tim.h"

// Класс для сигма-дельта модуляции и PWM
// Преобразует аудиосигнал в битовый поток для PWM
class SigmaDeltaPWM {
public:
    static SigmaDeltaPWM& getInstance();
    
    // Инициализация
    bool init(uint32_t sampleRate = 44100, uint32_t pwmFreq = 1000000);
    
    // Добавление сэмпла для обработки
    void pushSample(float sample); // sample в диапазоне [-1.0, 1.0]
    
    // Обновление PWM (вызывается из высокочастотного таймера)
    void updatePWM(); // Вызывается на частоте pwmFreq
    
    // Управление
    void start();
    void stop();
    bool isRunning() const { return running; }
    
    // Настройки
    void setSampleRate(uint32_t rate);
    void setPWMFreq(uint32_t freq);
    void setVolume(float volume); // 0.0 - 1.0
    
private:
    SigmaDeltaPWM() : running(false), pwmFreq(0), sampleRate(0), 
                     phaseAccumulator(0), error(0.0f), volume(1.0f), 
                     currentSample(0.0f) {}
    ~SigmaDeltaPWM() = default;
    SigmaDeltaPWM(const SigmaDeltaPWM&) = delete;
    SigmaDeltaPWM& operator=(const SigmaDeltaPWM&) = delete;
    
    bool running;
    uint32_t pwmFreq;
    uint32_t sampleRate;
    
    // Сигма-дельта модулятор
    uint32_t phaseAccumulator;
    float error;
    float currentSample;
    float volume;
    
    // Буфер для сглаживания переходов между семплами
    float sampleBuffer[2];
    uint8_t bufferIndex;
    
    void updateTimerSettings();
    float interpolateSample();
};

#endif // SIGMA_DELTA_PWM_HPP

