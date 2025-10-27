#!/usr/bin/env python3
"""
Демонстрационный скрипт для Python Synthesizer
Показывает различные возможности синтезатора
"""

import time
import threading
from python_synthesizer import Synthesizer, DrumPreset, ADSR

def demo_basic_sounds():
    """Демонстрация базовых звуков"""
    print("=== Демонстрация базовых звуков ===")
    
    try:
        synth = Synthesizer()
        
        # Простые ноты
        print("Воспроизведение нот C4, E4, G4...")
        notes = [60, 64, 67]  # C4, E4, G4
        
        for note in notes:
            print(f"  Нота {note}")
            synth.note_on(0, note, 0.8)
            time.sleep(0.8)  # Увеличиваем время для лучшего восприятия
            synth.note_off(0, note)
            time.sleep(0.2)
        
        time.sleep(1)
        
        # Аккорд
        print("Воспроизведение аккорда C мажор...")
        chord_notes = [60, 64, 67]  # C, E, G
        
        for note in chord_notes:
            synth.note_on(0, note, 0.6)
        
        time.sleep(2)
        
        for note in chord_notes:
            synth.note_off(0, note)
        
        time.sleep(1)
        
    except Exception as e:
        print(f"Ошибка в демонстрации базовых звуков: {e}")

def demo_adsr_envelope():
    """Демонстрация ADSR огибающей"""
    print("=== Демонстрация ADSR огибающей ===")
    
    try:
        synth = Synthesizer()
        
        # Различные ADSR настройки
        adsr_presets = [
            ("Быстрая атака", ADSR(0.01, 0.05, 0.8, 0.1)),
            ("Медленная атака", ADSR(0.3, 0.2, 0.6, 0.5)),
            ("Длинный релиз", ADSR(0.05, 0.1, 0.7, 1.0)),
            ("Короткий звук", ADSR(0.01, 0.05, 0.3, 0.1))
        ]
        
        for name, adsr in adsr_presets:
            print(f"Воспроизведение: {name}")
            # Применяем ADSR перед воспроизведением
            synth.set_adsr(0, adsr)
            # Воспроизводим ноту с учетом ADSR
            synth.note_on(0, 60, 0.8)  # C4
            time.sleep(2.0)  # Увеличиваем время для лучшего восприятия ADSR
            synth.note_off(0, 60)
            time.sleep(0.5)
            
    except Exception as e:
        print(f"Ошибка в демонстрации ADSR: {e}")

def demo_drums():
    """Демонстрация барабанных звуков"""
    print("=== Демонстрация барабанных звуков ===")
    
    synth = Synthesizer()
    
    # Различные барабаны
    drums = [
        (DrumPreset.KICK, "Бас-барабан"),
        (DrumPreset.SNARE, "Малый барабан"),
        (DrumPreset.HIHAT, "Хай-хэт"),
        (DrumPreset.CRASH, "Крэш"),
        (DrumPreset.RIDE, "Райд"),
        (DrumPreset.TOM_HIGH, "Высокий том"),
        (DrumPreset.TOM_MID, "Средний том"),
        (DrumPreset.TOM_LOW, "Низкий том")
    ]
    
    for preset, name in drums:
        print(f"Воспроизведение: {name}")
        synth.play_drum(preset, 0.8)
        time.sleep(0.5)

def demo_rhythm_pattern():
    """Демонстрация ритмического паттерна"""
    print("=== Демонстрация ритмического паттерна ===")
    
    synth = Synthesizer()
    
    # Простой ритмический паттерн
    pattern = [
        (DrumPreset.KICK, 0.8),   # 1
        (None, 0),                 # 2
        (DrumPreset.SNARE, 0.6),  # 3
        (None, 0),                 # 4
        (DrumPreset.KICK, 0.8),   # 5
        (None, 0),                 # 6
        (DrumPreset.SNARE, 0.6),  # 7
        (None, 0),                 # 8
    ]
    
    print("Воспроизведение ритмического паттерна...")
    for _ in range(4):  # Повторяем 4 раза
        for drum, volume in pattern:
            if drum is not None:
                synth.play_drum(drum, volume)
            time.sleep(0.25)  # 16-я нота при 120 BPM

def demo_polyphony():
    """Демонстрация полифонии"""
    print("=== Демонстрация полифонии ===")
    
    synth = Synthesizer()
    
    # Воспроизведение нескольких нот одновременно
    print("Воспроизведение аккордов...")
    
    chords = [
        [60, 64, 67],  # C мажор
        [62, 65, 69],  # D мажор
        [64, 67, 71],  # E мажор
        [65, 69, 72],  # F мажор
    ]
    
    for chord in chords:
        print(f"Аккорд: {chord}")
        # Включаем все ноты аккорда
        for note in chord:
            synth.note_on(0, note, 0.6)
        
        time.sleep(1)
        
        # Выключаем все ноты
        for note in chord:
            synth.note_off(0, note)
        
        time.sleep(0.5)

def demo_melody():
    """Демонстрация мелодии"""
    print("=== Демонстрация мелодии ===")
    
    synth = Synthesizer()
    
    # Простая мелодия "Twinkle Twinkle Little Star"
    melody = [
        (60, 0.5),  # C
        (60, 0.5),  # C
        (67, 0.5),  # G
        (67, 0.5),  # G
        (69, 0.5),  # A
        (69, 0.5),  # A
        (67, 1.0),  # G
        (65, 0.5),  # F
        (65, 0.5),  # F
        (64, 0.5),  # E
        (64, 0.5),  # E
        (62, 0.5),  # D
        (62, 0.5),  # D
        (60, 1.0),  # C
    ]
    
    print("Воспроизведение мелодии 'Twinkle Twinkle Little Star'...")
    
    for note, duration in melody:
        synth.note_on(0, note, 0.7)
        time.sleep(duration)
        synth.note_off(0, note)
        time.sleep(0.1)

