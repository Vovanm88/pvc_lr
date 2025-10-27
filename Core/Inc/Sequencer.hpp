#ifndef SEQUENCER_HPP
#define SEQUENCER_HPP

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include "drivers/Synthesizer.hpp"

// Типы дорожек
enum class TrackType {
    DRUM,
    PIANO
};

// Структура для одного такта
struct Beat {
    bool active;           // Для барабанов - есть ли удар
    uint8_t note;          // Для пианино (C=0, C#=1, ..., B=11, 255=пусто)
    bool halfTone;         // Полутон для пианино
    
    Beat() : active(false), note(255), halfTone(false) {}
    Beat(bool act, uint8_t n = 255, bool half = false) : active(act), note(n), halfTone(half) {}
};

// Структура для блока из 4 тактов
struct Block {
    Beat beats[4];
    
    Block() {
        for (int i = 0; i < 4; i++) {
            beats[i] = Beat();
        }
    }
    
    // Проверка, есть ли хотя бы один звук в блоке
    bool hasAnySound() const {
        for (int i = 0; i < 4; i++) {
            if (beats[i].active || beats[i].note != 255) {
                return true;
            }
        }
        return false;
    }
};

// Структура для дорожки
struct Track {
    TrackType type;
    DrumPreset drumType;   // Тип барабана (для DRUM)
    std::vector<Block> blocks;
    uint8_t currentBlock;  // Текущий блок для воспроизведения
    
    Track() : type(TrackType::DRUM), drumType(DrumPreset::KICK), currentBlock(0) {}
    
    // Добавить новый блок
    void addBlock() {
        blocks.push_back(Block());
    }
    
    // Получить текущий блок
    Block* getCurrentBlock() {
        if (blocks.empty()) return nullptr;
        if (currentBlock >= blocks.size()) currentBlock = 0;
        return &blocks[currentBlock];
    }
    
    // Перейти к следующему блоку
    void nextBlock() {
        if (blocks.empty()) return;
        currentBlock = (currentBlock + 1) % blocks.size();
    }
    
    // Сбросить позицию
    void reset() {
        currentBlock = 0;
    }
};

// Структура для проекта
struct Project {
    Track tracks[6];       // 4 барабана + 2 пианино
    uint16_t bpm;          // Темп 60-240
    uint8_t volume;        // Громкость 0-10
    bool isPlaying;        // Играет ли сейчас
    uint32_t lastBeatTime; // Время последнего такта
    uint8_t currentBeat;   // Текущий такт (0-3)
    
    Project() : bpm(120), volume(8), isPlaying(false), lastBeatTime(0), currentBeat(0) {
        // Инициализация дорожек
        for (int i = 0; i < 4; i++) {
            tracks[i].type = TrackType::DRUM;
            tracks[i].drumType = static_cast<DrumPreset>(i % 8);
            tracks[i].addBlock(); // Добавляем один пустой блок
        }
        for (int i = 4; i < 6; i++) {
            tracks[i].type = TrackType::PIANO;
            tracks[i].addBlock(); // Добавляем один пустой блок
        }
    }
    
    // Сбросить все позиции
    void reset() {
        for (int i = 0; i < 6; i++) {
            tracks[i].reset();
        }
        currentBeat = 0;
        isPlaying = false;
    }
    
    // Получить время одного такта в миллисекундах (четвертная нота)
    uint32_t getBeatDuration() const {
        return 60000 / (bpm * 4); // 60 секунд / (BPM * 4) для четвертной ноты
    }
};

// Основной класс секвенсора
class Sequencer {
public:
    static Sequencer& getInstance();
    
    // Инициализация
    bool init();
    
    // Управление воспроизведением
    void play();
    void stop();
    void pause();
    bool isPlaying() const { return project.isPlaying; }
    
    // Управление проектом
    Project& getProject() { return project; }
    const Project& getProject() const { return project; }
    
    // Настройки
    void setBPM(uint16_t bpm);
    void setVolume(uint8_t volume);
    uint16_t getBPM() const { return project.bpm; }
    uint8_t getVolume() const { return project.volume; }
    
    // Работа с дорожками
    Track& getTrack(uint8_t trackIndex);
    void addBlockToTrack(uint8_t trackIndex);
    
    // Обновление (вызывается из задачи)
    void update();
    
    // Константы
    static constexpr uint8_t MAX_TRACKS = 6;
    static constexpr uint8_t MAX_BLOCKS_PER_TRACK = 16;
    static constexpr uint16_t MIN_BPM = 60;
    static constexpr uint16_t MAX_BPM = 240;
    static constexpr uint8_t MAX_VOLUME = 10;
    
private:
    Sequencer() = default;
    ~Sequencer() = default;
    Sequencer(const Sequencer&) = delete;
    Sequencer& operator=(const Sequencer&) = delete;
    
    Project project;
    uint32_t lastUpdateTime;
    
    // Внутренние методы
    void playBeat();
    void processDrumTrack(uint8_t trackIndex, const Beat& beat);
    void processPianoTrack(uint8_t trackIndex, const Beat& beat);
    uint8_t getMidiNote(uint8_t note, bool halfTone) const;
};

#endif // SEQUENCER_HPP
