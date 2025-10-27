#include "synthesizer/AudioToBuzzerAdapter.hpp"
#include "drivers/Uart.hpp"
#include "tim.h"
#include <math.h>

// Реализация AudioToBuzzerAdapter
AudioToBuzzerAdapter& AudioToBuzzerAdapter::getInstance() {
    static AudioToBuzzerAdapter instance;
    return instance;
}

bool AudioToBuzzerAdapter::init() {
    // Инициализация голосов
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i] = Voice();
    }
    
    // Инициализация каналов
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channelVolumes[i] = MAX_VOLUME;
    }
    
    masterVolume = MAX_VOLUME;
    
    // Инициализация Buzzer
    buzzer = Buzzer::getInstance();
    buzzer.init();
    
    Uart::getInstance().printf("AudioToBuzzerAdapter initialized\n");
    return true;
}

void AudioToBuzzerAdapter::processAudioSignal(float audioSample) {
    // Этот метод не используется в адаптере, так как мы работаем напрямую с Buzzer
    // Вместо этого используем selectBestVoice() для выбора лучшего голоса
    selectBestVoice();
}

void AudioToBuzzerAdapter::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
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
    voice.waveType = WaveType::SINE;
    voice.phase = 0.0f;
    voice.phaseIncrement = (2.0f * M_PI * voice.frequency) / 44100.0f;
    
    // Применяем настройки ADSR по умолчанию
    voice.adsr = ADSR(50, 100, 7, 200);
    
    Uart::getInstance().printf("AudioToBuzzer: noteOn ch=%d, note=%d, freq=%d, vel=%d, voice=%d\n", 
                              channel, note, voice.frequency, voice.velocity, voiceIndex);
    
    // Обновляем Buzzer
    selectBestVoice();
}

void AudioToBuzzerAdapter::noteOff(uint8_t channel, uint8_t note) {
    uint8_t voiceIndex = findVoice(channel, note);
    if (voiceIndex < MAX_VOICES) {
        voices[voiceIndex].released = true;
        voices[voiceIndex].releaseTime = HAL_GetTick();
        
        Uart::getInstance().printf("AudioToBuzzer: noteOff ch=%d, note=%d, voice=%d\n", 
                                  channel, note, voiceIndex);
    }
    
    // Обновляем Buzzer
    selectBestVoice();
}

void AudioToBuzzerAdapter::allNotesOff() {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i].active = false;
        voices[i].released = false;
    }
    buzzer.stopAll();
}

void AudioToBuzzerAdapter::setWaveType(uint8_t channel, WaveType type) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].waveType = type;
        }
    }
}

void AudioToBuzzerAdapter::setADSR(uint8_t channel, const ADSR& adsr) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr = adsr;
        }
    }
}

void AudioToBuzzerAdapter::setMasterVolume(uint8_t volume) {
    masterVolume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
}

void AudioToBuzzerAdapter::setChannelVolume(uint8_t channel, uint8_t volume) {
    if (channel < MAX_CHANNELS) {
        channelVolumes[channel] = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    }
}

void AudioToBuzzerAdapter::update() {
    // Обновляем все активные голоса
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            updateVoice(voices[i]);
        }
    }
    
    // Выбираем лучший голос для воспроизведения
    selectBestVoice();
}

uint8_t AudioToBuzzerAdapter::getActiveVoices() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) count++;
    }
    return count;
}

bool AudioToBuzzerAdapter::isChannelActive(uint8_t channel) const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            return true;
        }
    }
    return false;
}

uint8_t AudioToBuzzerAdapter::findFreeVoice() const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].active) {
            return i;
        }
    }
    return MAX_VOICES;
}

uint8_t AudioToBuzzerAdapter::findVoice(uint8_t channel, uint8_t note) const {
    uint16_t frequency = midiToFrequency(note);
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel && voices[i].frequency == frequency) {
            return i;
        }
    }
    return MAX_VOICES;
}

