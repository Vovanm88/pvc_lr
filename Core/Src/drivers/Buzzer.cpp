#include "drivers/Buzzer.hpp"
#include "drivers/Uart.hpp"
#include "tim.h"
#include "stm32f4xx_hal.h"

// Внешние переменные из HAL
extern TIM_HandleTypeDef htim1;

Buzzer& Buzzer::getInstance() {
    static Buzzer instance;
    return instance;
}

bool Buzzer::init() {
    // Инициализация каналов
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channels[i] = Channel();
    }
    
    globalVolume = MAX_VOLUME;
    
    // Запуск таймера
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    
    return true;
}

void Buzzer::playNote(uint8_t channel, uint16_t frequency, uint8_t volume) {
    if (channel >= MAX_CHANNELS) return;
    
    Uart::getInstance().printf("Buzzer::playNote: ch=%d, freq=%d, vol=%d\n", 
                              channel, frequency, volume);
    
    channels[channel].frequency = frequency;
    channels[channel].volume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    channels[channel].active = (frequency > 0);
    
    updatePWM();
}

void Buzzer::stopNote(uint8_t channel) {
    if (channel >= MAX_CHANNELS) return;
    
    channels[channel].active = false;
    channels[channel].frequency = 0;
    channels[channel].volume = 0;
    
    updatePWM();
}

void Buzzer::stopAll() {
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        channels[i].active = false;
        channels[i].frequency = 0;
        channels[i].volume = 0;
    }
    
    updatePWM();
}

void Buzzer::setVolume(uint8_t channel, uint8_t volume) {
    if (channel >= MAX_CHANNELS) return;
    
    channels[channel].volume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    updatePWM();
}

void Buzzer::setGlobalVolume(uint8_t volume) {
    globalVolume = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
    updatePWM();
}

void Buzzer::playMelody(const Melody& melody) {
    melodyPlayer.play(melody);
}

void Buzzer::stopMelody() {
    melodyPlayer.stop();
}

bool Buzzer::isPlaying() const {
    return melodyPlayer.isPlaying();
}

void Buzzer::update() {
    melodyPlayer.update();
}

void Buzzer::updatePWM() {
    // Находим активный канал с максимальной частотой
    uint16_t maxFreq = 0;
    uint8_t maxVolume = 0;
    
    for (uint8_t i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active && channels[i].frequency > maxFreq) {
            maxFreq = channels[i].frequency;
            maxVolume = channels[i].volume;
        }
    }
    
    if (maxFreq > 0 && maxFreq < 20000) { // Проверяем разумные пределы частоты
        // Настраиваем частоту и громкость
        // Частота = PCLK / ((PSC + 1) * (ARR + 1))
        // Для простоты используем ARR = 1000, тогда PSC = PCLK / (freq * 1000) - 1
        uint32_t pclk = HAL_RCC_GetPCLK1Freq();
        uint32_t psc = (pclk / (maxFreq * 1000)) - 1;
        
        // Ограничиваем PSC разумными значениями
        if (psc > 65535) psc = 65535;
        if (psc < 1) psc = 1;
        
        // Останавливаем таймер перед изменением параметров
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        
        TIM1->PSC = psc;
        TIM1->ARR = 1000;
        
        // Устанавливаем duty cycle для громкости
        uint32_t duty = (maxVolume * globalVolume * 1000) / (MAX_VOLUME * MAX_VOLUME);
        if (duty > 1000) duty = 1000;
        TIM1->CCR1 = duty;
        
        // Запускаем таймер с новыми параметрами
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        
        // Отладочный вывод
        Uart::getInstance().printf("PWM: freq=%d, pclk=%lu, psc=%lu, duty=%lu\n", 
                                  maxFreq, pclk, psc, duty);
    } else {
        // Выключаем звук
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        TIM1->CCR1 = 0;
    }
}

uint16_t Buzzer::calculatePWMValue(uint16_t frequency, uint8_t volume) {
    if (frequency == 0) return 0;
    
    // Простое микширование - берем максимальную частоту
    return volume;
}

void Buzzer::mixChannels() {
    // Простое микширование - находим канал с максимальной частотой
    updatePWM();
}

// Реализация MelodyPlayer
MelodyPlayer::MelodyPlayer() 
    : currentMelody(nullptr), currentNote(0), noteStartTime(0), playing(false), paused(false) {
}

void MelodyPlayer::play(const Melody& melody) {
    currentMelody = &melody;
    currentNote = 0;
    noteStartTime = HAL_GetTick();
    playing = true;
    paused = false;
    
    // Начинаем воспроизведение первой ноты
    if (currentNote < currentMelody->length) {
        const Note& note = currentMelody->notes[currentNote];
        Buzzer::getInstance().playNote(note.channel, note.frequency, Buzzer::MAX_VOLUME);
    }
}

void MelodyPlayer::stop() {
    playing = false;
    paused = false;
    Buzzer::getInstance().stopAll();
}

void MelodyPlayer::pause() {
    if (playing && !paused) {
        paused = true;
        Buzzer::getInstance().stopAll();
    }
}

void MelodyPlayer::resume() {
    if (playing && paused) {
        paused = false;
        noteStartTime = HAL_GetTick();
        
        // Возобновляем текущую ноту
        if (currentNote < currentMelody->length) {
            const Note& note = currentMelody->notes[currentNote];
            Buzzer::getInstance().playNote(note.channel, note.frequency, Buzzer::MAX_VOLUME);
        }
    }
}

void MelodyPlayer::update() {
    if (!playing || paused) return;
    
    if (currentNote >= currentMelody->length) {
        stop();
        return;
    }
    
    const Note& note = currentMelody->notes[currentNote];
    uint32_t noteDuration = getNoteDuration(note.duration);
    
    if (HAL_GetTick() - noteStartTime >= noteDuration) {
        // Переходим к следующей ноте
        currentNote++;
        noteStartTime = HAL_GetTick();
        
        if (currentNote < currentMelody->length) {
            const Note& nextNote = currentMelody->notes[currentNote];
            Buzzer::getInstance().playNote(nextNote.channel, nextNote.frequency, Buzzer::MAX_VOLUME);
        } else {
            stop();
        }
    }
}

uint32_t MelodyPlayer::getNoteDuration(uint32_t duration) const {
    if (currentMelody == nullptr) return 0;
    
    // Конвертируем длительность ноты в миллисекунды на основе темпа
    // 60000 мс / (BPM * 4) = длительность четвертной ноты
    uint32_t quarterNoteMs = 60000 / (currentMelody->tempo * 4);
    return (duration * quarterNoteMs) / 4;
}
