#ifndef SIGMA_DELTA_ADAPTER_HPP
#define SIGMA_DELTA_ADAPTER_HPP

#include <stdint.h>
#include <stdbool.h>
#include "synthesizer/WaveSynthesizer.hpp"
#include "synthesizer/SigmaDeltaPWM.hpp"

// Адаптер для преобразования полифонического синтезатора в сигма-дельта PWM
class SigmaDeltaAdapter {
public:
    static SigmaDeltaAdapter& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление ноты (проксируются в синтезатор)
    void noteOn(uint8_t channel, uint8_t note, uint8_t velocity = 64);
    void noteOff(uint8_t channel, uint8_t note);
    void allNotesOff();
    
    // Управление типом волны
    void setWaveType(uint8_t channel, WaveType type);
    
    // Управление ADSR
    void setADSR(uint8_t channel, const ADSR& adsr);
    
    // Управление громкостью
    void setMasterVolume(uint8_t volume);
    void setChannelVolume(uint8_t channel, uint8_t volume);
    
    // Обновление (вызывается из задачи с частотой sampleRate)
    void update();
    
    // Обновление PWM (вызывается из таймера с частотой PWM)
    void updatePWM();
    
    // Состояние
    uint8_t getActiveVoices() const;
    bool isChannelActive(uint8_t channel) const;
    
private:
    SigmaDeltaAdapter() = default;
    ~SigmaDeltaAdapter() = default;
    SigmaDeltaAdapter(const SigmaDeltaAdapter&) = delete;
    SigmaDeltaAdapter& operator=(const SigmaDeltaAdapter&) = delete;
    
    WaveSynthesizer& synthesizer;
    SigmaDeltaPWM& pwmDriver;
    
    static constexpr uint32_t SAMPLE_RATE = 44100;
    static constexpr uint32_t PWM_FREQ = 1000000; // 1 MHz
};

#endif // SIGMA_DELTA_ADAPTER_HPP

