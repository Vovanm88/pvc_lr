#!/usr/bin/env python3
"""
Тест полифонии MIDI-синтезатора
Проверяет, что несколько нот могут играть одновременно
"""

import serial
import time
import struct

# Настройки UART
SERIAL_PORT = 'COM3'  # Измените на ваш порт
BAUD_RATE = 115200

def send_midi_note_on(ser, channel, note, velocity=64):
    """Отправка MIDI Note On"""
    # MIDI Note On: 0x90 + channel, note, velocity
    midi_channel = 0x90 + channel
    data = bytes([midi_channel, note, velocity])
    ser.write(data)
    print(f"MIDI Note On: ch={channel}, note={note}, vel={velocity}")

def send_midi_note_off(ser, channel, note):
    """Отправка MIDI Note Off"""
    # MIDI Note Off: 0x80 + channel, note, velocity=0
    midi_channel = 0x80 + channel
    data = bytes([midi_channel, note, 0])
    ser.write(data)
    print(f"MIDI Note Off: ch={channel}, note={note}")

def test_polyphony():
    """Тест полифонии - несколько нот одновременно"""
    try:
        # Подключение к UART
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Подключено к {SERIAL_PORT} на скорости {BAUD_RATE}")
        
        # Ждем инициализации
        time.sleep(2)
        
        print("\n=== Тест полифонии MIDI-синтезатора ===")
        print("Проверяем, что несколько нот могут играть одновременно")
        
        # Тест 1: Аккорд из 3 нот
        print("\n1. Играем аккорд C-E-G (до-ми-соль)")
        send_midi_note_on(ser, 0, 60, 80)  # C4
        time.sleep(0.1)
        send_midi_note_on(ser, 0, 64, 80)  # E4
        time.sleep(0.1)
        send_midi_note_on(ser, 0, 67, 80)  # G4
        time.sleep(2)
        
        print("Отключаем аккорд")
        send_midi_note_off(ser, 0, 60)
        send_midi_note_off(ser, 0, 64)
        send_midi_note_off(ser, 0, 67)
        time.sleep(1)
        
        # Тест 2: Последовательные ноты (должны накладываться)
        print("\n2. Последовательные ноты (полифония)")
        send_midi_note_on(ser, 0, 60, 70)  # C4
        time.sleep(0.5)
        send_midi_note_on(ser, 0, 64, 70)  # E4
        time.sleep(0.5)
        send_midi_note_on(ser, 0, 67, 70)  # G4
        time.sleep(0.5)
        send_midi_note_on(ser, 0, 72, 70)  # C5
        time.sleep(2)
        
        print("Отключаем все ноты")
        send_midi_note_off(ser, 0, 60)
        send_midi_note_off(ser, 0, 64)
        send_midi_note_off(ser, 0, 67)
        send_midi_note_off(ser, 0, 72)
        time.sleep(1)
        
        # Тест 3: Разные каналы
        print("\n3. Разные MIDI каналы")
        send_midi_note_on(ser, 0, 60, 60)  # C4 на канале 0
        send_midi_note_on(ser, 1, 64, 60)  # E4 на канале 1
        send_midi_note_on(ser, 2, 67, 60)  # G4 на канале 2
        time.sleep(2)
        
        print("Отключаем все ноты")
        send_midi_note_off(ser, 0, 60)
        send_midi_note_off(ser, 1, 64)
        send_midi_note_off(ser, 2, 67)
        time.sleep(1)
        
        # Тест 4: Максимальная полифония
        print("\n4. Максимальная полифония (8 голосов)")
        notes = [60, 62, 64, 65, 67, 69, 71, 72]  # C4, D4, E4, F4, G4, A4, B4, C5
        for i, note in enumerate(notes):
            send_midi_note_on(ser, 0, note, 50)
            time.sleep(0.1)
        time.sleep(2)
        
        print("Отключаем все ноты")
        for note in notes:
            send_midi_note_off(ser, 0, note)
        time.sleep(1)
        
        print("\n=== Тест завершен ===")
        print("Проверьте, что:")
        print("- Аккорды играют одновременно")
        print("- Последовательные ноты накладываются")
        print("- Разные каналы работают независимо")
        print("- Максимум 8 голосов могут играть одновременно")
        
    except serial.SerialException as e:
        print(f"Ошибка подключения к {SERIAL_PORT}: {e}")
        print("Убедитесь, что:")
        print("1. Устройство подключено")
        print("2. Порт указан правильно")
        print("3. Драйверы установлены")
    except KeyboardInterrupt:
        print("\nТест прерван пользователем")
    finally:
        if 'ser' in locals():
            ser.close()
            print("Соединение закрыто")

if __name__ == "__main__":
    test_polyphony()
