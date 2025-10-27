#include "drivers/Synthesizer.hpp"
#include "drivers/Uart.hpp"
#include "tim.h"
#include <math.h>

// Внешние переменные из HAL
extern TIM_HandleTypeDef htim1;

// Реализация Synthesizer
Synthesizer& Synthesizer::getInstance() {
    static Synthesizer instance;
    return instance;
}

bool Synthesizer::init() {
    // Инициализация голосов
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i] = Voice();
    }
    
    // Инициализация каналов
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channelVolumes[i] = MAX_VOLUME;
    }
    
    masterVolume = MAX_VOLUME;
    reverbLevel = 0;
    chorusLevel = 0;
    
    // Инициализация Buzzer
    Buzzer::getInstance().init();
    
    return true;
}

void Synthesizer::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (channel >= MAX_CHANNELS || note > 127) return;
    
    // АГРЕССИВНО: Отключаем ВСЕ старые голоса при новой ноте
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            voices[i].active = false;
            Uart::getInstance().printf("Stopping old voice %d for new note\n", i);
        }
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
    
    // Применяем настройки ADSR по умолчанию
    voice.adsr = ADSR(50, 100, 7, 200);
    
    // Отладочный вывод
    Uart::getInstance().printf("MIDI: noteOn ch=%d, note=%d, freq=%d, vel=%d, voice=%d\n", 
                              channel, note, voice.frequency, voice.velocity, voiceIndex);
}

void Synthesizer::noteOff(uint8_t channel, uint8_t note) {
    uint8_t voiceIndex = findVoice(channel, note);
    if (voiceIndex < MAX_VOICES) {
        voices[voiceIndex].released = true;
        voices[voiceIndex].releaseTime = HAL_GetTick();
        
        Uart::getInstance().printf("MIDI: noteOff ch=%d, note=%d, voice=%d\n", 
                                  channel, note, voiceIndex);
    } else {
        Uart::getInstance().printf("MIDI: noteOff ch=%d, note=%d - voice not found\n", 
                                  channel, note);
    }
}

void Synthesizer::allNotesOff() {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        voices[i].active = false;
        voices[i].released = false;
    }
    Buzzer::getInstance().stopAll();
}

void Synthesizer::setADSR(uint8_t channel, const ADSR& adsr) {
    // Применяем ADSR ко всем активным голосам канала
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr = adsr;
        }
    }
}

void Synthesizer::setAttack(uint8_t channel, uint16_t attack) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.attack = attack;
        }
    }
}

void Synthesizer::setDecay(uint8_t channel, uint16_t decay) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.decay = decay;
        }
    }
}

void Synthesizer::setSustain(uint8_t channel, uint8_t sustain) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.sustain = (sustain > MAX_VOLUME) ? MAX_VOLUME : sustain;
        }
    }
}

void Synthesizer::setRelease(uint8_t channel, uint16_t release) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            voices[i].adsr.release = release;
        }
    }
}

void Synthesizer::playDrum(DrumPreset preset, uint8_t velocity) {
    DrumSound sound = getDrumPreset(preset);
    sound.volume = (velocity * MAX_VOLUME) / MAX_VELOCITY;
    
    // Отладочный вывод
    Uart::getInstance().printf("Drum: preset %d, freq %d, vol %d, noise %d\n", 
                              (int)preset, sound.frequency, sound.volume, sound.isNoise);
    
    generateDrumSound(sound);
}

void Synthesizer::playCustomDrum(const DrumSound& sound) {
    generateDrumSound(sound);
}

void Synthesizer::setMasterVolume(uint8_t volume) {
    masterVolume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
}

void Synthesizer::setChannelVolume(uint8_t channel, uint8_t volume) {
    if (channel < MAX_CHANNELS) {
        channelVolumes[channel] = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    }
}

void Synthesizer::setReverb(uint8_t level) {
    reverbLevel = (level > MAX_VOLUME) ? MAX_VOLUME : level;
}

void Synthesizer::setChorus(uint8_t level) {
    chorusLevel = (level > MAX_VOLUME) ? MAX_VOLUME : level;
}

void Synthesizer::update() {
    // Обновляем все активные голоса
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            updateVoice(voices[i]);
        }
    }
    
    // Микшируем голоса
    mixVoices();
}

uint8_t Synthesizer::getActiveVoices() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) count++;
    }
    return count;
}

bool Synthesizer::isChannelActive(uint8_t channel) const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel) {
            return true;
        }
    }
    return false;
}

