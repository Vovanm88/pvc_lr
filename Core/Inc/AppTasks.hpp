#ifndef APPTASKS_HPP
#define APPTASKS_HPP

#include "scheduler/Task.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/Display.hpp"
#include "drivers/Uart.hpp"
#include "drivers/Buzzer.hpp"
#include "drivers/Synthesizer.hpp"
#include "UartControl.hpp"

// Forward declarations
class Sequencer;
class PianoController;

// Задача для обработки клавиатуры
class KeyboardTask : public Task {
public:
    KeyboardTask();
    void onInit() override;
    void update() override;
    
private:
    Keyboard& keyboard;
    void onKeyEvent(uint8_t keyCode, KeyEvent event);
    static void keyEventCallback(uint8_t keyCode, KeyEvent event);
    static KeyboardTask* instance;
};

// Задача для обновления дисплея
class DisplayTask : public Task {
public:
    DisplayTask();
    void onInit() override;
    void update() override;
    
private:
    Display& display;
    uint32_t lastUpdateTime;
    void updateDisplay();
};

// Задача для обработки UART
class UartTask : public Task {
public:
    UartTask();
    void onInit() override;
    void update() override;
    
private:
    Uart& uart;
    void processReceivedData();
};

// Задача для управления buzzer
class BuzzerTask : public Task {
public:
    BuzzerTask();
    void onInit() override;
    void update() override;
    
private:
    Buzzer& buzzer;
    void updateBuzzer();
};

// Задача для синтезатора
class SynthesizerTask : public Task {
public:
    SynthesizerTask();
    void onInit() override;
    void update() override;
    
private:
    Synthesizer& synthesizer;
    void updateSynthesizer();
};

// Задача для пианино контроллера
class PianoTask : public Task {
public:
    PianoTask();
    void onInit() override;
    void update() override;
    
private:
    PianoController& pianoController;
    void updatePiano();
};

// Задача для секвенсора
class SequencerTask : public Task {
public:
    SequencerTask();
    void onInit() override;
    void update() override;
    
private:
    Sequencer& sequencer;
    void updateSequencer();
};

// Задача для управления UART через кнопку
class UartControlTask : public Task {
public:
    UartControlTask();
    void onInit() override;
    void update() override;
    
private:
    UartControl& uartControl;
    void updateUartControl();
};

// Отладочная задача для вывода информации о шедулере
class DebugTask : public Task {
public:
    DebugTask();
    void onInit() override;
    void update() override;
    
private:
    uint32_t lastPrintTime;
    static constexpr uint32_t PRINT_INTERVAL_MS = 1000; // Каждые 5 секунд
};

#endif // APPTASKS_HPP
