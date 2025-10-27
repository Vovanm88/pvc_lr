#ifndef BUZZER_HPP
#define BUZZER_HPP

#include <stdint.h>
#include <stdbool.h>

// Структура ноты
struct Note {
    uint16_t frequency;  // Частота в Гц (0 = тишина)
    uint32_t duration;  // Длительность в мс
    uint8_t channel;     // Канал (0-3)
    
    Note(uint16_t freq = 0, uint32_t dur = 0, uint8_t ch = 0) 
        : frequency(freq), duration(dur), channel(ch) {}
};

// Структура мелодии
struct Melody {
    const Note* notes;
    uint16_t length;
    uint16_t tempo;  // Темп в BPM
    
    Melody(const Note* n, uint16_t len, uint16_t t = 120) 
        : notes(n), length(len), tempo(t) {}
};

// Класс для воспроизведения мелодий
class MelodyPlayer {
public:
    MelodyPlayer();
    
    // Управление воспроизведением
    void play(const Melody& melody);
    void stop();
    void pause();
    void resume();
    
    // Состояние
    bool isPlaying() const { return playing; }
    bool isPaused() const { return paused; }
    uint16_t getCurrentNote() const { return currentNote; }
    
    // Обновление (вызывается из задачи)
    void update();
    
private:
    const Melody* currentMelody;
    uint16_t currentNote;
    uint32_t noteStartTime;
    bool playing;
    bool paused;
    
    uint32_t getNoteDuration(uint32_t duration) const;
};

class Buzzer {
public:
    static Buzzer& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление каналами
    void playNote(uint8_t channel, uint16_t frequency, uint8_t volume = 10);
    void stopNote(uint8_t channel);
    void stopAll();
    
    // Управление громкостью
    void setVolume(uint8_t channel, uint8_t volume);
    void setGlobalVolume(uint8_t volume);
    
    // Воспроизведение мелодий
    void playMelody(const Melody& melody);
    void stopMelody();
    bool isPlaying() const;
    
    // Обновление (вызывается из задачи)
    void update();
    
    // Константы
    static constexpr uint8_t MAX_CHANNELS = 4;
    static constexpr uint8_t MAX_VOLUME = 10;
    static constexpr uint8_t MIN_VOLUME = 0;
    
    // Частоты нот (из buzzer.h)
    static constexpr uint16_t NOTE_C4 = 262;
    static constexpr uint16_t NOTE_CS4 = 277;
    static constexpr uint16_t NOTE_D4 = 294;
    static constexpr uint16_t NOTE_DS4 = 311;
    static constexpr uint16_t NOTE_E4 = 330;
    static constexpr uint16_t NOTE_F4 = 349;
    static constexpr uint16_t NOTE_FS4 = 370;
    static constexpr uint16_t NOTE_G4 = 392;
    static constexpr uint16_t NOTE_GS4 = 415;
    static constexpr uint16_t NOTE_A4 = 440;
    static constexpr uint16_t NOTE_AS4 = 466;
    static constexpr uint16_t NOTE_B4 = 494;
    static constexpr uint16_t NOTE_C5 = 523;
    static constexpr uint16_t NOTE_CS5 = 554;
    static constexpr uint16_t NOTE_D5 = 587;
    static constexpr uint16_t NOTE_DS5 = 622;
    static constexpr uint16_t NOTE_E5 = 659;
    static constexpr uint16_t NOTE_F5 = 698;
    static constexpr uint16_t NOTE_FS5 = 740;
    static constexpr uint16_t NOTE_G5 = 784;
    static constexpr uint16_t NOTE_GS5 = 831;
    static constexpr uint16_t NOTE_A5 = 880;
    static constexpr uint16_t NOTE_AS5 = 932;
    static constexpr uint16_t NOTE_B5 = 988;
    static constexpr uint16_t NOTE_C6 = 1047;
    static constexpr uint16_t NOTE_CS6 = 1109;
    static constexpr uint16_t NOTE_D6 = 1175;
    static constexpr uint16_t NOTE_DS6 = 1245;
    static constexpr uint16_t NOTE_E6 = 1319;
    static constexpr uint16_t NOTE_F6 = 1397;
    static constexpr uint16_t NOTE_FS6 = 1480;
    static constexpr uint16_t NOTE_G6 = 1568;
    static constexpr uint16_t NOTE_GS6 = 1661;
    static constexpr uint16_t NOTE_A6 = 1760;
    static constexpr uint16_t NOTE_AS6 = 1865;
    static constexpr uint16_t NOTE_B6 = 1976;
    static constexpr uint16_t NOTE_C7 = 2093;
    static constexpr uint16_t NOTE_CS7 = 2217;
    static constexpr uint16_t NOTE_D7 = 2349;
    static constexpr uint16_t NOTE_DS7 = 2489;
    static constexpr uint16_t NOTE_E7 = 2637;
    static constexpr uint16_t NOTE_F7 = 2794;
    static constexpr uint16_t NOTE_FS7 = 2960;
    static constexpr uint16_t NOTE_G7 = 3136;
    static constexpr uint16_t NOTE_GS7 = 3322;
    static constexpr uint16_t NOTE_A7 = 3520;
    static constexpr uint16_t NOTE_AS7 = 3729;
    static constexpr uint16_t NOTE_B7 = 3951;
    static constexpr uint16_t NOTE_C8 = 4186;
    static constexpr uint16_t NOTE_CS8 = 4435;
    static constexpr uint16_t NOTE_D8 = 4699;
    static constexpr uint16_t NOTE_DS8 = 4978;
    
private:
    Buzzer() = default;
    ~Buzzer() = default;
    Buzzer(const Buzzer&) = delete;
    Buzzer& operator=(const Buzzer&) = delete;
    
    // Структура канала
    struct Channel {
        uint16_t frequency;
        uint8_t volume;
        bool active;
        
        Channel() : frequency(0), volume(0), active(false) {}
    };
    
    // Состояние каналов
    Channel channels[MAX_CHANNELS];
    uint8_t globalVolume;
    MelodyPlayer melodyPlayer;
    
    // Внутренние методы
    void updatePWM();
    uint16_t calculatePWMValue(uint16_t frequency, uint8_t volume);
    void mixChannels();
};

#endif // BUZZER_HPP