uint16_t AudioToBuzzerAdapter::midiToFrequency(uint8_t note) const {
    if (note == 0) return 0;
    float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    return (uint16_t)frequency;
}

float AudioToBuzzerAdapter::calculateADSRVolume(const Voice& voice) const {
    uint32_t currentTime = HAL_GetTick();
    uint32_t elapsed = currentTime - voice.startTime;
    
    float baseVolume = (float)voice.velocity / 127.0f;
    float adsrVolume = baseVolume;
    
    if (voice.released) {
        // Фаза Release
        uint32_t releaseElapsed = currentTime - voice.releaseTime;
        if (releaseElapsed >= voice.adsr.release) {
            return 0.0f;
        }
        adsrVolume = baseVolume * ((float)voice.adsr.sustain / 10.0f) * 
                     (1.0f - (float)releaseElapsed / voice.adsr.release);
    } else {
        // Фазы Attack, Decay, Sustain
        if (elapsed < voice.adsr.attack) {
            adsrVolume = baseVolume * ((float)elapsed / voice.adsr.attack);
        } else if (elapsed < voice.adsr.attack + voice.adsr.decay) {
            uint32_t decayElapsed = elapsed - voice.adsr.attack;
            float sustainLevel = (float)voice.adsr.sustain / 10.0f;
            adsrVolume = baseVolume - (baseVolume - sustainLevel) * 
                        ((float)decayElapsed / voice.adsr.decay);
        } else {
            adsrVolume = baseVolume * ((float)voice.adsr.sustain / 10.0f);
        }
    }
    
    return adsrVolume;
}

void AudioToBuzzerAdapter::updateVoice(Voice& voice) {
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

void AudioToBuzzerAdapter::selectBestVoice() {
    // Находим самый громкий активный голос
    uint16_t bestFreq = 0;
    uint8_t bestVolume = 0;
    uint8_t bestChannel = 0;
    float maxVolume = 0.0f;
    
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            float adsrVolume = calculateADSRVolume(voices[i]);
            if (adsrVolume > maxVolume) {
                maxVolume = adsrVolume;
                bestFreq = voices[i].frequency;
                bestVolume = (uint8_t)(adsrVolume * MAX_VOLUME);
                bestChannel = voices[i].channel;
            }
        }
    }
    
    // Воспроизводим лучший голос
    if (bestFreq > 0 && bestVolume > 0) {
        buzzer.playNote(0, bestFreq, bestVolume);
        Uart::getInstance().printf("Best voice: freq=%d, vol=%d, ch=%d\n", 
                                  bestFreq, bestVolume, bestChannel);
    } else {
        buzzer.stopAll();
    }
}

// Генерация волн (упрощенная версия для демонстрации)
float AudioToBuzzerAdapter::generateSine(float phase) {
    return sinf(phase);
}

float AudioToBuzzerAdapter::generateSquare(float phase) {
    return (sinf(phase) >= 0.0f) ? 1.0f : -1.0f;
}

float AudioToBuzzerAdapter::generateSawtooth(float phase) {
    while (phase < 0.0f) phase += 2.0f * M_PI;
    while (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    return 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
}

float AudioToBuzzerAdapter::generateTriangle(float phase) {
    while (phase < 0.0f) phase += 2.0f * M_PI;
    while (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    
    if (phase < M_PI) {
        return 2.0f * (phase / M_PI) - 1.0f;
    } else {
        return 3.0f - 2.0f * (phase / M_PI);
    }
}

float AudioToBuzzerAdapter::generateNoise() {
    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

float AudioToBuzzerAdapter::generateWave(WaveType type, float phase) {
    switch (type) {
        case WaveType::SINE:
            return generateSine(phase);
        case WaveType::SQUARE:
            return generateSquare(phase);
        case WaveType::SAWTOOTH:
            return generateSawtooth(phase);
        case WaveType::TRIANGLE:
            return generateTriangle(phase);
        case WaveType::NOISE:
            return generateNoise();
        default:
            return generateSine(phase);
    }
}
