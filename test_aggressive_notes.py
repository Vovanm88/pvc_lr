#!/usr/bin/env python3
"""
Агрессивный тест для проверки смены нот
"""

import serial
import time
import sys

def test_aggressive_notes():
    """Тестирует агрессивную смену нот"""
    
    try:
        # Подключаемся к UART
        ser = serial.Serial('COM3', 115200, timeout=1)  # Измените порт при необходимости
        time.sleep(2)  # Ждем инициализации
        
        print("=== АГРЕССИВНЫЙ ТЕСТ НОТ ===")
        print("Отправляем команды на STM32...")
        
        # Очищаем буфер
        ser.reset_input_buffer()
        
        # Отправляем команду статуса
        ser.write(b's\r\n')
        time.sleep(0.5)
        
        # Читаем ответ
        response = ser.read_all().decode('utf-8', errors='ignore')
        print("Ответ от STM32:")
        print(response)
        
        # Запускаем воспроизведение
        print("\nЗапускаем воспроизведение...")
        ser.write(b'p\r\n')
        time.sleep(0.5)
        
        # Читаем ответ
        response = ser.read_all().decode('utf-8', errors='ignore')
        print("Ответ от STM32:")
        print(response)
        
        # Ждем и наблюдаем за отладочным выводом
        print("\nНаблюдаем за АГРЕССИВНЫМ отладочным выводом в течение 15 секунд...")
        print("Должны быть сообщения о смене нот и принудительных обновлениях Buzzer")
        start_time = time.time()
        
        while time.time() - start_time < 15:
            if ser.in_waiting > 0:
                response = ser.read_all().decode('utf-8', errors='ignore')
                if response.strip():
                    print(f"[{time.time() - start_time:.1f}s] {response.strip()}")
            time.sleep(0.1)
        
        # Останавливаем воспроизведение
        print("\nОстанавливаем воспроизведение...")
        ser.write(b'p\r\n')
        time.sleep(0.5)
        
        # Читаем ответ
        response = ser.read_all().decode('utf-8', errors='ignore')
        print("Ответ от STM32:")
        print(response)
        
        ser.close()
        print("\nАгрессивный тест завершен!")
        print("\nОжидаемые сообщения:")
        print("- 'MIDI: noteOn ch=X, note=Y, freq=Z, vel=W, voice=V'")
        print("- 'FORCED Buzzer update: freq=X, vol=Y'")
        print("- 'Stopping old voice X for new note'")
        print("- 'mixVoices: X active voices, freq=Y, vol=Z'")
        
    except serial.SerialException as e:
        print(f"Ошибка подключения к UART: {e}")
        print("Проверьте:")
        print("1. Подключен ли STM32 к компьютеру")
        print("2. Правильный ли COM-порт")
        print("3. Настроена ли скорость 115200")
        
    except Exception as e:
        print(f"Ошибка: {e}")

if __name__ == "__main__":
    test_aggressive_notes()
