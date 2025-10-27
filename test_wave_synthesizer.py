#!/usr/bin/env python3
"""
Тест нового синтезатора с микшированием волн
Проверяет различные типы волн и полифонию
"""

import serial
import time
import struct
import numpy as np
import matplotlib.pyplot as plt

# Настройки UART
SERIAL_PORT = 'COM3'  # Измените на ваш порт
BAUD_RATE = 115200

def send_midi_note_on(ser, channel, note, velocity=64):
    """Отправка MIDI Note On"""
    midi_channel = 0x90 + channel
    data = bytes([midi_channel, note, velocity])
    ser.write(data)
    print(f"MIDI Note On: ch={channel}, note={note}, vel={velocity}")

def send_midi_note_off(ser, channel, note):
    """Отправка MIDI Note Off"""
    midi_channel = 0x80 + channel
    data = bytes([midi_channel, note, 0])
    ser.write(data)
    print(f"MIDI Note Off: ch={channel}, note={note}")

def send_wave_type_command(ser, channel, wave_type):
    """Отправка команды смены типа волны"""
    # Специальная команда для смены типа волны
    # Формат: 0xFF, channel, wave_type
    data = bytes([0xFF, channel, wave_type])
    ser.write(data)
    print(f"Wave Type: ch={channel}, type={wave_type}")

def test_wave_types():
    """Тест различных типов волн"""
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Подключено к {SERIAL_PORT} на скорости {BAUD_RATE}")
        
        time.sleep(2)
        
        print("\n=== Тест типов волн ===")
        
        # Тест синусоиды
        print("\n1. Синусоида (SINE)")
        send_wave_type_command(ser, 0, 0)  # SINE
        send_midi_note_on(ser, 0, 60, 80)
        time.sleep(2)
        send_midi_note_off(ser, 0, 60)
        time.sleep(1)
        
        # Тест прямоугольной волны
        print("\n2. Прямоугольная волна (SQUARE)")
        send_wave_type_command(ser, 0, 1)  # SQUARE
        send_midi_note_on(ser, 0, 60, 80)
        time.sleep(2)
        send_midi_note_off(ser, 0, 60)
        time.sleep(1)
        
        # Тест пилообразной волны
        print("\n3. Пилообразная волна (SAWTOOTH)")
        send_wave_type_command(ser, 0, 2)  # SAWTOOTH
        send_midi_note_on(ser, 0, 60, 80)
        time.sleep(2)
        send_midi_note_off(ser, 0, 60)
        time.sleep(1)
        
        # Тест треугольной волны
        print("\n4. Треугольная волна (TRIANGLE)")
        send_wave_type_command(ser, 0, 3)  # TRIANGLE
        send_midi_note_on(ser, 0, 60, 80)
        time.sleep(2)
        send_midi_note_off(ser, 0, 60)
        time.sleep(1)
        
        # Тест шума
        print("\n5. Шум (NOISE)")
        send_wave_type_command(ser, 0, 4)  # NOISE
        send_midi_note_on(ser, 0, 60, 80)
        time.sleep(2)
        send_midi_note_off(ser, 0, 60)
        time.sleep(1)
        
        print("\n=== Тест завершен ===")
        
    except serial.SerialException as e:
        print(f"Ошибка подключения: {e}")
    except KeyboardInterrupt:
        print("\nТест прерван")
    finally:
        if 'ser' in locals():
            ser.close()

def test_polyphony_with_waves():
    """Тест полифонии с разными типами волн"""
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Подключено к {SERIAL_PORT} на скорости {BAUD_RATE}")
        
        time.sleep(2)
        
        print("\n=== Тест полифонии с разными волнами ===")
        
        # Каждый канал - свой тип волны
        wave_types = [0, 1, 2, 3]  # SINE, SQUARE, SAWTOOTH, TRIANGLE
        notes = [60, 64, 67, 72]  # C4, E4, G4, C5
        
        print("\nНастраиваем разные типы волн для каналов:")
        for i, wave_type in enumerate(wave_types):
            send_wave_type_command(ser, i, wave_type)
            time.sleep(0.1)
        
        print("\nИграем аккорд с разными волнами:")
        for i, note in enumerate(notes):
            send_midi_note_on(ser, i, note, 70)
            time.sleep(0.1)
        
        time.sleep(3)
        
        print("Отключаем все ноты:")
        for i, note in enumerate(notes):
            send_midi_note_off(ser, i, note)
            time.sleep(0.1)
        
        time.sleep(1)
        
        print("\n=== Тест завершен ===")
        
    except serial.SerialException as e:
        print(f"Ошибка подключения: {e}")
    except KeyboardInterrupt:
        print("\nТест прерван")
    finally:
        if 'ser' in locals():
            ser.close()

def generate_wave_samples():
    """Генерация образцов волн для сравнения"""
    sample_rate = 44100
    duration = 0.1  # 100ms
    frequency = 440  # A4
    
    t = np.linspace(0, duration, int(sample_rate * duration))
    
    # Генерируем различные типы волн
    sine_wave = np.sin(2 * np.pi * frequency * t)
    square_wave = np.sign(np.sin(2 * np.pi * frequency * t))
    sawtooth_wave = 2 * (t * frequency - np.floor(t * frequency + 0.5))
    triangle_wave = 2 * np.abs(2 * (t * frequency - np.floor(t * frequency + 0.5))) - 1
    
    # Создаем график
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    fig.suptitle('Типы волн синтезатора')
    
    axes[0, 0].plot(t[:1000], sine_wave[:1000])
    axes[0, 0].set_title('Синусоида')
    axes[0, 0].set_ylabel('Амплитуда')
    
    axes[0, 1].plot(t[:1000], square_wave[:1000])
    axes[0, 1].set_title('Прямоугольная')
    
    axes[1, 0].plot(t[:1000], sawtooth_wave[:1000])
    axes[1, 0].set_title('Пилообразная')
    axes[1, 0].set_xlabel('Время (с)')
    axes[1, 0].set_ylabel('Амплитуда')
    
    axes[1, 1].plot(t[:1000], triangle_wave[:1000])
    axes[1, 1].set_title('Треугольная')
    axes[1, 1].set_xlabel('Время (с)')
    
    plt.tight_layout()
    plt.savefig('wave_samples.png')
    print("Образцы волн сохранены в wave_samples.png")
    
    return sine_wave, square_wave, sawtooth_wave, triangle_wave

if __name__ == "__main__":
    print("Тест синтезатора с микшированием волн")
    print("1. Тест типов волн")
    print("2. Тест полифонии с разными волнами")
    print("3. Генерация образцов волн")
    
    choice = input("\nВыберите тест (1-3): ")
    
    if choice == "1":
        test_wave_types()
    elif choice == "2":
        test_polyphony_with_waves()
    elif choice == "3":
        generate_wave_samples()
    else:
        print("Неверный выбор")
