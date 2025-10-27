#ifndef SEQUENCERUI_HPP
#define SEQUENCERUI_HPP

#include <stdint.h>
#include <stdbool.h>
#include "Sequencer.hpp"
#include "drivers/Display.hpp"

// Режимы работы секвенсора
enum class SequencerMode {
    SPLASH_SCREEN,      // Начальная заставка
    MAIN_MENU,          // Главное меню
    EDIT_MODE,          // Режим редактирования
    SETTINGS_MODE,      // Настройки
    HELP_SCREEN         // Справка
};

// Состояние редактирования
struct EditState {
    uint8_t selectedTrack;      // Выбранная дорожка (0-5)
    uint8_t selectedBlock;      // Выбранный блок
    uint8_t selectedBeat;        // Выбранный такт (0-3)
    bool isEditing;             // Режим редактирования
    uint8_t currentNote;        // Текущая нота для пианино
    bool currentHalfTone;       // Полутон для пианино
};

// Класс для отрисовки секвенсора
class SequencerRenderer {
public:
    static SequencerRenderer& getInstance();
    
    // Инициализация
    bool init();
    
    // Отрисовка различных экранов
    void drawSplashScreen();
    void drawMainMenu();
    void drawEditMode(const EditState& editState);
    void drawSettingsMode();
    void drawHelpScreen();
    
    // Отрисовка дорожек
    void drawTracks(const Project& project, const EditState& editState);
    void drawTrack(uint8_t trackIndex, const Track& track, const EditState& editState);
    void drawDrumTrack(uint8_t trackIndex, const Track& track, const EditState& editState);
    void drawPianoTrack(uint8_t trackIndex, const Track& track, const EditState& editState);
    
    // Отрисовка блоков и тактов
    void drawBlock(uint8_t x, uint8_t y, const Block& block, bool isSelected, bool isCurrent);
    void drawBeat(uint8_t x, uint8_t y, const Beat& beat, bool isSelected, bool isCurrent, TrackType trackType);
    
    // Отрисовка статусной строки
    void drawStatusBar(const Project& project);
    
    // Отрисовка курсора
    void drawCursor(uint8_t x, uint8_t y);
    
    // Константы для отрисовки
    static constexpr uint8_t TRACK_HEIGHT = 8;
    static constexpr uint8_t BEAT_WIDTH = 8;
    static constexpr uint8_t BLOCK_WIDTH = 32;  // 4 такта * 8 пикселей
    static constexpr uint8_t STATUS_HEIGHT = 10;
    static constexpr uint8_t TRACKS_START_Y = STATUS_HEIGHT;
    static constexpr uint8_t MAX_VISIBLE_BLOCKS = 2;  // На экране помещается 2 блока
    
private:
    SequencerRenderer() : display(Display::getInstance()) {}
    ~SequencerRenderer() = default;
    SequencerRenderer(const SequencerRenderer&) = delete;
    SequencerRenderer& operator=(const SequencerRenderer&) = delete;
    
    Display& display;
    
    // Внутренние методы
    void drawBeatBox(uint8_t x, uint8_t y, bool active, bool selected, bool current);
    void drawNoteBox(uint8_t x, uint8_t y, uint8_t note, bool halfTone, bool selected, bool current);
    const char* getNoteName(uint8_t note, bool halfTone) const;
    void drawTextCentered(const char* text, uint8_t y);
};

// Класс для управления меню
class MenuSystem {
public:
    static MenuSystem& getInstance();
    
    // Инициализация
    bool init();
    
    // Обработка нажатий клавиш
    void onKeyPress(uint8_t keyCode);
    void onKeyRelease(uint8_t keyCode);
    
    // Обновление (вызывается из задачи)
    void update();
    
    // Получение состояния
    SequencerMode getCurrentMode() const { return currentMode; }
    EditState& getEditState() { return editState; }
    const EditState& getEditState() const { return editState; }
    
    // Управление режимами
    void setMode(SequencerMode mode);
    void showSplashScreen();
    void showMainMenu();
    void showEditMode();
    void showSettingsMode();
    void showHelpScreen();
    
    // Обработка навигации
    void handleNavigation(uint8_t keyCode);
    void handleEditMode(uint8_t keyCode);
    void handleSettingsMode(uint8_t keyCode);
    
    // Константы
    static constexpr uint8_t MAX_TRACKS = 6;
    static constexpr uint8_t MAX_BLOCKS_PER_TRACK = 16;
    static constexpr uint8_t BEATS_PER_BLOCK = 4;
    
private:
    MenuSystem() = default;
    ~MenuSystem() = default;
    MenuSystem(const MenuSystem&) = delete;
    MenuSystem& operator=(const MenuSystem&) = delete;
    
    SequencerMode currentMode;
    EditState editState;
    uint32_t splashStartTime;
    bool keyPressed[13];  // Состояние клавиш (индекс 0 не используется)
    
    // Внутренние методы
    void processMainMenu(uint8_t keyCode);
    void processEditMode(uint8_t keyCode);
    void processSettingsMode(uint8_t keyCode);
    void updateEditState();
    void selectTrack(uint8_t trackIndex);
    void selectBlock(uint8_t blockIndex);
    void selectBeat(uint8_t beatIndex);
    void toggleBeat();
    void setNote(uint8_t note, bool halfTone);
    void clearNote();
};

// Класс для отрисовки изображений
class ImageRenderer {
public:
    static ImageRenderer& getInstance();
    
    // Отрисовка изображения из hex-данных
    void drawImage(const uint8_t* imageData, uint16_t width, uint16_t height, uint8_t x, uint8_t y);
    
    // Отрисовка заставки
    void drawSplashImage();
    
    // Отрисовка экрана справки
    void drawHelpImage();
    
private:
    ImageRenderer() : display(Display::getInstance()) {}
    ~ImageRenderer() = default;
    ImageRenderer(const ImageRenderer&) = delete;
    ImageRenderer& operator=(const ImageRenderer&) = delete;
    
    Display& display;
};

#endif // SEQUENCERUI_HPP
