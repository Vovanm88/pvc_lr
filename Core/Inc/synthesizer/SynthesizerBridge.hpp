#ifndef SYNTHESIZER_BRIDGE_HPP
#define SYNTHESIZER_BRIDGE_HPP

#include "synthesizer/WaveSynthesizer.hpp"
#include "synthesizer/AudioToBuzzerAdapter.hpp"
#include "drivers/Synthesizer.hpp"

// Мост между старым и новым синтезатором
class SynthesizerBridge {
public:
    static SynthesizerBridge& getInstance();
    
    // Инициализация
    bool init();
    
    // Переключение между синтезаторами
    void useOldSynthesizer();
    void useNewSynthesizer();
    void useBuzzerAdapter();
    
    // Универсальный интерфейс
    void noteOn(uint8_t channel, uint8_t note, uint8_t velocity = 64);
    void noteOff(uint8_t channel, uint8_t note);
    void allNotesOff();
    
    // Управление типом волны (только для нового синтезатора)
    void setWaveType(uint8_t channel, WaveType type);
    
    // Обновление
    void update();
    
    // Состояние
    uint8_t getActiveVoices() const;
    bool isChannelActive(uint8_t channel) const;
    
    // Получение текущего синтезатора
    bool isUsingNewSynthesizer() const { return useNew; }
    bool isUsingBuzzerAdapter() const { return useAdapter; }
    
private:
    SynthesizerBridge() = default;
    ~SynthesizerBridge() = default;
    SynthesizerBridge(const SynthesizerBridge&) = delete;
    SynthesizerBridge& operator=(const SynthesizerBridge&) = delete;
    
    bool useNew; // true = новый синтезатор, false = старый
    bool useAdapter; // true = адаптер для Buzzer
    Synthesizer& oldSynth;
    WaveSynthesizer& newSynth;
    AudioToBuzzerAdapter& adapter;
};

#endif // SYNTHESIZER_BRIDGE_HPP
