#include "AppTasks.hpp"
#include "scheduler/Scheduler.hpp"
#include "drivers/Uart.hpp"
#include "UartControl.hpp"
#include "usart.h"
#include "Sequencer.hpp"
#include "SequencerUI.hpp"
#include <stdio.h>
#include <string.h>

// Статические переменные
KeyboardTask* KeyboardTask::instance = nullptr;

// Реализация KeyboardTask
KeyboardTask::KeyboardTask() : Task(10), keyboard(Keyboard::getInstance()) {
    instance = this;
}

void KeyboardTask::onInit() {
    keyboard.init();
    keyboard.setCallback(keyEventCallback);
}

void KeyboardTask::update() {
    // Отладочный вывод каждые 3000мс
    static uint32_t lastKeyboardDebug = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastKeyboardDebug >= 3000) {
        Uart::getInstance().printf("KeyboardTask::update called\n");
        lastKeyboardDebug = currentTime;
    }
    
    keyboard.scan();
}

void KeyboardTask::onKeyEvent(uint8_t keyCode, KeyEvent event) {
    switch (event) {
        case KeyEvent::PRESS:
            // Отправляем информацию о нажатии по UART
            Uart::getInstance().printf("Key %d pressed\n", keyCode);
            
            // Передаем событие в систему меню секвенсора
            MenuSystem::getInstance().onKeyPress(keyCode);
            break;
            
        case KeyEvent::RELEASE:
            MenuSystem::getInstance().onKeyRelease(keyCode);
            break;
            
        case KeyEvent::LONG_PRESS:
            Uart::getInstance().printf("Key %d long pressed\n", keyCode);
            // Длинное нажатие - барабанный звук
            if (keyCode < 8) {
                // Клавиши 0-7 - барабаны
                DrumPreset drums[] = {
                    DrumPreset::KICK, DrumPreset::SNARE, DrumPreset::HIHAT, DrumPreset::CRASH,
                    DrumPreset::RIDE, DrumPreset::TOM_HIGH, DrumPreset::TOM_MID, DrumPreset::TOM_LOW
                };
                Synthesizer::getInstance().playDrum(drums[keyCode], 80);
            } else {
                // Остальные клавиши - ноты
                PianoController::getInstance().onKeyPress(keyCode);
            }
            break;
    }
}

void KeyboardTask::keyEventCallback(uint8_t keyCode, KeyEvent event) {
    if (instance) {
        instance->onKeyEvent(keyCode, event);
    }
}

// Реализация DisplayTask
DisplayTask::DisplayTask() : Task(20), display(Display::getInstance()), lastUpdateTime(0) {
}

void DisplayTask::onInit() {
    display.init();
    display.clear(DisplayColor::BLACK);
    display.setCursor(0, 0);
    display.print("STM32F4 Synthesizer");
    display.setCursor(0, 15);
    display.print("Keyboard Ready");
    display.setCursor(0, 30);
    display.print("UART Ready");
    display.setCursor(0, 45);
    display.print("Press any key");
    display.update();
}

void DisplayTask::update() {
    uint32_t currentTime = HAL_GetTick();
    
    // Обновляем дисплей каждые 50 мс
    if (currentTime - lastUpdateTime >= 50) {
        updateDisplay();
        lastUpdateTime = currentTime;
    }
}

void DisplayTask::updateDisplay() {
    // Делегируем отрисовку в SequencerRenderer
    SequencerRenderer& renderer = SequencerRenderer::getInstance();
    MenuSystem& menuSystem = MenuSystem::getInstance();
    
    switch (menuSystem.getCurrentMode()) {
        case SequencerMode::SPLASH_SCREEN:
            renderer.drawSplashScreen();
            break;
            
        case SequencerMode::MAIN_MENU:
            renderer.drawMainMenu();
            break;
            
        case SequencerMode::EDIT_MODE:
            renderer.drawEditMode(menuSystem.getEditState());
            break;
            
        case SequencerMode::SETTINGS_MODE:
            renderer.drawSettingsMode();
            break;
            
        case SequencerMode::HELP_SCREEN:
            renderer.drawHelpScreen();
            break;
    }
}

// Реализация UartTask
UartTask::UartTask() : Task(30), uart(Uart::getInstance()) {
}

