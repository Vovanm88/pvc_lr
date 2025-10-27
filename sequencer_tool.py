#!/usr/bin/env python3
"""
STM32 Sequencer Tool
Инструмент для работы с секвенсором через UART
"""

import serial
import sys
import time
import argparse
import csv
import os

class SequencerTool:
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_conn = None
        
    def connect(self):
        """Подключение к устройству"""
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS
            )
            print(f"Подключено к {self.port} на скорости {self.baudrate}")
            return True
        except serial.SerialException as e:
            print(f"Ошибка подключения: {e}")
            return False
    
    def disconnect(self):
        """Отключение от устройства"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("Отключено от устройства")
    
    def send_command(self, command):
        """Отправка команды устройству"""
        if not self.serial_conn or not self.serial_conn.is_open:
            print("Нет подключения к устройству")
            return False
        
        try:
            self.serial_conn.write(f"{command}\r\n".encode())
            time.sleep(0.1)  # Небольшая задержка
            return True
        except Exception as e:
            print(f"Ошибка отправки команды: {e}")
            return False
    
    def read_response(self, timeout=2):
        """Чтение ответа от устройства"""
        if not self.serial_conn or not self.serial_conn.is_open:
            return ""
        
        response = ""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            if self.serial_conn.in_waiting > 0:
                data = self.serial_conn.read(self.serial_conn.in_waiting)
                response += data.decode('utf-8', errors='ignore')
            time.sleep(0.01)
        
        return response.strip()
    
    def save_project(self, filename):
        """Сохранение проекта в файл"""
        print("Запрос сохранения проекта...")
        
        if not self.send_command("SAVE"):
            return False
        
        # Ждем ответ от устройства
        response = self.read_response(5)
        print("Ответ от устройства:")
        print(response)
        
        # Сохраняем в файл
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(response)
            print(f"Проект сохранен в {filename}")
            return True
        except Exception as e:
            print(f"Ошибка сохранения файла: {e}")
            return False
    
    def load_project(self, filename):
        """Загрузка проекта из файла"""
        if not os.path.exists(filename):
            print(f"Файл {filename} не найден")
            return False
        
        print(f"Загрузка проекта из {filename}...")
        
        try:
            with open(filename, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Отправляем команду загрузки
            if not self.send_command("LOAD"):
                return False
            
            # Отправляем содержимое файла
            lines = content.split('\n')
            for line in lines:
                if line.strip():
                    self.serial_conn.write(f"{line}\r\n".encode())
                    time.sleep(0.01)  # Небольшая задержка между строками
            
            # Ждем подтверждения
            response = self.read_response(3)
            print("Ответ от устройства:")
            print(response)
            
            if "OK" in response or "SUCCESS" in response:
                print("Проект успешно загружен")
                return True
            else:
                print("Ошибка загрузки проекта")
                return False
                
        except Exception as e:
            print(f"Ошибка загрузки файла: {e}")
            return False
    
    def monitor(self):
        """Мониторинг устройства в реальном времени"""
        print("Мониторинг устройства (Ctrl+C для выхода)...")
        print("=" * 50)
        
        try:
            while True:
                if self.serial_conn.in_waiting > 0:
                    data = self.serial_conn.read(self.serial_conn.in_waiting)
                    print(data.decode('utf-8', errors='ignore'), end='')
                time.sleep(0.01)
        except KeyboardInterrupt:
            print("\nМониторинг остановлен")
    
    def get_status(self):
        """Получение статуса устройства"""
        print("Запрос статуса...")
        
        if not self.send_command("s"):
            return False
        
        response = self.read_response(3)
        print("Статус устройства:")
        print(response)
        return True

def main():
    parser = argparse.ArgumentParser(description='STM32 Sequencer Tool')
    parser.add_argument('port', help='COM порт (например, COM3 или /dev/ttyUSB0)')
    parser.add_argument('command', choices=['save', 'load', 'monitor', 'status'], 
                       help='Команда для выполнения')
    parser.add_argument('filename', nargs='?', help='Имя файла для save/load команд')
    parser.add_argument('--baudrate', type=int, default=115200, 
                       help='Скорость передачи (по умолчанию 115200)')
    
    args = parser.parse_args()
    
    # Создаем инструмент
    tool = SequencerTool(args.port, args.baudrate)
    
    try:
        # Подключаемся
        if not tool.connect():
            return 1
        
        # Выполняем команду
        if args.command == 'save':
            if not args.filename:
                print("Для команды save необходимо указать имя файла")
                return 1
            success = tool.save_project(args.filename)
            
        elif args.command == 'load':
            if not args.filename:
                print("Для команды load необходимо указать имя файла")
                return 1
            success = tool.load_project(args.filename)
            
        elif args.command == 'monitor':
            tool.monitor()
            success = True
            
        elif args.command == 'status':
            success = tool.get_status()
        
        return 0 if success else 1
        
    except Exception as e:
        print(f"Ошибка: {e}")
        return 1
        
    finally:
        tool.disconnect()

if __name__ == "__main__":
    sys.exit(main())