uint8_t Synthesizer::findFreeVoice() const {
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (!voices[i].active) {
            return i;
        }
    }
    return MAX_VOICES; // Нет свободных голосов
}

uint8_t Synthesizer::findVoice(uint8_t channel, uint8_t note) const {
    uint16_t frequency = midiToFrequency(note);
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].channel == channel && voices[i].frequency == frequency) {
            return i;
        }
    }
    return MAX_VOICES; // Голос не найден
}

uint16_t Synthesizer::midiToFrequency(uint8_t note) const {
    // A4 = 440 Hz, MIDI note 69
    if (note == 0) return 0;
    
    // Формула: f = 440 * 2^((note - 69) / 12)
    float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    return (uint16_t)frequency;
}

uint8_t Synthesizer::calculateVolume(const Voice& voice) const {
    uint32_t currentTime = HAL_GetTick();
    uint32_t elapsed = currentTime - voice.startTime;
    
    uint8_t baseVolume = (voice.velocity * MAX_VOLUME) / MAX_VELOCITY;
    uint8_t adsrVolume = baseVolume;
    
    if (voice.released) {
        // Фаза Release
        uint32_t releaseElapsed = currentTime - voice.releaseTime;
        if (releaseElapsed >= voice.adsr.release) {
            return 0; // Голос должен быть отключен
        }
        // Линейное затухание в фазе Release
        adsrVolume = (voice.adsr.sustain * (voice.adsr.release - releaseElapsed)) / voice.adsr.release;
    } else {
        // Фазы Attack, Decay, Sustain
        if (elapsed < voice.adsr.attack) {
            // Фаза Attack
            adsrVolume = (baseVolume * elapsed) / voice.adsr.attack;
        } else if (elapsed < voice.adsr.attack + voice.adsr.decay) {
            // Фаза Decay
            uint32_t decayElapsed = elapsed - voice.adsr.attack;
            adsrVolume = baseVolume - ((baseVolume - voice.adsr.sustain) * decayElapsed) / voice.adsr.decay;
        } else {
            // Фаза Sustain
            adsrVolume = voice.adsr.sustain;
        }
    }
    
    // Применяем громкость канала и мастер-громкость
    adsrVolume = (adsrVolume * channelVolumes[voice.channel] * masterVolume) / (MAX_VOLUME * MAX_VOLUME);
    
    return adsrVolume;
}

void Synthesizer::updateVoice(Voice& voice) {
    uint8_t volume = calculateVolume(voice);
    
    if (volume == 0 && voice.released) {
        // Голос завершил фазу Release
        voice.active = false;
        return;
    }
    
    // Не обновляем Buzzer здесь - это делается в mixVoices()
    // Просто сохраняем текущую громкость для микширования
}

void Synthesizer::mixVoices() {
    // Полифоническое микширование - находим самый громкий активный голос
    uint16_t mixedFreq = 0;
    uint8_t mixedVolume = 0;
    uint8_t activeVoices = 0;
    uint8_t loudestVoice = 0;
    uint8_t maxVolume = 0;
    
    // Находим самый громкий активный голос
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            uint8_t volume = calculateVolume(voices[i]);
            if (volume > 0) {
                activeVoices++;
                if (volume > maxVolume) {
                    maxVolume = volume;
                    loudestVoice = i;
                    mixedFreq = voices[i].frequency;
                    mixedVolume = volume;
                }
            }
        }
    }
    
    // Отладочный вывод
    if (activeVoices > 0) {
        Uart::getInstance().printf("mixVoices: %d active voices, loudest=%d, freq=%d, vol=%d\n", 
                                  activeVoices, loudestVoice, mixedFreq, mixedVolume);
    }
    
    // Обновляем Buzzer только если есть активные голоса
    if (activeVoices > 0 && mixedFreq > 0 && mixedVolume > 0) {
        Buzzer::getInstance().playNote(0, mixedFreq, mixedVolume);
    } else {
        Buzzer::getInstance().stopAll();
    }
}

void Synthesizer::generateDrumSound(const DrumSound& sound) {
    Uart::getInstance().printf("generateDrumSound: freq=%d, vol=%d, noise=%d\n", 
                              sound.frequency, sound.volume, sound.isNoise);
    
    // АГРЕССИВНО: Отключаем ВСЕ голоса для барабанного звука
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active) {
            voices[i].active = false;
            Uart::getInstance().printf("Stopping voice %d for drum sound\n", i);
        }
    }
    
    // ПРИНУДИТЕЛЬНО играем барабанный звук
    Buzzer::getInstance().playNote(0, sound.frequency, sound.volume);
    Uart::getInstance().printf("FORCED Drum sound: freq=%d, vol=%d\n", sound.frequency, sound.volume);
}

