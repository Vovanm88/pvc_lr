#include "SequencerUI.hpp"
#include "Sequencer.hpp"
#include "drivers/Display.hpp"
#include "drivers/Synthesizer.hpp"
#include "drivers/Keyboard.hpp"
#include "drivers/Uart.hpp"
#include "stm32f4xx_hal.h"
#include <cstdio>

// Key mapping for UI navigation - VERTICAL LAYOUT
// Using vertical keypad layout:
// 1 2 3
// 4 5 6  
// 7 8 9
// 11 10 12
// 
// VERTICAL COLUMNS for different functions:
// Column 1 (1,4,7,11): Track/Beat selection
// Column 2 (2,5,8,10): Navigation arrows
// Column 3 (3,6,9,12): Menu/Control buttons
namespace KeyMapping {
    // Track/Beat selection keys (Column 1)
    static constexpr uint8_t KEY_TRACK_1 = Keyboard::KEY_1;    // 1 key
    static constexpr uint8_t KEY_TRACK_2 = Keyboard::KEY_4;    // 4 key  
    static constexpr uint8_t KEY_TRACK_3 = Keyboard::KEY_7;    // 7 key
    static constexpr uint8_t KEY_TRACK_4 = Keyboard::KEY_11;   // 11 key
    
    // Navigation keys (Column 2)
    static constexpr uint8_t KEY_UP = Keyboard::KEY_2;         // 2 key
    static constexpr uint8_t KEY_DOWN = Keyboard::KEY_5;       // 5 key  
    static constexpr uint8_t KEY_LEFT = Keyboard::KEY_8;       // 8 key
    static constexpr uint8_t KEY_RIGHT = Keyboard::KEY_10;     // 10 key
    
    // Menu/Control keys (Column 3)
    static constexpr uint8_t KEY_OK = Keyboard::KEY_3;         // 3 key
    static constexpr uint8_t KEY_BACK = Keyboard::KEY_6;       // 6 key
    static constexpr uint8_t KEY_MENU = Keyboard::KEY_9;       // 9 key
    static constexpr uint8_t KEY_SETTINGS = Keyboard::KEY_12;  // 12 key
}

// Реализация SequencerRenderer
SequencerRenderer& SequencerRenderer::getInstance() {
    static SequencerRenderer instance;
    return instance;
}

bool SequencerRenderer::init() {
    return true;
}

void SequencerRenderer::drawSplashScreen() {
    display.clear(DisplayColor::BLACK);
    display.setCursor(0, 0);
    display.print("STM32F4");
    display.setCursor(0, 15);
    display.print("Synthesizer");
    display.setCursor(0, 30);
    display.print("Keyboard");
    display.setCursor(0, 45);
    display.print("Loading...");
    display.update();
}

void SequencerRenderer::drawMainMenu() {
    display.clear(DisplayColor::BLACK);
    display.setCursor(0, 0);
    display.print("SEQUENCER MENU");
    
    // Отображаем громкость и темп
    Project& project = Sequencer::getInstance().getProject();
    char tempStr[32];
    sprintf(tempStr, "BPM: %d Vol: %d", project.bpm, project.volume);
    display.setCursor(0, 12);
    display.print(tempStr);
    
    display.setCursor(0, 24);
    display.print("1,4,7,11: Edit tracks");
    display.setCursor(0, 36);
    display.print("3: Play/Stop 6: Back");
    display.setCursor(0, 48);
    display.print("9: Help 12: Settings");
    display.update();
}

void SequencerRenderer::drawEditMode(const EditState& editState) {
    display.clear(DisplayColor::BLACK);
    
    // Статусная строка
    drawStatusBar(Sequencer::getInstance().getProject());
    
    // Отрисовка дорожек
    drawTracks(Sequencer::getInstance().getProject(), editState);
    
    display.update();
}

void SequencerRenderer::drawSettingsMode() {
    display.clear(DisplayColor::BLACK);
    display.setCursor(0, 0);
    display.print("SETTINGS");
    
    Project& project = Sequencer::getInstance().getProject();
    display.setCursor(0, 15);
    display.printf("BPM: %d", project.bpm);
    display.setCursor(0, 25);
    display.printf("Volume: %d", project.volume);
    display.setCursor(0, 35);
    display.print("Up/Down: Change");
    display.setCursor(0, 45);
    display.print("Back: Main menu");
    
    display.update();
}

