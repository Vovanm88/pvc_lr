#include "synthesizer/WaveSynthesizer.hpp"
#include <math.h>

// Реализация VoiceMixer
VoiceMixer& VoiceMixer::getInstance() {
    static VoiceMixer instance;
    return instance;
}

float VoiceMixer::mixVoices(Voice* voices, uint8_t voiceCount) {
    float mixedSample = 0.0f;
    uint8_t activeVoices = 0;
    
    // Смешиваем все активные голоса
    for (uint8_t i = 0; i < voiceCount; i++) {
        if (voices[i].active) {
            float sample = generateWaveSample(voices[i]);
            float adsrVolume = applyADSR(voices[i], sample);
            
            if (adsrVolume > 0.0f) {
                mixedSample += adsrVolume;
                activeVoices++;
            }
        }
    }
    
    // Нормализация для предотвращения клиппинга
    if (activeVoices > 0) {
        mixedSample /= activeVoices;
        // Ограничиваем амплитуду
        if (mixedSample > 1.0f) mixedSample = 1.0f;
        if (mixedSample < -1.0f) mixedSample = -1.0f;
    }
    
    return mixedSample;
}

float VoiceMixer::applyADSR(const Voice& voice, float sample) {
    float adsrVolume = calculateADSRVolume(voice);
    return sample * adsrVolume;
}

float VoiceMixer::calculateADSRVolume(const Voice& voice) {
    uint32_t currentTime = HAL_GetTick();
    uint32_t elapsed = currentTime - voice.startTime;
    
    float baseVolume = (float)voice.velocity / 127.0f;
    float adsrVolume = baseVolume;
    
    if (voice.released) {
        // Фаза Release
        uint32_t releaseElapsed = currentTime - voice.releaseTime;
        if (releaseElapsed >= voice.adsr.release) {
            return 0.0f; // Голос должен быть отключен
        }
        // Линейное затухание в фазе Release
        adsrVolume = baseVolume * ((float)voice.adsr.sustain / 10.0f) * 
                     (1.0f - (float)releaseElapsed / voice.adsr.release);
    } else {
        // Фазы Attack, Decay, Sustain
        if (elapsed < voice.adsr.attack) {
            // Фаза Attack
            adsrVolume = baseVolume * ((float)elapsed / voice.adsr.attack);
        } else if (elapsed < voice.adsr.attack + voice.adsr.decay) {
            // Фаза Decay
            uint32_t decayElapsed = elapsed - voice.adsr.attack;
            float sustainLevel = (float)voice.adsr.sustain / 10.0f;
            adsrVolume = baseVolume - (baseVolume - sustainLevel) * 
                        ((float)decayElapsed / voice.adsr.decay);
        } else {
            // Фаза Sustain
            adsrVolume = baseVolume * ((float)voice.adsr.sustain / 10.0f);
        }
    }
    
    return adsrVolume;
}

float VoiceMixer::generateWaveSample(const Voice& voice) {
    WaveGenerator& waveGen = WaveGenerator::getInstance();
    
    // Генерируем волну
    float sample = waveGen.generateWave(voice.waveType, voice.phase);
    
    // Добавляем гармоники для более богатого звука
    if (voice.waveType != WaveType::NOISE) {
        sample = waveGen.generateWithHarmonics(voice.waveType, voice.phase, 4);
    }
    
    return sample;
}