DrumSound Synthesizer::getDrumPreset(DrumPreset preset) const {
    switch (preset) {
        case DrumPreset::KICK:
            return DrumSound(60, 200, 10, false);  // Низкая частота, длинный звук
        case DrumPreset::SNARE:
            return DrumSound(200, 100, 8, true);   // Средняя частота, шум
        case DrumPreset::HIHAT:
            return DrumSound(8000, 50, 6, true); // Высокая частота, шум, короткий
        case DrumPreset::CRASH:
            return DrumSound(5000, 300, 9, true); // Высокая частота, шум, длинный
        case DrumPreset::RIDE:
            return DrumSound(3000, 150, 7, true); // Средняя частота, шум
        case DrumPreset::TOM_HIGH:
            return DrumSound(400, 120, 7, false); // Высокий том
        case DrumPreset::TOM_MID:
            return DrumSound(200, 150, 8, false); // Средний том
        case DrumPreset::TOM_LOW:
            return DrumSound(100, 200, 9, false); // Низкий том
        default:
            return DrumSound(440, 100, 5, false);
    }
}

// Реализация PianoController
PianoController& PianoController::getInstance() {
    static PianoController instance;
    return instance;
}

bool PianoController::init() {
    currentOctave = 0;
    currentChannel = 0;
    currentInstrument = 0;
    
    // Инициализация состояний клавиш
    for (uint8_t i = 0; i < KEYBOARD_KEYS; i++) {
        keyStates[i] = false;
    }
    
    // Инициализация маппинга клавиш
    updateKeyMapping();
    
    return true;
}

void PianoController::onKeyPress(uint8_t keyCode) {
    if (keyCode >= KEYBOARD_KEYS) return;
    
    if (!keyStates[keyCode]) {
        keyStates[keyCode] = true;
        uint8_t midiNote = getMidiNote(keyCode);
        if (midiNote != INVALID_NOTE) {
            Synthesizer::getInstance().noteOn(currentChannel, midiNote, 64);
            
            // Отладочный вывод
            Uart::getInstance().printf("Piano: key %d -> MIDI note %d on channel %d\n", 
                                      keyCode, midiNote, currentChannel);
        }
    }
}

void PianoController::onKeyRelease(uint8_t keyCode) {
    if (keyCode >= KEYBOARD_KEYS) return;
    
    if (keyStates[keyCode]) {
        keyStates[keyCode] = false;
        uint8_t midiNote = getMidiNote(keyCode);
        if (midiNote != INVALID_NOTE) {
            Synthesizer::getInstance().noteOff(currentChannel, midiNote);
        }
    }
}

void PianoController::setOctave(int8_t octave) {
    if (octave >= -2 && octave <= 2) {
        currentOctave = octave;
        updateKeyMapping();
    }
}

void PianoController::setChannel(uint8_t channel) {
    if (channel < Synthesizer::MAX_CHANNELS) {
        currentChannel = channel;
    }
}

void PianoController::setInstrument(uint8_t instrument) {
    currentInstrument = instrument;
}

uint8_t PianoController::getMidiNote(uint8_t keyCode) const {
    if (keyCode >= KEYBOARD_KEYS) return INVALID_NOTE;
    
    uint8_t baseNote = keyToNote[keyCode];
    if (baseNote == INVALID_NOTE) return INVALID_NOTE;
    
    // Применяем октаву
    int8_t midiNote = baseNote + (currentOctave + 4) * 12;
    
    // Проверяем диапазон MIDI (0-127)
    if (midiNote < 0 || midiNote > 127) return INVALID_NOTE;
    
    return (uint8_t)midiNote;
}

void PianoController::updateKeyMapping() {
    // Маппинг клавиш 0-15 на ноты C4-C5 (60-72)
    // Клавиши: 0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B, 12=C, 13=C#, 14=D, 15=D#
    uint8_t baseNotes[KEYBOARD_KEYS] = {
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75
    };
    
    for (uint8_t i = 0; i < KEYBOARD_KEYS; i++) {
        keyToNote[i] = baseNotes[i];
    }
}