void SequencerRenderer::drawHelpScreen() {
    display.clear(DisplayColor::BLACK);
    display.setCursor(0, 0);
    display.print("HELP");
    display.setCursor(0, 15);
    display.print("1,4,7,11: Edit tracks");
    display.setCursor(0, 30);
    display.print("2,5,8,10: Navigation");
    display.setCursor(0, 45);
    display.print("3,6,9,12: Actions");
    display.update();
}

void SequencerRenderer::drawTracks(const Project& project, const EditState& editState) {
    for (uint8_t i = 0; i < 6; i++) {
        drawTrack(i, project.tracks[i], editState);
    }
}

void SequencerRenderer::drawTrack(uint8_t trackIndex, const Track& track, const EditState& editState) {
    uint8_t y = TRACKS_START_Y + trackIndex * TRACK_HEIGHT;
    
    // Отрисовка названия дорожки
    display.setCursor(0, y);
    if (track.type == TrackType::DRUM) {
        display.printf("D%d", trackIndex + 1);
    } else {
        display.printf("P%d", trackIndex - 3);
    }
    
    // Отрисовка блоков
    uint8_t startBlock = 0; // TODO: Реализовать прокрутку блоков
    for (uint8_t blockIndex = startBlock; blockIndex < startBlock + MAX_VISIBLE_BLOCKS && blockIndex < track.blocks.size(); blockIndex++) {
        uint8_t x = 20 + (blockIndex - startBlock) * BLOCK_WIDTH;
        bool isSelected = (editState.selectedTrack == trackIndex && editState.selectedBlock == blockIndex);
        bool isCurrent = (track.currentBlock == blockIndex);
        
        drawBlock(x, y, track.blocks[blockIndex], isSelected, isCurrent);
    }
}

void SequencerRenderer::drawDrumTrack(uint8_t trackIndex, const Track& track, const EditState& editState) {
    // Реализация отрисовки барабанной дорожки
    // Аналогично drawTrack, но с учетом особенностей барабанов
}

void SequencerRenderer::drawPianoTrack(uint8_t trackIndex, const Track& track, const EditState& editState) {
    // Реализация отрисовки пианино дорожки
    // Аналогично drawTrack, но с отображением нот
}

void SequencerRenderer::drawBlock(uint8_t x, uint8_t y, const Block& block, bool isSelected, bool isCurrent) {
    // Рамка блока
    display.drawRect(x, y, BLOCK_WIDTH, TRACK_HEIGHT - 1, DisplayColor::WHITE);
    
    // Отрисовка тактов
    for (uint8_t beatIndex = 0; beatIndex < 4; beatIndex++) {
        uint8_t beatX = x + beatIndex * BEAT_WIDTH;
        bool beatSelected = isSelected && (Sequencer::getInstance().getProject().currentBeat == beatIndex);
        bool beatCurrent = isCurrent && (Sequencer::getInstance().getProject().currentBeat == beatIndex);
        
        drawBeat(beatX, y, block.beats[beatIndex], beatSelected, beatCurrent, TrackType::DRUM);
    }
}

void SequencerRenderer::drawBeat(uint8_t x, uint8_t y, const Beat& beat, bool isSelected, bool isCurrent, TrackType trackType) {
    if (trackType == TrackType::DRUM) {
        drawBeatBox(x, y, beat.active, isSelected, isCurrent);
    } else {
        drawNoteBox(x, y, beat.note, beat.halfTone, isSelected, isCurrent);
    }
}

void SequencerRenderer::drawStatusBar(const Project& project) {
    display.setCursor(0, 0);
    display.printf("BPM:%d Vol:%d", project.bpm, project.volume);
    
    if (project.isPlaying) {
        display.setCursor(80, 0);
        display.print("PLAY");
    } else {
        display.setCursor(80, 0);
        display.print("STOP");
    }
}

void SequencerRenderer::drawBeatBox(uint8_t x, uint8_t y, bool active, bool selected, bool current) {
    DisplayColor color = DisplayColor::BLACK;
    
    if (active) {
        color = DisplayColor::WHITE;
    }
    
    display.drawFilledRect(x + 1, y + 1, BEAT_WIDTH - 2, TRACK_HEIGHT - 3, color);
    
    // Рамка для выделения
    if (selected || current) {
        display.drawRect(x, y, BEAT_WIDTH, TRACK_HEIGHT - 1, DisplayColor::WHITE);
    }
}

