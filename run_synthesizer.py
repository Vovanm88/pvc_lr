#!/usr/bin/env python3
"""
Скрипт для быстрого запуска Python Synthesizer
"""

import sys
import os

def check_dependencies():
    """Проверка зависимостей"""
    missing_deps = []
    
    try:
        import numpy
    except ImportError:
        missing_deps.append("numpy")
    
    try:
        import pygame
    except ImportError:
        missing_deps.append("pygame")
    
    if missing_deps:
        print("Отсутствуют зависимости:")
        for dep in missing_deps:
            print(f"  - {dep}")
        print("\nУстановите их командой:")
        print("pip install -r requirements.txt")
        return False
    
    return True

def main():
    """Основная функция"""
    print("Python Synthesizer Launcher")
    print("=" * 40)
    
    if not check_dependencies():
        return
    
    print("Выберите режим запуска:")
    print("1. Быстрый тест")
    print("2. Консольная версия")
    print("3. GUI версия")
    print("4. Демонстрация")
    print("5. Интерактивная демонстрация")
    print("6. Выход")
    
    while True:
        try:
            choice = input("\nВведите номер (1-6): ").strip()
            
            if choice == "1":
                print("Запуск быстрого теста...")
                os.system("python quick_test.py")
                break
            
            elif choice == "2":
                print("Запуск консольной версии...")
                os.system("python python_synthesizer.py")
                break
            
            elif choice == "3":
                print("Запуск GUI версии...")
                os.system("python synthesizer_gui.py")
                break
            
            elif choice == "4":
                print("Запуск демонстрации...")
                os.system("python demo_synthesizer.py")
                break
            
            elif choice == "5":
                print("Запуск интерактивной демонстрации...")
                os.system("python demo_synthesizer.py interactive")
                break
            
            elif choice == "6":
                print("До свидания!")
                break
            
            else:
                print("Неверный выбор. Введите число от 1 до 6.")
        
        except KeyboardInterrupt:
            print("\nДо свидания!")
            break
        except Exception as e:
            print(f"Ошибка: {e}")

if __name__ == "__main__":
    main()
