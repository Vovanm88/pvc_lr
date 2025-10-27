#!/usr/bin/env python3
"""
STM32 Debug UART Monitor
Мониторинг отладочного вывода через UART для STM32 проекта
"""

import sys
import time
import threading
import serial
import serial.tools.list_ports


def reader(port: serial.Serial):
    """Поток для чтения данных из UART"""
    try:
        while True:
            b = port.read(1)
            if not b:
                continue
            sys.stdout.write(b.decode(errors='ignore'))
            sys.stdout.flush()
    except KeyboardInterrupt:
        pass


def list_ports():
    """Список доступных COM портов"""
    ports = serial.tools.list_ports.comports()
    print("Доступные COM порты:")
    if not ports:
        print("  COM порты не найдены!")
    else:
        for i, port in enumerate(ports):
            print(f"{i+1}. {port.device} - {port.description}")
    return ports


def select_port():
    """Интерактивный выбор COM порта"""
    ports = list_ports()
    if not ports:
        print("COM порты не найдены!")
        return None
    
    # Попробуем найти STM32 порт автоматически
    stm32_ports = []
    for port in ports:
        desc = port.description.lower()
        if any(keyword in desc for keyword in ['stm32', 'st-link', 'debug', 'uart', 'serial']):
            stm32_ports.append(port)
    
    if stm32_ports:
        print(f"\nНайдены возможные STM32 порты:")
        for i, port in enumerate(stm32_ports):
            print(f"  {i+1}. {port.device} - {port.description}")
        print(f"  {len(stm32_ports)+1}. Показать все порты")
        
    while True:
        try:
            if stm32_ports:
                choice = input(f"\nВыберите порт (1-{len(stm32_ports)+1}) или введите полное имя (например COM7): ")
            else:
                choice = input(f"\nВыберите порт (1-{len(ports)}) или введите полное имя (например COM7): ")
            
            # Проверяем, является ли ввод числом
            if choice.isdigit():
                choice_num = int(choice)
                if stm32_ports and choice_num == len(stm32_ports) + 1:
                    # Показать все порты
                    list_ports()
                    continue
                elif stm32_ports and 1 <= choice_num <= len(stm32_ports):
                    return stm32_ports[choice_num - 1].device
                elif not stm32_ports and 1 <= choice_num <= len(ports):
                    return ports[choice_num - 1].device
                else:
                    print("Неверный номер порта!")
                    continue
            else:
                # Проверяем, существует ли такой порт
                for port in ports:
                    if port.device.upper() == choice.upper():
                        return port.device
                print("Порт не найден!")
                continue
                
        except KeyboardInterrupt:
            print("\nВыход...")
            return None
        except Exception as e:
            print(f"Ошибка: {e}")
            continue


def main():
    print("=== STM32 Debug UART Monitor ===")
    print("Мониторинг отладочного вывода STM32 через UART")
    print()
    
    # Определяем порт и скорость
    if len(sys.argv) >= 2:
        com = sys.argv[1]
        baud = int(sys.argv[2]) if len(sys.argv) >= 3 else 115200
    else:
        com = select_port()
        if com is None:
            return
        baud = 115200
    
    print(f"\nПодключение к {com} на скорости {baud}...")
    
    try:
        with serial.Serial(com, baudrate=baud, timeout=0.05) as ser:
            print(f"Подключено к {com}")
            print("Ожидание данных... (Ctrl+C для выхода)")
            print("Можно вводить команды для отправки в STM32")
            print("-" * 50)
            
            # Запускаем поток для чтения
            t = threading.Thread(target=reader, args=(ser,), daemon=True)
            t.start()
            
            # Основной поток для отправки данных
            try:
                while True:
                    s = sys.stdin.read(1)
                    if not s:
                        time.sleep(0.01)
                        continue
                    ser.write(s.encode())
            except KeyboardInterrupt:
                print("\n\nМониторинг остановлен пользователем")
                
    except serial.SerialException as e:
        print(f"Ошибка подключения к {com}: {e}")
    except Exception as e:
        print(f"Неожиданная ошибка: {e}")


if __name__ == "__main__":
    main()