void SequencerRenderer::drawNoteBox(uint8_t x, uint8_t y, uint8_t note, bool halfTone, bool selected, bool current) {
    if (note == 255) {
        // Пустая нота
        if (selected || current) {
            display.drawRect(x, y, BEAT_WIDTH, TRACK_HEIGHT - 1, DisplayColor::WHITE);
        }
        return;
    }
    
    // Отрисовка ноты
    display.setCursor(x + 1, y + 1);
    const char* noteName = getNoteName(note, halfTone);
    display.print(noteName);
    
    // Рамка для выделения
    if (selected || current) {
        display.drawRect(x, y, BEAT_WIDTH, TRACK_HEIGHT - 1, DisplayColor::WHITE);
    }
}

const char* SequencerRenderer::getNoteName(uint8_t note, bool halfTone) const {
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    if (note >= 12) return "?";
    
    uint8_t index = note + (halfTone ? 1 : 0);
    if (index >= 12) index = 0;
    
    return noteNames[index];
}

// Реализация MenuSystem
MenuSystem& MenuSystem::getInstance() {
    static MenuSystem instance;
    return instance;
}

bool MenuSystem::init() {
    currentMode = SequencerMode::SPLASH_SCREEN;
    splashStartTime = HAL_GetTick();
    
    // Инициализация состояния редактирования
    editState.selectedTrack = 0;
    editState.selectedBlock = 0;
    editState.selectedBeat = 0;
    editState.isEditing = false;
    editState.currentNote = 0; // C
    editState.currentHalfTone = false;
    
    // Инициализация состояния клавиш
    for (int i = 0; i < 13; i++) {
        keyPressed[i] = false;
    }
    
    return true;
}

void MenuSystem::onKeyPress(uint8_t keyCode) {
    if (keyCode == 0 || keyCode > 12) return;
    
    keyPressed[keyCode] = true;
    
    switch (currentMode) {
        case SequencerMode::SPLASH_SCREEN:
            // Любая клавиша переходит в главное меню
            Uart::getInstance().printf("Splash screen: key %d pressed, switching to main menu\n", keyCode);
            showMainMenu();
            break;
            
        case SequencerMode::MAIN_MENU:
            processMainMenu(keyCode);
            break;
            
        case SequencerMode::EDIT_MODE:
            processEditMode(keyCode);
            break;
            
        case SequencerMode::SETTINGS_MODE:
            processSettingsMode(keyCode);
            break;
            
        case SequencerMode::HELP_SCREEN:
            if (keyCode == KeyMapping::KEY_BACK) {
                showMainMenu();
            }
            break;
    }
}

void MenuSystem::onKeyRelease(uint8_t keyCode) {
    if (keyCode == 0 || keyCode > 12) return;
    keyPressed[keyCode] = false;
}

void MenuSystem::update() {
    // Автоматический переход с заставки
    if (currentMode == SequencerMode::SPLASH_SCREEN) {
        if (HAL_GetTick() - splashStartTime > 2000) { // 2 секунды
            showMainMenu();
        }
    }
}

void MenuSystem::setMode(SequencerMode mode) {
    currentMode = mode;
}

void MenuSystem::showSplashScreen() {
    currentMode = SequencerMode::SPLASH_SCREEN;
    splashStartTime = HAL_GetTick();
}

void MenuSystem::showMainMenu() {
    currentMode = SequencerMode::MAIN_MENU;
    Uart::getInstance().printf("Switched to MAIN_MENU mode\n");
    // Принудительно обновляем дисплей
    SequencerRenderer::getInstance().drawMainMenu();
}

void MenuSystem::showEditMode() {
    currentMode = SequencerMode::EDIT_MODE;
    editState.isEditing = true;
    
    // Отладочный вывод
    Uart::getInstance().printf("Entering Edit Mode: track=%d, block=%d, beat=%d\n", 
        editState.selectedTrack, editState.selectedBlock, editState.selectedBeat);
    
    // Убеждаемся, что selectedBlock в пределах
    Track& track = Sequencer::getInstance().getTrack(editState.selectedTrack);
    if (editState.selectedBlock >= track.blocks.size()) {
        editState.selectedBlock = 0;
        Uart::getInstance().printf("Reset selectedBlock to 0\n");
    }
    
    // Принудительно обновляем дисплей
    SequencerRenderer::getInstance().drawEditMode(editState);
}

void MenuSystem::showSettingsMode() {
    currentMode = SequencerMode::SETTINGS_MODE;
}

void MenuSystem::showHelpScreen() {
    currentMode = SequencerMode::HELP_SCREEN;
}

