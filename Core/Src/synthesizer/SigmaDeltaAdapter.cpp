#include "synthesizer/SigmaDeltaAdapter.hpp"
#include "drivers/Uart.hpp"

SigmaDeltaAdapter& SigmaDeltaAdapter::getInstance() {
    static SigmaDeltaAdapter instance;
    return instance;
}

bool SigmaDeltaAdapter::init() {
    // Инициализация синтезатора
    synthesizer = WaveSynthesizer::getInstance();
    if (!synthesizer.init()) {
        Uart::getInstance().printf("Failed to initialize WaveSynthesizer\n");
        return false;
    }
    
    // Инициализация сигма-дельта PWM
    pwmDriver = SigmaDeltaPWM::getInstance();
    if (!pwmDriver.init(SAMPLE_RATE, PWM_FREQ)) {
        Uart::getInstance().printf("Failed to initialize SigmaDeltaPWM\n");
        return false;
    }
    
    // Запускаем PWM
    pwmDriver.start();
    
    Uart::getInstance().printf("SigmaDeltaAdapter initialized\n");
    return true;
}

void SigmaDeltaAdapter::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    synthesizer.noteOn(channel, note, velocity);
}

void SigmaDeltaAdapter::noteOff(uint8_t channel, uint8_t note) {
    synthesizer.noteOff(channel, note);
}

void SigmaDeltaAdapter::allNotesOff() {
    synthesizer.allNotesOff();
}

void SigmaDeltaAdapter::setWaveType(uint8_t channel, WaveType type) {
    synthesizer.setWaveType(channel, type);
}

void SigmaDeltaAdapter::setADSR(uint8_t channel, const ADSR& adsr) {
    synthesizer.setADSR(channel, adsr);
}

void SigmaDeltaAdapter::setMasterVolume(uint8_t volume) {
    synthesizer.setMasterVolume(volume);
}

void SigmaDeltaAdapter::setChannelVolume(uint8_t channel, uint8_t volume) {
    synthesizer.setChannelVolume(channel, volume);
}

void SigmaDeltaAdapter::update() {
    // Обновляем синтезатор
    synthesizer.update();
    
    // Генерируем аудио сэмпл с полифонией
    float sample = synthesizer.generateSample();
    
    // Отправляем сэмпл в сигма-дельта модулятор
    pwmDriver.pushSample(sample);
}

void SigmaDeltaAdapter::updatePWM() {
    // Обновляем PWM (вызывается из прерывания таймера)
    pwmDriver.updatePWM();
}

uint8_t SigmaDeltaAdapter::getActiveVoices() const {
    return synthesizer.getActiveVoices();
}

bool SigmaDeltaAdapter::isChannelActive(uint8_t channel) const {
    return synthesizer.isChannelActive(channel);
}

