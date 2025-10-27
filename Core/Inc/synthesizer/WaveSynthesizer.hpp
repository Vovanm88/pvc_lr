#ifndef WAVE_SYNTHESIZER_HPP
#define WAVE_SYNTHESIZER_HPP

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Константы для синтезатора
#define SAMPLE_RATE 44100
#define MAX_VOICES 8
#define MAX_CHANNELS 16
#define WAVE_TABLE_SIZE 1024
#define MAX_HARMONICS 8

// Типы волн
enum class WaveType {
    SINE,       // Синусоида
    SQUARE,     // Прямоугольная
    SAWTOOTH,   // Пилообразная
    TRIANGLE,   // Треугольная
    NOISE       // Шум
};

// Структура для ADSR огибающей
struct ADSR {
    uint16_t attack;   // Время атаки в мс
    uint16_t decay;    // Время спада в мс
    uint8_t sustain;   // Уровень сустейна (0-10)
    uint16_t release;  // Время релиза в мс
    
    ADSR(uint16_t a = 50, uint16_t d = 100, uint8_t s = 7, uint16_t r = 200)
        : attack(a), decay(d), sustain(s), release(r) {}
};

// Структура для голоса синтезатора
struct Voice {
    uint16_t frequency;        // Частота в Гц
    uint8_t velocity;          // Громкость (0-127)
    uint8_t channel;           // MIDI канал
    uint32_t startTime;        // Время начала ноты
    uint32_t releaseTime;      // Время начала релиза
    bool active;              // Активен ли голос
    bool released;            // В фазе релиза
    ADSR adsr;                // ADSR огибающая
    WaveType waveType;        // Тип волны
    float phase;              // Текущая фаза (0-2π)
    float phaseIncrement;     // Приращение фазы за семпл
    
    Voice() : frequency(0), velocity(0), channel(0), startTime(0), 
              releaseTime(0), active(false), released(false), adsr(),
              waveType(WaveType::SINE), phase(0.0f), phaseIncrement(0.0f) {}
};

// Класс для генерации волн
class WaveGenerator {
public:
    static WaveGenerator& getInstance();
    
    // Генерация различных типов волн
    float generateSine(float phase);
    float generateSquare(float phase);
    float generateSawtooth(float phase);
    float generateTriangle(float phase);
    float generateNoise();
    
    // Генерация волны по типу
    float generateWave(WaveType type, float phase);
    
    // Генерация с гармониками
    float generateWithHarmonics(WaveType type, float phase, uint8_t harmonics);
    
private:
    WaveGenerator() = default;
    ~WaveGenerator() = default;
    WaveGenerator(const WaveGenerator&) = delete;
    WaveGenerator& operator=(const WaveGenerator&) = delete;
    
    // Таблица волн для быстрого доступа
    float waveTable[WAVE_TABLE_SIZE];
    bool tableGenerated;
    
    // Генерация таблицы волн
    void generateWaveTable();
};

// Класс для микширования голосов
class VoiceMixer {
public:
    static VoiceMixer& getInstance();
    
    // Микширование голосов
    float mixVoices(Voice* voices, uint8_t voiceCount);
    
    // Применение ADSR огибающей
    float applyADSR(const Voice& voice, float sample);
    
    // Расчет громкости по ADSR
    float calculateADSRVolume(const Voice& voice);
    
    // Генерация семпла волны
    float generateWaveSample(const Voice& voice);
    
private:
    VoiceMixer() = default;
    ~VoiceMixer() = default;
    VoiceMixer(const VoiceMixer&) = delete;
    VoiceMixer& operator=(const VoiceMixer&) = delete;
};

// Основной класс синтезатора с микшированием волн
class WaveSynthesizer {
public:
    static WaveSynthesizer& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление голосами
    void noteOn(uint8_t channel, uint8_t note, uint8_t velocity = 64);
    void noteOff(uint8_t channel, uint8_t note);
    void allNotesOff();
    
    // Управление типом волны
    void setWaveType(uint8_t channel, WaveType type);
    void setWaveType(uint8_t voice, WaveType type);
    
    // Управление ADSR
    void setADSR(uint8_t channel, const ADSR& adsr);
    void setAttack(uint8_t channel, uint16_t attack);
    void setDecay(uint8_t channel, uint16_t decay);
    void setSustain(uint8_t channel, uint8_t sustain);
    void setRelease(uint8_t channel, uint16_t release);
    
    // Управление громкостью
    void setMasterVolume(uint8_t volume);
    void setChannelVolume(uint8_t channel, uint8_t volume);
    
    // Генерация аудиосигнала
    float generateSample();
    
    // Обновление (вызывается из задачи)
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
    WaveSynthesizer() = default;
    ~WaveSynthesizer() = default;
    WaveSynthesizer(const WaveSynthesizer&) = delete;
    WaveSynthesizer& operator=(const WaveSynthesizer&) = delete;
    
    // Голоса синтезатора
    Voice voices[MAX_VOICES];
    uint8_t masterVolume;
    uint8_t channelVolumes[MAX_CHANNELS];
    
    // Генератор волн и микшер
    WaveGenerator& waveGen;
    VoiceMixer& mixer;
    
    // Внутренние методы
    uint8_t findFreeVoice() const;
    uint8_t findVoice(uint8_t channel, uint8_t note) const;
    uint16_t midiToFrequency(uint8_t note) const;
    void updateVoice(Voice& voice);
};

#endif // WAVE_SYNTHESIZER_HPP
