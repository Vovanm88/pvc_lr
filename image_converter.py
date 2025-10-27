#!/usr/bin/env python3
"""
Image Converter for STM32 Sequencer
Конвертер изображений в hex-данные для OLED дисплея 128x64
"""

import sys
import argparse
from PIL import Image
import numpy as np

class ImageConverter:
    def __init__(self, width=128, height=64):
        self.width = width
        self.height = height
        
    def load_image(self, filename):
        """Загрузка изображения"""
        try:
            img = Image.open(filename)
            return img
        except Exception as e:
            print(f"Ошибка загрузки изображения: {e}")
            return None
    
    def resize_image(self, img):
        """Изменение размера изображения до 128x64"""
        return img.resize((self.width, self.height), Image.Resampling.LANCZOS)
    
    def convert_to_monochrome(self, img):
        """Конвертация в монохромное изображение"""
        # Конвертируем в оттенки серого
        gray = img.convert('L')
        
        # Применяем пороговое значение для получения черно-белого изображения
        threshold = 128
        mono = gray.point(lambda x: 0 if x < threshold else 255, mode='1')
        
        return mono
    
    def image_to_hex_array(self, img):
        """Конвертация изображения в массив hex-значений"""
        # Конвертируем в numpy массив
        img_array = np.array(img)
        
        # Инвертируем цвета (0 = черный, 1 = белый)
        img_array = 1 - img_array
        
        # Конвертируем в формат для OLED (8 пикселей на байт)
        hex_data = []
        
        for y in range(0, self.height, 8):  # Обрабатываем по 8 строк
            for x in range(self.width):
                byte_value = 0
                for bit in range(8):
                    if y + bit < self.height:
                        if img_array[y + bit, x]:
                            byte_value |= (1 << bit)
                hex_data.append(byte_value)
        
        return hex_data
    
    def generate_c_header(self, hex_data, array_name="image_data"):
        """Генерация C header файла"""
        lines = []
        lines.append(f"// Generated image data for {array_name}")
        lines.append(f"// Size: {self.width}x{self.height} pixels")
        lines.append(f"// Total bytes: {len(hex_data)}")
        lines.append("")
        lines.append(f"#ifndef {array_name.upper()}_H")
        lines.append(f"#define {array_name.upper()}_H")
        lines.append("")
        lines.append(f"#include <stdint.h>")
        lines.append("")
        lines.append(f"const uint8_t {array_name}[] = {{")
        
        # Разбиваем на строки по 16 байт
        for i in range(0, len(hex_data), 16):
            line = "    "
            for j in range(16):
                if i + j < len(hex_data):
                    line += f"0x{hex_data[i + j]:02X}"
                    if i + j < len(hex_data) - 1:
                        line += ", "
            if i + 16 >= len(hex_data):
                line += "  // Last line"
            lines.append(line)
        
        lines.append("};")
        lines.append("")
        lines.append(f"#endif // {array_name.upper()}_H")
        
        return "\n".join(lines)
    
    def convert_image(self, input_file, output_file=None, array_name="image_data"):
        """Полная конвертация изображения"""
        print(f"Конвертация {input_file}...")
        
        # Загружаем изображение
        img = self.load_image(input_file)
        if img is None:
            return False
        
        print(f"Исходный размер: {img.size}")
        
        # Изменяем размер
        img = self.resize_image(img)
        print(f"Новый размер: {img.size}")
        
        # Конвертируем в монохромное
        mono_img = self.convert_to_monochrome(img)
        
        # Конвертируем в hex-массив
        hex_data = self.image_to_hex_array(mono_img)
        print(f"Сгенерировано {len(hex_data)} байт данных")
        
        # Генерируем C header
        c_header = self.generate_c_header(hex_data, array_name)
        
        # Сохраняем результат
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(c_header)
                print(f"Результат сохранен в {output_file}")
            except Exception as e:
                print(f"Ошибка сохранения файла: {e}")
                return False
        else:
            # Выводим в stdout
            print(c_header)
        
        return True

def create_sample_images():
    """Создание примеров изображений для тестирования"""
    from PIL import Image, ImageDraw, ImageFont
    
    # Создаем заставку
    splash = Image.new('RGB', (128, 64), 'black')
    draw = ImageDraw.Draw(splash)
    
    # Рисуем заголовок
    try:
        font = ImageFont.truetype("arial.ttf", 16)
    except:
        font = ImageFont.load_default()
    
    draw.text((10, 10), "STM32", fill='white', font=font)
    draw.text((10, 30), "SEQUENCER", fill='white', font=font)
    draw.text((10, 50), "v1.0", fill='white', font=font)
    
    splash.save("splash_sample.png")
    print("Создан пример заставки: splash_sample.png")
    
    # Создаем экран справки
    help_img = Image.new('RGB', (128, 64), 'black')
    draw = ImageDraw.Draw(help_img)
    
    draw.text((5, 5), "HELP", fill='white', font=font)
    draw.text((5, 20), "1-4: Drums", fill='white', font=font)
    draw.text((5, 30), "A/B: Save/Load", fill='white', font=font)
    draw.text((5, 40), "OK: Select", fill='white', font=font)
    draw.text((5, 50), "Back: Exit", fill='white', font=font)
    
    help_img.save("help_sample.png")
    print("Создан пример справки: help_sample.png")

def main():
    parser = argparse.ArgumentParser(description='STM32 Sequencer Image Converter')
    parser.add_argument('input', help='Входной файл изображения')
    parser.add_argument('-o', '--output', help='Выходной файл (.h)')
    parser.add_argument('-n', '--name', default='image_data', 
                       help='Имя массива в C коде')
    parser.add_argument('--create-samples', action='store_true',
                       help='Создать примеры изображений')
    
    args = parser.parse_args()
    
    if args.create_samples:
        create_sample_images()
        return 0
    
    if not args.input:
        print("Необходимо указать входной файл")
        return 1
    
    # Создаем конвертер
    converter = ImageConverter()
    
    # Выполняем конвертацию
    success = converter.convert_image(
        args.input, 
        args.output, 
        args.name
    )
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
