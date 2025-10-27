#!/usr/bin/env python3
"""
Быстрый тест синтезатора
"""

import time

def test_basic():
    """Базовый тест"""
    print("Быстрый тест Python Synthesizer")
    print("=" * 40)
    
    try:
        from python_synthesizer import Synthesizer, DrumPreset
        
        print("Инициализация синтезатора...")
        synth = Synthesizer()
        
        print("✓ Синтезатор инициализирован")
        
        # Тест ноты
        print("Тест ноты C4 (60)...")
        synth.note_on(0, 60, 0.8)
        time.sleep(1)
        synth.note_off(0, 60)
        print("✓ Нота воспроизведена")
        
        time.sleep(0.5)
        
        # Тест барабана
        print("Тест барабана Kick...")
        synth.play_drum(DrumPreset.KICK, 0.8)
        time.sleep(0.5)
        print("✓ Барабан воспроизведен")
        
        print("\n🎵 Все тесты пройдены успешно!")
        print("Теперь можно запускать полную демонстрацию:")
        print("python demo_synthesizer.py")
        
    except ImportError as e:
        print(f"❌ Ошибка импорта: {e}")
        print("Установите зависимости: pip install numpy pygame")
        
    except Exception as e:
        print(f"❌ Ошибка: {e}")
        print("Проверьте настройки звука в системе")

if __name__ == "__main__":
    test_basic()