void MenuSystem::processMainMenu(uint8_t keyCode) {
    switch (keyCode) {
        case KeyMapping::KEY_TRACK_1:
        case KeyMapping::KEY_TRACK_2:
        case KeyMapping::KEY_TRACK_3:
        case KeyMapping::KEY_TRACK_4:
            // Выбор дорожки для редактирования (1,4,7,11 -> 0,1,2,3)
            if (keyCode == KeyMapping::KEY_TRACK_1) editState.selectedTrack = 0;
            else if (keyCode == KeyMapping::KEY_TRACK_2) editState.selectedTrack = 1;
            else if (keyCode == KeyMapping::KEY_TRACK_3) editState.selectedTrack = 2;
            else if (keyCode == KeyMapping::KEY_TRACK_4) editState.selectedTrack = 3;
            Uart::getInstance().printf("Selected track %d, entering edit mode\n", editState.selectedTrack);
            showEditMode();
            break;
            
        case Keyboard::KEY_5:
        case Keyboard::KEY_6:
            // Пианино дорожки (5->4, 6->5)
            editState.selectedTrack = keyCode + 1; 
            showEditMode();
            break;
            
        case KeyMapping::KEY_OK:
            // Play/Stop
            Uart::getInstance().printf("Play/Stop key pressed\n");
            if (Sequencer::getInstance().isPlaying()) {
                Uart::getInstance().printf("Stopping sequencer\n");
                Sequencer::getInstance().stop();
            } else {
                Uart::getInstance().printf("Starting sequencer\n");
                Sequencer::getInstance().play();
            }
            break;
            
        case KeyMapping::KEY_SETTINGS:
            Uart::getInstance().printf("Settings key pressed, switching to settings mode\n");
            showSettingsMode();
            break;
            
        case KeyMapping::KEY_MENU:
            // Показать справку
            showHelpScreen();
            break;
            
        default:
            // Игнорируем неизвестные клавиши
            break;
    }
}

void MenuSystem::processEditMode(uint8_t keyCode) {
    Track& track = Sequencer::getInstance().getTrack(editState.selectedTrack);
    
    switch (keyCode) {
        case KeyMapping::KEY_TRACK_1:
        case KeyMapping::KEY_TRACK_2:
        case KeyMapping::KEY_TRACK_3:
        case KeyMapping::KEY_TRACK_4:
            // Для барабанных дорожек - установка/снятие удара в конкретном такте
            // Для пианино дорожек - выбор такта
            if (track.type == TrackType::DRUM) {
                // Определяем номер такта по нажатой клавише
                uint8_t beatIndex = 0;
                if (keyCode == KeyMapping::KEY_TRACK_1) beatIndex = 0;
                else if (keyCode == KeyMapping::KEY_TRACK_2) beatIndex = 1;
                else if (keyCode == KeyMapping::KEY_TRACK_3) beatIndex = 2;
                else if (keyCode == KeyMapping::KEY_TRACK_4) beatIndex = 3;
                
                // Устанавливаем выбранный такт и переключаем удар
                editState.selectedBeat = beatIndex;
                toggleBeat();
                Uart::getInstance().printf("Drum track: toggled beat %d\n", editState.selectedBeat);
            } else if (track.type == TrackType::PIANO) {
                // Выбираем такт для пианино
                uint8_t beatIndex = 0;
                if (keyCode == KeyMapping::KEY_TRACK_1) beatIndex = 0;
                else if (keyCode == KeyMapping::KEY_TRACK_2) beatIndex = 1;
                else if (keyCode == KeyMapping::KEY_TRACK_3) beatIndex = 2;
                else if (keyCode == KeyMapping::KEY_TRACK_4) beatIndex = 3;
                
                editState.selectedBeat = beatIndex;
                Uart::getInstance().printf("Piano track: selected beat %d\n", editState.selectedBeat);
            }
            // Принудительно обновляем дисплей
            SequencerRenderer::getInstance().drawEditMode(editState);
            break;
            
        case KeyMapping::KEY_UP:
            // Навигация вверх
            if (editState.selectedTrack > 0) {
                editState.selectedTrack--;
                Uart::getInstance().printf("Track up: selected track = %d\n", editState.selectedTrack);
                // Принудительно обновляем дисплей
                SequencerRenderer::getInstance().drawEditMode(editState);
            }
            break;
            
        case KeyMapping::KEY_DOWN:
            // Навигация вниз
            if (editState.selectedTrack < MAX_TRACKS - 1) {
                editState.selectedTrack++;
                Uart::getInstance().printf("Track down: selected track = %d\n", editState.selectedTrack);
                // Принудительно обновляем дисплей
                SequencerRenderer::getInstance().drawEditMode(editState);
            }
            break;
            
        case KeyMapping::KEY_LEFT:
            // Предыдущий такт
            if (editState.selectedBeat > 0) {
                editState.selectedBeat--;
                Uart::getInstance().printf("Beat left: selected beat = %d\n", editState.selectedBeat);
                // Принудительно обновляем дисплей
                SequencerRenderer::getInstance().drawEditMode(editState);
            }
            break;
            
        case KeyMapping::KEY_RIGHT:
            // Следующий такт
            if (editState.selectedBeat < 3) {
                editState.selectedBeat++;
                Uart::getInstance().printf("Beat right: selected beat = %d\n", editState.selectedBeat);
                // Принудительно обновляем дисплей
                SequencerRenderer::getInstance().drawEditMode(editState);
            }
            break;
            
        case KeyMapping::KEY_BACK:
            showMainMenu();
            break;
            
        default:
            // Игнорируем неизвестные клавиши
            break;
    }
}

