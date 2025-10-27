#include "synthesizer/SynthesizerBridge.hpp"
#include "drivers/Uart.hpp"

// Реализация SynthesizerBridge
SynthesizerBridge& SynthesizerBridge::getInstance() {
    static SynthesizerBridge instance;
    return instance;
}

bool SynthesizerBridge::init() {
    // Инициализируем все синтезаторы
    bool oldInit = oldSynth.init();
    bool newInit = newSynth.init();
    bool adapterInit = adapter.init();
    
    // По умолчанию используем адаптер для Buzzer
    useNew = false;
    useAdapter = true;
    
    Uart::getInstance().printf("SynthesizerBridge: Old=%s, New=%s, Adapter=%s, Using=%s\n",
                              oldInit ? "OK" : "FAIL",
                              newInit ? "OK" : "FAIL",
                              adapterInit ? "OK" : "FAIL",
                              useAdapter ? "ADAPTER" : (useNew ? "NEW" : "OLD"));
    
    return oldInit && newInit && adapterInit;
}

void SynthesizerBridge::useOldSynthesizer() {
    useNew = false;
    useAdapter = false;
    Uart::getInstance().printf("Switched to OLD synthesizer\n");
}

void SynthesizerBridge::useNewSynthesizer() {
    useNew = true;
    useAdapter = false;
    Uart::getInstance().printf("Switched to NEW synthesizer\n");
}

void SynthesizerBridge::useBuzzerAdapter() {
    useNew = false;
    useAdapter = true;
    Uart::getInstance().printf("Switched to BUZZER ADAPTER\n");
}

void SynthesizerBridge::noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (useAdapter) {
        adapter.noteOn(channel, note, velocity);
    } else if (useNew) {
        newSynth.noteOn(channel, note, velocity);
    } else {
        oldSynth.noteOn(channel, note, velocity);
    }
}

void SynthesizerBridge::noteOff(uint8_t channel, uint8_t note) {
    if (useAdapter) {
        adapter.noteOff(channel, note);
    } else if (useNew) {
        newSynth.noteOff(channel, note);
    } else {
        oldSynth.noteOff(channel, note);
    }
}

void SynthesizerBridge::allNotesOff() {
    if (useAdapter) {
        adapter.allNotesOff();
    } else if (useNew) {
        newSynth.allNotesOff();
    } else {
        oldSynth.allNotesOff();
    }
}

void SynthesizerBridge::setWaveType(uint8_t channel, WaveType type) {
    if (useAdapter) {
        adapter.setWaveType(channel, type);
    } else if (useNew) {
        newSynth.setWaveType(channel, type);
    } else {
        Uart::getInstance().printf("Wave type control only available in NEW synthesizer or ADAPTER\n");
    }
}

void SynthesizerBridge::update() {
    if (useAdapter) {
        adapter.update();
    } else if (useNew) {
        newSynth.update();
    } else {
        oldSynth.update();
    }
}

uint8_t SynthesizerBridge::getActiveVoices() const {
    if (useAdapter) {
        return adapter.getActiveVoices();
    } else if (useNew) {
        return newSynth.getActiveVoices();
    } else {
        return oldSynth.getActiveVoices();
    }
}

bool SynthesizerBridge::isChannelActive(uint8_t channel) const {
    if (useAdapter) {
        return adapter.isChannelActive(channel);
    } else if (useNew) {
        return newSynth.isChannelActive(channel);
    } else {
        return oldSynth.isChannelActive(channel);
    }
}
