#include "Sequencer.hpp"
#include "drivers/Synthesizer.hpp"
#include "drivers/Display.hpp"
#include "drivers/Uart.hpp"
#include <algorithm>
#include "stm32f4xx_hal.h"

Sequencer& Sequencer::getInstance() {
    static Sequencer instance;
    return instance;
}

bool Sequencer::init() {
    project = Project();
    lastUpdateTime = 0;
    
    // Добавляем тестовые данные для проверки
    Uart::getInstance().printf("Initializing Sequencer with test data\n");
    
    // Создаем тестовые блоки для первой дорожки (барабаны)
    if (project.tracks[0].blocks.empty()) {
        project.tracks[0].addBlock();
        Block& block = project.tracks[0].blocks[0];
        block.beats[0].active = true;  // Удар на первом такте
        block.beats[2].active = true;  // Удар на третьем такте
        
        // Устанавливаем более слышимый тип барабана
        project.tracks[0].drumType = DrumPreset::SNARE;  // Малый барабан вместо бас-барабана
        
        Uart::getInstance().printf("Added test drum pattern with SNARE\n");
    }
    
    return true;
}

void Sequencer::play() {
    project.isPlaying = true;
    project.lastBeatTime = HAL_GetTick();
    project.currentBeat = 0;
    
    // Сбросить все дорожки
    for (int i = 0; i < MAX_TRACKS; i++) {
        project.tracks[i].reset();
    }
}

void Sequencer::stop() {
    project.isPlaying = false;
    project.currentBeat = 0;
    
    // Остановить все звуки
    Synthesizer::getInstance().allNotesOff();
    
    // Сбросить все дорожки
    for (int i = 0; i < MAX_TRACKS; i++) {
        project.tracks[i].reset();
    }
}

void Sequencer::pause() {
    project.isPlaying = false;
}

void Sequencer::setBPM(uint16_t bpm) {
    project.bpm = std::max(MIN_BPM, std::min(MAX_BPM, bpm));
}

void Sequencer::setVolume(uint8_t volume) {
    project.volume = std::min(MAX_VOLUME, volume);
}

Track& Sequencer::getTrack(uint8_t trackIndex) {
    if (trackIndex >= MAX_TRACKS) {
        static Track dummy;
        return dummy;
    }
    return project.tracks[trackIndex];
}

void Sequencer::addBlockToTrack(uint8_t trackIndex) {
    if (trackIndex >= MAX_TRACKS) return;
    
    Track& track = project.tracks[trackIndex];
    if (track.blocks.size() < MAX_BLOCKS_PER_TRACK) {
        track.addBlock();
    }
}

void Sequencer::update() {
    // Отладочный вывод каждые 500мс
    static uint32_t lastUpdateDebug = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastUpdateDebug >= 500) {
        Uart::getInstance().printf("Sequencer::update called, isPlaying=%d\n", project.isPlaying);
        lastUpdateDebug = currentTime;
    }
    
    if (!project.isPlaying) return;
    
    uint32_t beatDuration = project.getBeatDuration();
    
    // Отладочный вывод каждые 100мс
    static uint32_t lastDebugTime = 0;
    if (currentTime - lastDebugTime >= 100) {
        Uart::getInstance().printf("Sequencer update: playing=%d, beat=%d, duration=%lu, elapsed=%lu\n", 
                                  project.isPlaying, project.currentBeat, beatDuration, 
                                  currentTime - project.lastBeatTime);
        lastDebugTime = currentTime;
    }
    
    // Проверяем, нужно ли сыграть следующий такт
    if (currentTime - project.lastBeatTime >= beatDuration) {
        Uart::getInstance().printf("Time to play beat! Duration=%lu, elapsed=%lu\n", 
                                  beatDuration, currentTime - project.lastBeatTime);
        playBeat();
        project.lastBeatTime = currentTime;
        project.currentBeat = (project.currentBeat + 1) % 4;
        
        // Если закончили блок, переходим к следующему
        if (project.currentBeat == 0) {
            for (int i = 0; i < MAX_TRACKS; i++) {
                project.tracks[i].nextBlock();
            }
        }
    }
}

void Sequencer::playBeat() {
    Uart::getInstance().printf("Playing beat %d\n", project.currentBeat);
    
    // Проходим по всем дорожкам
    for (int trackIndex = 0; trackIndex < MAX_TRACKS; trackIndex++) {
        Track& track = project.tracks[trackIndex];
        Block* currentBlock = track.getCurrentBlock();
        
        if (!currentBlock) {
            Uart::getInstance().printf("Track %d: no current block\n", trackIndex);
            continue;
        }
        
        Beat& beat = currentBlock->beats[project.currentBeat];
        
        if (track.type == TrackType::DRUM) {
            if (beat.active) {
                Uart::getInstance().printf("Track %d: drum beat %d is active\n", trackIndex, project.currentBeat);
            }
            processDrumTrack(trackIndex, beat);
        } else {
            if (beat.note != 255) {
                Uart::getInstance().printf("Track %d: piano note %d on beat %d\n", trackIndex, beat.note, project.currentBeat);
            }
            processPianoTrack(trackIndex, beat);
        }
    }
}

void Sequencer::processDrumTrack(uint8_t trackIndex, const Beat& beat) {
    if (!beat.active) return;
    
    Track& track = project.tracks[trackIndex];
    Synthesizer& synth = Synthesizer::getInstance();
    
    // Играем барабанный звук
    synth.playDrum(track.drumType, project.volume * 12); // Масштабируем громкость
    
    // Отладочный вывод
    Uart::getInstance().printf("Drum track %d: drum type %d, volume %d\n", 
                              trackIndex, (int)track.drumType, project.volume * 12);
}

void Sequencer::processPianoTrack(uint8_t trackIndex, const Beat& beat) {
    if (beat.note == 255) return; // Нет ноты
    
    Synthesizer& synth = Synthesizer::getInstance();
    uint8_t midiNote = getMidiNote(beat.note, beat.halfTone);
    
    // Играем ноту на канале дорожки
    synth.noteOn(trackIndex, midiNote, project.volume * 12);
    
    // Отладочный вывод
    Uart::getInstance().printf("Piano track %d: note %d, midi %d, volume %d\n", 
                              trackIndex, beat.note, midiNote, project.volume * 12);
    
    // Останавливаем ноту через половину длительности такта
    // (это будет обработано в следующем update)
    
    // Автоматически останавливаем ноту через короткое время
    static uint32_t lastNoteTime = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastNoteTime > 100) { // 100мс задержка
        synth.noteOff(trackIndex, midiNote);
        lastNoteTime = currentTime;
    }
}

uint8_t Sequencer::getMidiNote(uint8_t note, bool halfTone) const {
    // C=0, C#=1, D=2, D#=3, E=4, F=5, F#=6, G=7, G#=8, A=9, A#=10, B=11
    // MIDI C4 = 60, C5 = 72, C6 = 84
    uint8_t baseNote = note + (halfTone ? 1 : 0);
    
    // Базовые MIDI ноты для октавы 4 (C4 = 60)
    uint8_t midiNotes[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71}; // C4-B4
    
    if (baseNote >= 12) return 60; // Fallback
    
    return midiNotes[baseNote];
}
