#include "synthesizer/WaveSynthesizer.hpp"
#include <math.h>
#include <stdlib.h>

// Реализация WaveGenerator
WaveGenerator& WaveGenerator::getInstance() {
    static WaveGenerator instance;
    return instance;
}

float WaveGenerator::generateSine(float phase) {
    return sinf(phase);
}

float WaveGenerator::generateSquare(float phase) {
    return (sinf(phase) >= 0.0f) ? 1.0f : -1.0f;
}

float WaveGenerator::generateSawtooth(float phase) {
    // Нормализуем фазу к [0, 2π]
    while (phase < 0.0f) phase += 2.0f * M_PI;
    while (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    
    // Пилообразная волна: линейное нарастание от -1 до 1
    return 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
}

float WaveGenerator::generateTriangle(float phase) {
    // Нормализуем фазу к [0, 2π]
    while (phase < 0.0f) phase += 2.0f * M_PI;
    while (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    
    if (phase < M_PI) {
        // Восходящий треугольник: от -1 до 1
        return 2.0f * (phase / M_PI) - 1.0f;
    } else {
        // Нисходящий треугольник: от 1 до -1
        return 3.0f - 2.0f * (phase / M_PI);
    }
}

float WaveGenerator::generateNoise() {
    // Простой белый шум
    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

float WaveGenerator::generateWave(WaveType type, float phase) {
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

float WaveGenerator::generateWithHarmonics(WaveType type, float phase, uint8_t harmonics) {
    float result = 0.0f;
    float amplitude = 1.0f;
    
    for (uint8_t i = 1; i <= harmonics && i <= MAX_HARMONICS; i++) {
        float harmonicPhase = phase * i;
        float harmonicAmplitude = amplitude / i; // Убывающая амплитуда гармоник
        
        switch (type) {
            case WaveType::SINE:
                result += harmonicAmplitude * generateSine(harmonicPhase);
                break;
            case WaveType::SQUARE:
                result += harmonicAmplitude * generateSine(harmonicPhase);
                break;
            case WaveType::SAWTOOTH:
                result += harmonicAmplitude * generateSine(harmonicPhase);
                break;
            case WaveType::TRIANGLE:
                // Треугольная волна имеет только нечетные гармоники
                if (i % 2 == 1) {
                    result += harmonicAmplitude * generateSine(harmonicPhase);
                }
                break;
            default:
                result += harmonicAmplitude * generateSine(harmonicPhase);
                break;
        }
    }
    
    // Нормализация амплитуды
    if (harmonics > 1) {
        result /= harmonics;
    }
    
    return result;
}

void WaveGenerator::generateWaveTable() {
    if (tableGenerated) return;
    
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        float phase = (2.0f * M_PI * i) / WAVE_TABLE_SIZE;
        waveTable[i] = generateSine(phase);
    }
    
    tableGenerated = true;
}
