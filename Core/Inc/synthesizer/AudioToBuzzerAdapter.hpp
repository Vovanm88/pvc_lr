#ifndef AUDIO_TO_BUZZER_ADAPTER_HPP
#define AUDIO_TO_BUZZER_ADAPTER_HPP

#include <stdint.h>
#include <stdbool.h>
#include "drivers/Buzzer.hpp"

// Адаптер для преобразования аудиосигналов в команды Buzzer
class AudioToBuzzerAdapter {
public:
    static AudioToBuzzerAdapter& getInstance();
    
    // Инициализация
    bool init();
    
    // Преобразование аудиосигнала в команды Buzzer
    void processAudioSignal(float audioSample);
    
    // Управление голосами (адаптированное для Buzzer)
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
    
    // Обновление
    void update();
    
    // Состояние
    uint8_t getActiveVoices() const;
    bool isChannelActive(uint8_t channel) const;
    
    // Константы
    static constexpr uint8_t MAX_VOICES = 8;
    static constexpr uint8_t MAX_CHANNELS = 16;
    static constexpr uint8_t MAX_VELOCITY = 127;
    static constexpr uint8_t MAX_VOLUME = 10;
    
private:
    AudioToBuzzerAdapter() = default;
    ~AudioToBuzzerAdapter() = default;
    AudioToBuzzerAdapter(const AudioToBuzzerAdapter&) = delete;
    AudioToBuzzerAdapter& operator=(const AudioToBuzzerAdapter&) = delete;
    
    // Структура для голоса
    struct Voice {
        uint16_t frequency;
        uint8_t velocity;
        uint8_t channel;
        uint32_t startTime;
        uint32_t releaseTime;
        bool active;
        bool released;
        ADSR adsr;
        WaveType waveType;
        float phase;
        float phaseIncrement;
        
        Voice() : frequency(0), velocity(0), channel(0), startTime(0), 
                  releaseTime(0), active(false), released(false), adsr(),
                  waveType(WaveType::SINE), phase(0.0f), phaseIncrement(0.0f) {}
    };
    
    // Голоса синтезатора
    Voice voices[MAX_VOICES];
    uint8_t masterVolume;
    uint8_t channelVolumes[MAX_CHANNELS];
    
    // Buzzer для воспроизведения
    Buzzer& buzzer;
    
    // Внутренние методы
    uint8_t findFreeVoice() const;
    uint8_t findVoice(uint8_t channel, uint8_t note) const;
    uint16_t midiToFrequency(uint8_t note) const;
    float calculateADSRVolume(const Voice& voice) const;
    void updateVoice(Voice& voice);
    void selectBestVoice();
    
    // Генерация волн
    float generateSine(float phase);
    float generateSquare(float phase);
    float generateSawtooth(float phase);
    float generateTriangle(float phase);
    float generateNoise();
    float generateWave(WaveType type, float phase);
};

#endif // AUDIO_TO_BUZZER_ADAPTER_HPP