def demo_volume_control():
    """Демонстрация управления громкостью"""
    print("=== Демонстрация управления громкостью ===")
    
    synth = Synthesizer()
    
    # Изменение громкости
    volumes = [0.2, 0.4, 0.6, 0.8, 1.0]
    
    for volume in volumes:
        print(f"Громкость: {volume}")
        synth.set_master_volume(volume)
        synth.note_on(0, 60, 0.8)
        time.sleep(0.5)
        synth.note_off(0, 60)
        time.sleep(0.2)

def demo_channel_control():
    """Демонстрация управления каналами"""
    print("=== Демонстрация управления каналами ===")
    
    synth = Synthesizer()
    
    # Различные громкости для каналов
    synth.set_channel_volume(0, 1.0)
    synth.set_channel_volume(1, 0.8)
    synth.set_channel_volume(2, 0.6)
    
    print("Воспроизведение нот на разных каналах...")
    
    # Воспроизводим ноты на разных каналах
    for channel in range(3):
        print(f"Канал {channel}")
        synth.note_on(channel, 60 + channel * 2, 0.8)
        time.sleep(0.5)
        synth.note_off(channel, 60 + channel * 2)
        time.sleep(0.2)

def run_demo():
    """Запуск всех демонстраций"""
    print("Python Synthesizer Demo")
    print("=" * 50)
    
    demos = [
        ("Базовые звуки", demo_basic_sounds),
        ("ADSR огибающая", demo_adsr_envelope),
        ("Барабанные звуки", demo_drums),
        ("Ритмический паттерн", demo_rhythm_pattern),
        ("Полифония", demo_polyphony),
        ("Мелодия", demo_melody),
        ("Управление громкостью", demo_volume_control),
        ("Управление каналами", demo_channel_control),
    ]
    
    for name, demo_func in demos:
        print(f"\n{name}...")
        try:
            demo_func()
            print(f"✓ {name} завершена")
        except Exception as e:
            print(f"✗ Ошибка в {name}: {e}")
        
        time.sleep(1)
    
    print("\n" + "=" * 50)
    print("Демонстрация завершена!")

def interactive_demo():
    """Интерактивная демонстрация"""
    print("Интерактивная демонстрация Python Synthesizer")
    print("=" * 50)
    
    synth = Synthesizer()
    
    while True:
        print("\nДоступные команды:")
        print("1. note <номер> - воспроизвести MIDI ноту")
        print("2. drum <тип> - воспроизвести барабан")
        print("3. chord - воспроизвести аккорд")
        print("4. melody - воспроизвести мелодию")
        print("5. volume <0.0-1.0> - установить громкость")
        print("6. adsr - настроить ADSR")
        print("7. quit - выход")
        
        try:
            command = input("\n> ").strip().lower().split()
            
            if not command:
                continue
            
            if command[0] == "note" and len(command) > 1:
                note = int(command[1])
                if 0 <= note <= 127:
                    synth.note_on(0, note, 0.8)
                    print(f"Воспроизведена нота {note}")
                    time.sleep(1)
                    synth.note_off(0, note)
                else:
                    print("Номер ноты должен быть от 0 до 127")
            
            elif command[0] == "drum" and len(command) > 1:
                drum_type = command[1].upper()
                try:
                    preset = DrumPreset(drum_type.lower())
                    synth.play_drum(preset, 0.8)
                    print(f"Воспроизведен {drum_type}")
                except ValueError:
                    print("Неизвестный тип барабана")
            
            elif command[0] == "chord":
                chord_notes = [60, 64, 67]  # C мажор
                for note in chord_notes:
                    synth.note_on(0, note, 0.6)
                print("Воспроизведен аккорд C мажор")
                time.sleep(2)
                for note in chord_notes:
                    synth.note_off(0, note)
            
            elif command[0] == "melody":
                demo_melody()
            
            elif command[0] == "volume" and len(command) > 1:
                volume = float(command[1])
                if 0.0 <= volume <= 1.0:
                    synth.set_master_volume(volume)
                    print(f"Громкость установлена: {volume}")
                else:
                    print("Громкость должна быть от 0.0 до 1.0")
            
            elif command[0] == "adsr":
                print("Введите параметры ADSR (по умолчанию Enter):")
                attack = input("Attack (0.001-2.0): ") or "0.05"
                decay = input("Decay (0.001-2.0): ") or "0.1"
                sustain = input("Sustain (0.0-1.0): ") or "0.7"
                release = input("Release (0.001-2.0): ") or "0.2"
                
                try:
                    adsr = ADSR(float(attack), float(decay), float(sustain), float(release))
                    synth.set_adsr(0, adsr)
                    print("ADSR применен")
                except ValueError:
                    print("Неверные значения ADSR")
            
            elif command[0] == "quit":
                break
            
            else:
                print("Неизвестная команда")
        
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Ошибка: {e}")
    
    print("До свидания!")

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1 and sys.argv[1] == "interactive":
        interactive_demo()
    else:
        run_demo()
