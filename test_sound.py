#!/usr/bin/env python3
"""
Простой тест звука для проверки работы синтезатора
"""

import time
import numpy as np
import pygame

def test_pygame_sound():
    """Тест базового звука pygame"""
    print("Тестирование pygame звука...")
    
    # Инициализация pygame
    pygame.mixer.init(frequency=44100, size=-16, channels=2, buffer=512)
    
    # Генерируем простой тон
    duration = 1.0  # 1 секунда
    sample_rate = 44100
    frequency = 440  # A4
    
    t = np.linspace(0, duration, int(duration * sample_rate))
    samples = 0.5 * np.sin(2 * np.pi * frequency * t)
    
    # Конвертируем в 16-bit
    samples_16bit = (samples * 32767).astype(np.int16)
    stereo_samples = np.column_stack((samples_16bit, samples_16bit))
    
    # Воспроизводим
    sound_array = pygame.sndarray.make_sound(stereo_samples)
    sound_array.play()
    
    print("Звук должен воспроизводиться 1 секунду...")
    time.sleep(2)
    print("Тест завершен")

def test_synthesizer():
    """Тест синтезатора"""
    print("Тестирование синтезатора...")
    
    try:
        from python_synthesizer import Synthesizer, DrumPreset
        
        synth = Synthesizer()
        
        print("Тест нот...")
        # Простые ноты
        for note in [60, 64, 67]:  # C, E, G
            print(f"Воспроизведение ноты {note}")
            synth.note_on(0, note, 0.8)
            time.sleep(0.5)
            synth.note_off(0, note)
            time.sleep(0.2)
        
        print("Тест барабанов...")
        # Барабаны
        drums = [DrumPreset.KICK, DrumPreset.SNARE, DrumPreset.HIHAT]
        for drum in drums:
            print(f"Воспроизведение {drum.value}")
            synth.play_drum(drum, 0.8)
            time.sleep(0.5)
        
        print("Тест завершен успешно!")
        
    except Exception as e:
        print(f"Ошибка в тесте синтезатора: {e}")

def main():
    """Основная функция"""
    print("Тест звука Python Synthesizer")
    print("=" * 40)
    
    try:
        # Тест pygame
        test_pygame_sound()
        time.sleep(1)
        
        # Тест синтезатора
        test_synthesizer()
        
    except Exception as e:
        print(f"Ошибка: {e}")
        print("Возможные причины:")
        print("1. pygame не установлен: pip install pygame")
        print("2. numpy не установлен: pip install numpy")
        print("3. Проблемы со звуковой картой")

if __name__ == "__main__":
    main()