void UartTask::onInit() {
    uart.init();
    
    // Запускаем прием данных по прерыванию
    static uint8_t rxBuffer;
    HAL_UART_Receive_IT(&huart6, &rxBuffer, 1);
    
    uart.printf("STM32F4 C++ System Started\n");
    uart.printf("Keyboard, Display, Buzzer ready\n");
}

void UartTask::update() {
    processReceivedData();
}

void UartTask::processReceivedData() {
    // Проверяем, включен ли UART
    if (!UartControl::getInstance().isEnabled()) {
        return;
    }
    
    uint8_t data;
    while (uart.receive(data)) {
        // Эхо полученных данных
        uart.send(data);
        
        // Обработка команд секвенсора
        switch (data) {
            case 'h':
            case 'H':
                uart.printf("\nSequencer Commands:\n");
                uart.printf("h - help\n");
                uart.printf("s - status\n");
                uart.printf("t - tasks info\n");
                uart.printf("save - save project\n");
                uart.printf("load - load project\n");
                uart.printf("play - start playback\n");
                uart.printf("stop - stop playback\n");
                break;
                
            case 's':
            case 'S':
                uart.printf("\nSequencer Status:\n");
                uart.printf("Uptime: %lu ms\n", HAL_GetTick());
                uart.printf("BPM: %d\n", Sequencer::getInstance().getBPM());
                uart.printf("Volume: %d\n", Sequencer::getInstance().getVolume());
                uart.printf("Playing: %s\n", Sequencer::getInstance().isPlaying() ? "Yes" : "No");
                uart.printf("Active voices: %d\n", Synthesizer::getInstance().getActiveVoices());
                break;
                
            case 'p':
            case 'P':
                uart.printf("\nPlayback toggled\n");
                if (Sequencer::getInstance().isPlaying()) {
                    Sequencer::getInstance().stop();
                    uart.printf("Stopped\n");
                } else {
                    Sequencer::getInstance().play();
                    uart.printf("Started\n");
                }
                break;
                
            case 't':
            case 'T':
                uart.printf("\n=== TASKS INFO ===\n");
                Scheduler::getInstance().printTaskInfo();
                uart.printf("=== END TASKS INFO ===\n");
                break;
                
            case '\r':
            case '\n':
                uart.printf("\n> ");
                break;
        }
    }
}

// Реализация BuzzerTask
BuzzerTask::BuzzerTask() : Task(40), buzzer(Buzzer::getInstance()) {
}

void BuzzerTask::onInit() {
    buzzer.init();
}

void BuzzerTask::update() {
    buzzer.update();
}

// Реализация SynthesizerTask
SynthesizerTask::SynthesizerTask() : Task(50), synthesizer(Synthesizer::getInstance()) {
}

void SynthesizerTask::onInit() {
    synthesizer.init();
}

void SynthesizerTask::update() {
    synthesizer.update();
}

// Реализация PianoTask
PianoTask::PianoTask() : Task(60), pianoController(PianoController::getInstance()) {
}

void PianoTask::onInit() {
    pianoController.init();
}

void PianoTask::update() {
    // PianoController doesn't need periodic updates
    // It responds to key events directly
}

// Реализация SequencerTask
SequencerTask::SequencerTask() : Task(100), sequencer(Sequencer::getInstance()) {
    Uart::getInstance().printf("SequencerTask constructor called\n");
}

void SequencerTask::onInit() {
    Uart::getInstance().printf("SequencerTask::onInit called\n");
    sequencer.init();
    Uart::getInstance().printf("SequencerTask::onInit completed\n");
}

void SequencerTask::update() {
    // Отладочный вывод каждые 1000мс
    static uint32_t lastTaskDebug = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastTaskDebug >= 1000) {
        Uart::getInstance().printf("SequencerTask::update called\n");
        lastTaskDebug = currentTime;
    }
    
    sequencer.update();
}

// Реализация UartControlTask
UartControlTask::UartControlTask() : Task(80), uartControl(UartControl::getInstance()) {
}

void UartControlTask::onInit() {
    uartControl.init();
}

void UartControlTask::update() {
    uartControl.update();
}

// Реализация DebugTask
DebugTask::DebugTask() : Task(90), lastPrintTime(0) {
}

void DebugTask::onInit() {
    lastPrintTime = HAL_GetTick();
    Uart::getInstance().printf("DebugTask: Initialized (interval=%lu ms)\n", PRINT_INTERVAL_MS);
}

void DebugTask::update() {
    // DebugTask теперь не выводит автоматически
    // Вывод только по команде через боковую кнопку
}