void MenuSystem::processSettingsMode(uint8_t keyCode) {
    Project& project = Sequencer::getInstance().getProject();
    
    switch (keyCode) {
        case KeyMapping::KEY_UP:
            // Увеличить BPM
            project.bpm = std::min(240, project.bpm + 10);
            break;
            
        case KeyMapping::KEY_DOWN:
            // Уменьшить BPM
            project.bpm = std::max(60, project.bpm - 10);
            break;
            
        case KeyMapping::KEY_LEFT:
            // Уменьшить громкость
            if (project.volume > 0) {
                project.volume--;
            }
            break;
            
        case KeyMapping::KEY_RIGHT:
            // Увеличить громкость
            if (project.volume < 10) {
                project.volume++;
            }
            break;
            
        case KeyMapping::KEY_BACK:
            showMainMenu();
            break;
    }
}

void MenuSystem::toggleBeat() {
    Track& track = Sequencer::getInstance().getTrack(editState.selectedTrack);
    
    // Отладочный вывод
    Uart::getInstance().printf("toggleBeat: track=%d, block=%d, beat=%d\n", 
        editState.selectedTrack, editState.selectedBlock, editState.selectedBeat);
    Uart::getInstance().printf("track.blocks.size()=%d\n", track.blocks.size());
    
    if (track.blocks.empty()) {
        Uart::getInstance().printf("ERROR: No blocks in track!\n");
        return;
    }
    
    // Проверяем границы
    if (editState.selectedBlock >= track.blocks.size()) {
        Uart::getInstance().printf("ERROR: selectedBlock out of bounds!\n");
        editState.selectedBlock = 0; // Сбрасываем на первый блок
    }
    
    Block& block = track.blocks[editState.selectedBlock];
    Beat& beat = block.beats[editState.selectedBeat];
    
    if (track.type == TrackType::DRUM) {
        beat.active = !beat.active;
        Uart::getInstance().printf("Drum beat toggled: %s\n", beat.active ? "ON" : "OFF");
    } else {
        if (beat.note == 255) {
            // Установить ноту
            beat.note = editState.currentNote;
            beat.halfTone = editState.currentHalfTone;
            Uart::getInstance().printf("Note set: %d%s\n", beat.note, beat.halfTone ? "#" : "");
        } else {
            // Убрать ноту
            beat.note = 255;
            beat.halfTone = false;
            Uart::getInstance().printf("Note removed\n");
        }
    }
}

// Реализация ImageRenderer
ImageRenderer& ImageRenderer::getInstance() {
    static ImageRenderer instance;
    return instance;
}

void ImageRenderer::drawImage(const uint8_t* imageData, uint16_t width, uint16_t height, uint8_t x, uint8_t y) {
    // Простая реализация отрисовки изображения
    // В реальной реализации здесь будет более сложная логика
    display.setCursor(x, y);
    display.print("IMAGE");
}

void ImageRenderer::drawSplashImage() {
    // Placeholder для заставки
    display.setCursor(0, 0);
    display.print("STM32 SEQUENCER");
    display.setCursor(0, 20);
    display.print("Version 1.0");
}

void ImageRenderer::drawHelpImage() {
    // Placeholder для справки
    display.setCursor(0, 0);
    display.print("HELP SCREEN");
    display.setCursor(0, 10);
    display.print("Use keys to navigate");
}
