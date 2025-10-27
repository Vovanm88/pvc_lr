#include "synthesizer/WaveSynthesizer.hpp"
#include "drivers/Uart.hpp"
#include "tim.h"
#include <math.h>

// Внешние переменные из HAL
extern TIM_HandleTypeDef htim1;

// Реализация WaveSynthesizer
WaveSynthesizer& WaveSynthesizer::getInstance() {
    static WaveSynthesizer instance;
    return instance;
}

bool WaveSynthesizer::init() {
    // Инициализация голосов
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i] = Voice();
    }
    
    // Инициализация каналов
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channelVolumes[i] = MAX_VOLUME;
    }
    
    masterVolume = MAX_VOLUME;
    
    // Инициализация генератора волн и микшера
    waveGen = WaveGenerator::getInstance();
    mixer = VoiceMixer::getInstance();
    
    Uart::getInstance().printf("WaveSynthesizer initialized\n");
    return true;
}

void WaveSynthesizer::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel >= MAX_CHANNELS || note > 127) return;
    
    // Ищем свободный голос
    uint8_t voiceIndex = findFreeVoice();
    if (voiceIndex >= MAX_VOICES) {
        // Если нет свободных голосов, освобождаем самый старый
        voiceIndex = 0;
        uint32_t oldestTime = voices[0].startTime;
        for (uint8_t i = 1; i < MAX_VOICES; i++) {
            if (voices[i].startTime < oldestTime) {
                oldestTime = voices[i].startTime;
                voiceIndex = i;
            }
        }
        Uart::getInstance().printf("No free voices, using oldest voice %d\n", voiceIndex);
    }
    
    // Настраиваем голос
    Voice& voice = voices[voiceIndex];
    voice.frequency = midiToFrequency(note);
    voice.velocity = (velocity > MAX_VELOCITY) ? MAX_VELOCITY : velocity;
    voice.channel = channel;
    voice.startTime = HAL_GetTick();
    voice.releaseTime = 0;
    voice.active = true;
    voice.released = false;
    voice.waveType = WaveType::SINE; // По умолчанию синусоида
    voice.phase = 0.0f;
    voice.phaseIncrement = (2.0f * M_PI * voice.frequency) / SAMPLE_RATE;
    
    // Применяем настройки ADSR по умолчанию
    voice.adsr = ADSR(50, 100, 7, 200);
    
    Uart::getInstance().printf("WaveSynthesizer: noteOn ch=%d, note=%d, freq=%d, vel=%d, voice=%d\n", 
                              channel, note, voice.frequency, voice.velocity, voiceIndex);
}

void WaveSynthesizer::noteOff(uint8_t channel, uint8_t note) {
    uint8_t voiceIndex = findVoice(channel, note);
    if (voiceIndex < MAX_VOICES) {
        voices[voiceIndex].released = true;
        voices[voiceIndex].releaseTime = HAL_GetTick();
        
        Uart::getInstance().printf("WaveSynthesizer: noteOff ch=%d, note=%d, voice=%d\n", 
                                  channel, note, voiceIndex);
    }
}

void WaveSynthesizer::allNotesOff() {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i].active = false;
        voices[i].released = false;
    }
}

void WaveSynthesizer::setWaveType(uint8_t channel, WaveType type) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].waveType = type;
        }
    }
}

void WaveSynthesizer::setWaveType(uint8_t voice, WaveType type) {
    if (voice < MAX_VOICES) {
        voices[voice].waveType = type;
    }
}

void WaveSynthesizer::setADSR(uint8_t channel, const ADSR& adsr) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr = adsr;
        }
    }
}

void WaveSynthesizer::setAttack(uint8_t channel, uint16_t attack) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.attack = attack;
        }
    }
}

void WaveSynthesizer::setDecay(uint8_t channel, uint16_t decay) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.decay = decay;
        }
    }
}

void WaveSynthesizer::setSustain(uint8_t channel, uint8_t sustain) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.sustain = (sustain > MAX_VOLUME) ? MAX_VOLUME : sustain;
        }
    }
}

void WaveSynthesizer::setRelease(uint8_t channel, uint16_t release) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.release = release;
        }
    }
}

void WaveSynthesizer::setMasterVolume(uint8_t volume) {
    masterVolume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
}

void WaveSynthesizer::setChannelVolume(uint8_t channel, uint8_t volume) {
    if (channel < MAX_CHANNELS) {
        channelVolumes[channel] = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    }
}

float WaveSynthesizer::generateSample() {
    // Генерируем микшированный семпл
    float sample = mixer.mixVoices(voices, MAX_VOICES);
    
    // Применяем мастер-громкость
    sample *= ((float)masterVolume / MAX_VOLUME);
    
    return sample;
}

void WaveSynthesizer::update() {
    // Обновляем все активные голоса
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            updateVoice(voices[i]);
        }
    }
}

uint8_t WaveSynthesizer::getActiveVoices() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) count++;
    }
    return count;
}

bool WaveSynthesizer::isChannelActive(uint8_t channel) const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            return true;
        }
    }
    return false;
}

uint8_t WaveSynthesizer::findFreeVoice() const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].active) {
            return i;
        }
    }
    return MAX_VOICES; // Нет свободных голосов
}

uint8_t WaveSynthesizer::findVoice(uint8_t channel, uint8_t note) const {
    uint16_t frequency = midiToFrequency(note);
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel && voices[i].frequency == frequency) {
            return i;
        }
    }
    return MAX_VOICES; // Голос не найден
}

uint16_t WaveSynthesizer::midiToFrequency(uint8_t note) const {
    // A4 = 440 Hz, MIDI note 69
    if (note == 0) return 0;
    
    // Формула: f = 440 * 2^((note - 69) / 12)
    float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    return (uint16_t)frequency;
}

void WaveSynthesizer::updateVoice(Voice& voice) {
    // Обновляем фазу
    voice.phase += voice.phaseIncrement;
    if (voice.phase >= 2.0f * M_PI) {
        voice.phase -= 2.0f * M_PI;
    }
    
    // Проверяем, нужно ли отключить голос
    if (voice.released) {
        uint32_t currentTime = HAL_GetTick();
        uint32_t releaseElapsed = currentTime - voice.releaseTime;
        if (releaseElapsed >= voice.adsr.release) {
            voice.active = false;
        }
    }
}
