#ifndef SYNTHESIZER_HPP
#define SYNTHESIZER_HPP

#include <stdint.h>
#include <stdbool.h>
#include "Buzzer.hpp"

// Типы волн для синтезатора
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
    uint16_t frequency;
    uint8_t velocity;      // Громкость (0-127)
    uint8_t channel;       // MIDI канал
    uint32_t startTime;    // Время начала ноты
    uint32_t releaseTime;  // Время начала релиза
    bool active;
    bool released;
    ADSR adsr;
    
    Voice() : frequency(0), velocity(0), channel(0), startTime(0), 
              releaseTime(0), active(false), released(false), adsr() {}
};

// Структура для барабанного звука
struct DrumSound {
    uint16_t frequency;
    uint16_t duration;     // Длительность в мс
    uint8_t volume;
    bool isNoise;          // true для шумовых звуков
    
    DrumSound(uint16_t freq = 0, uint16_t dur = 100, uint8_t vol = 10, bool noise = false)
        : frequency(freq), duration(dur), volume(vol), isNoise(noise) {}
};

// Пресеты барабанов
enum class DrumPreset {
    KICK,       // Бас-барабан
    SNARE,      // Малый барабан
    HIHAT,      // Хай-хэт
    CRASH,      // Крэш
    RIDE,       // Райд
    TOM_HIGH,   // Высокий том
    TOM_MID,    // Средний том
    TOM_LOW     // Низкий том
};

// Класс полифонического синтезатора
class Synthesizer {
public:
    static Synthesizer& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление голосами (полифония)
    void noteOn(uint8_t channel, uint8_t note, uint8_t velocity = 64);
    void noteOff(uint8_t channel, uint8_t note);
    void allNotesOff();
    
    // Управление ADSR
    void setADSR(uint8_t channel, const ADSR& adsr);
    void setAttack(uint8_t channel, uint16_t attack);
    void setDecay(uint8_t channel, uint16_t decay);
    void setSustain(uint8_t channel, uint8_t sustain);
    void setRelease(uint8_t channel, uint16_t release);
    
    // Барабанные звуки
    void playDrum(DrumPreset preset, uint8_t velocity = 64);
    void playCustomDrum(const DrumSound& sound);
    
    // Управление громкостью
    void setMasterVolume(uint8_t volume);
    void setChannelVolume(uint8_t channel, uint8_t volume);
    
    // Управление эффектами
    void setReverb(uint8_t level);  // 0-10
    void setChorus(uint8_t level);  // 0-10
    
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
    
    // MIDI ноты (C4 = 60)
    static constexpr uint8_t MIDI_C4 = 60;
    static constexpr uint8_t MIDI_C5 = 72;
    static constexpr uint8_t MIDI_C6 = 84;
    
private:
    Synthesizer() = default;
    ~Synthesizer() = default;
    Synthesizer(const Synthesizer&) = delete;
    Synthesizer& operator=(const Synthesizer&) = delete;
    
    // Голоса синтезатора
    Voice voices[MAX_VOICES];
    uint8_t masterVolume;
    uint8_t channelVolumes[MAX_CHANNELS];
    uint8_t reverbLevel;
    uint8_t chorusLevel;
    
    // Внутренние методы
    uint8_t findFreeVoice() const;
    uint8_t findVoice(uint8_t channel, uint8_t note) const;
    uint16_t midiToFrequency(uint8_t note) const;
    uint8_t calculateVolume(const Voice& voice) const;
    void updateVoice(Voice& voice);
    void mixVoices();
    void generateDrumSound(const DrumSound& sound);
    
    // Барабанные пресеты
    DrumSound getDrumPreset(DrumPreset preset) const;
};

// Класс для управления клавиатурой как пианино
class PianoController {
public:
    static PianoController& getInstance();
    
    // Инициализация
    bool init();
    
    // Обработка нажатий клавиш
    void onKeyPress(uint8_t keyCode);
    void onKeyRelease(uint8_t keyCode);
    
    // Настройки
    void setOctave(int8_t octave);  // -2 до +2
    void setChannel(uint8_t channel);
    void setInstrument(uint8_t instrument);  // 0-127
    
    // Состояние
    int8_t getOctave() const { return currentOctave; }
    uint8_t getChannel() const { return currentChannel; }
    uint8_t getInstrument() const { return currentInstrument; }
    
private:
    PianoController() = default;
    ~PianoController() = default;
    PianoController(const PianoController&) = delete;
    PianoController& operator=(const PianoController&) = delete;
    
    // Маппинг клавиш на ноты
    static constexpr uint8_t KEYBOARD_KEYS = 16;
    static constexpr uint8_t INVALID_NOTE = 255;
    
    // Маппинг клавиш клавиатуры на ноты (0-15 -> C, C#, D, D#, E, F, F#, G, G#, A, A#, B, C, C#, D, D#)
    uint8_t keyToNote[KEYBOARD_KEYS];
    
    // Состояние
    int8_t currentOctave;      // -2 до +2
    uint8_t currentChannel;   // 0-15
    uint8_t currentInstrument; // 0-127
    bool keyStates[KEYBOARD_KEYS];  // Состояние каждой клавиши
    
    // Внутренние методы
    uint8_t getMidiNote(uint8_t keyCode) const;
    void updateKeyMapping();
};

#endif // SYNTHESIZER_HPP
