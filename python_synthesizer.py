#!/usr/bin/env python3
"""
Python-версия синтезатора мелодии для локальной настройки
Основан на STM32F4 проекте с поддержкой ADSR, полифонии и барабанов
"""

import numpy as np
import pygame
import threading
import time
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import json
import math

# Константы
SAMPLE_RATE = 44100
MAX_VOICES = 8
MAX_CHANNELS = 16
MAX_VOLUME = 10
MIDI_C4 = 60

@dataclass
class ADSR:
    """ADSR огибающая для синтезатора"""
    attack: float = 0.05    # Время атаки в секундах
    decay: float = 0.1      # Время спада в секундах
    sustain: float = 0.7    # Уровень сустейна (0.0-1.0)
    release: float = 0.2    # Время релиза в секундах
    
    def __post_init__(self):
        # Ограничиваем значения
        self.attack = max(0.001, min(2.0, self.attack))
        self.decay = max(0.001, min(2.0, self.decay))
        self.sustain = max(0.0, min(1.0, self.sustain))
        self.release = max(0.001, min(2.0, self.release))

@dataclass
class Voice:
    """Голос синтезатора"""
    frequency: float = 0.0
    velocity: float = 0.0      # Громкость (0.0-1.0)
    channel: int = 0
    start_time: float = 0.0
    release_time: float = 0.0
    active: bool = False
    released: bool = False
    adsr: ADSR = None
    
    def __post_init__(self):
        if self.adsr is None:
            self.adsr = ADSR()

class DrumPreset(Enum):
    """Пресеты барабанов"""
    KICK = "kick"
    SNARE = "snare"
    HIHAT = "hihat"
    CRASH = "crash"
    RIDE = "ride"
    TOM_HIGH = "tom_high"
    TOM_MID = "tom_mid"
    TOM_LOW = "tom_low"

@dataclass
class DrumSound:
    """Барабанный звук"""
    frequency: float
    duration: float     # Длительность в секундах
    volume: float
    is_noise: bool = False  # True для шумовых звуков

class Synthesizer:
    """Полифонический синтезатор с ADSR огибающей"""
    
    def __init__(self):
        self.voices: List[Voice] = [Voice() for _ in range(MAX_VOICES)]
        self.master_volume = 1.0
        self.channel_volumes = [1.0] * MAX_CHANNELS
        self.reverb_level = 0.0
        self.chorus_level = 0.0
        
        # Инициализация pygame для звука
        pygame.mixer.init(frequency=SAMPLE_RATE, size=-16, channels=2, buffer=512)
        
        # Барабанные пресеты
        self.drum_presets = {
            DrumPreset.KICK: DrumSound(60, 0.2, 0.8, False),
            DrumPreset.SNARE: DrumSound(200, 0.1, 0.7, True),
            DrumPreset.HIHAT: DrumSound(8000, 0.05, 0.6, True),
            DrumPreset.CRASH: DrumSound(5000, 0.3, 0.9, True),
            DrumPreset.RIDE: DrumSound(3000, 0.15, 0.7, True),
            DrumPreset.TOM_HIGH: DrumSound(400, 0.12, 0.7, False),
            DrumPreset.TOM_MID: DrumSound(200, 0.15, 0.8, False),
            DrumPreset.TOM_LOW: DrumSound(100, 0.2, 0.9, False)
        }
    
    def midi_to_frequency(self, note: int) -> float:
        """Конвертация MIDI ноты в частоту"""
        if note <= 0:
            return 0.0
        # A4 = 440 Hz, MIDI note 69
        return 440.0 * (2.0 ** ((note - 69) / 12.0))
    
    def find_free_voice(self) -> int:
        """Поиск свободного голоса"""
        for i, voice in enumerate(self.voices):
            if not voice.active:
                return i
        
        # Если нет свободных голосов, освобождаем самый старый
        oldest_voice = 0
        for i in range(1, MAX_VOICES):
            if self.voices[i].start_time < self.voices[oldest_voice].start_time:
                oldest_voice = i
        return oldest_voice
    
    def find_voice(self, channel: int, note: int) -> int:
        """Поиск голоса по каналу и ноте"""
        frequency = self.midi_to_frequency(note)
        for i, voice in enumerate(self.voices):
            if (voice.active and voice.channel == channel and 
                abs(voice.frequency - frequency) < 1.0):
                return i
        return MAX_VOICES
    
    def note_on(self, channel: int, note: int, velocity: float = 0.5):
        """Включение ноты"""
        if channel >= MAX_CHANNELS or note > 127:
            return
        
        voice_index = self.find_free_voice()
        voice = self.voices[voice_index]
        
        voice.frequency = self.midi_to_frequency(note)
        voice.velocity = min(1.0, max(0.0, velocity))
        voice.channel = channel
        voice.start_time = time.time()
        voice.release_time = 0.0
        voice.active = True
        voice.released = False
        voice.adsr = ADSR()  # Используем настройки по умолчанию
        
        # Воспроизводим звук с учетом ADSR
        self._play_note_with_adsr(voice)
    
    def note_off(self, channel: int, note: int):
        """Выключение ноты"""
        voice_index = self.find_voice(channel, note)
        if voice_index < MAX_VOICES:
            self.voices[voice_index].released = True
            self.voices[voice_index].release_time = time.time()
    
    def all_notes_off(self):
        """Выключение всех нот"""
        for voice in self.voices:
            voice.active = False
            voice.released = False
    
    def set_adsr(self, channel: int, adsr: ADSR):
        """Установка ADSR для канала"""
        for voice in self.voices:
            if voice.active and voice.channel == channel:
                voice.adsr = adsr
    
    def set_master_volume(self, volume: float):
        """Установка мастер-громкости"""
        self.master_volume = min(1.0, max(0.0, volume))
    
    def set_channel_volume(self, channel: int, volume: float):
        """Установка громкости канала"""
        if 0 <= channel < MAX_CHANNELS:
            self.channel_volumes[channel] = min(1.0, max(0.0, volume))
    
    def play_drum(self, preset: DrumPreset, velocity: float = 0.5):
        """Воспроизведение барабанного звука"""
        if preset in self.drum_presets:
            sound = self.drum_presets[preset]
            sound.volume = velocity
            self._generate_drum_sound(sound)
    
    def _generate_tone(self, frequency: float, volume: float, duration: float = 0.1):
        """Генерация тонального звука"""
        if frequency <= 0 or volume <= 0:
            return
        
        duration_samples = int(duration * SAMPLE_RATE)
        t = np.linspace(0, duration, duration_samples)
        
        # Генерируем синусоидальный тон
        samples = volume * np.sin(2 * np.pi * frequency * t)
        
        # Применяем огибающую для плавного затухания
        envelope = np.exp(-t * 3)
        samples *= envelope
        
        # Конвертируем в 16-bit и воспроизводим
        samples_16bit = (samples * 32767).astype(np.int16)
        stereo_samples = np.column_stack((samples_16bit, samples_16bit))
        
        sound_array = pygame.sndarray.make_sound(stereo_samples)
        pygame.mixer.Sound.play(sound_array)
    
    def _play_note_with_adsr(self, voice: Voice):
        """Воспроизведение ноты с ADSR огибающей"""
        if voice.frequency <= 0 or voice.velocity <= 0:
            return
        
        # Рассчитываем общую длительность звука
        total_duration = voice.adsr.attack + voice.adsr.decay + voice.adsr.release + 0.1
        
        duration_samples = int(total_duration * SAMPLE_RATE)
        t = np.linspace(0, total_duration, duration_samples)
        
        # Генерируем синусоидальный тон
        samples = voice.velocity * np.sin(2 * np.pi * voice.frequency * t)
        
        # Применяем ADSR огибающую
        envelope = np.zeros_like(t)
        
        for i, time_point in enumerate(t):
            if time_point < voice.adsr.attack:
                # Фаза Attack
                envelope[i] = time_point / voice.adsr.attack
            elif time_point < voice.adsr.attack + voice.adsr.decay:
                # Фаза Decay
                decay_time = time_point - voice.adsr.attack
                envelope[i] = 1.0 - (1.0 - voice.adsr.sustain) * (decay_time / voice.adsr.decay)
            else:
                # Фаза Sustain + Release
                sustain_time = time_point - voice.adsr.attack - voice.adsr.decay
                if sustain_time < 0.1:  # Короткий сустейн
                    envelope[i] = voice.adsr.sustain
                else:
                    # Фаза Release
                    release_time = sustain_time - 0.1
                    if release_time < voice.adsr.release:
                        envelope[i] = voice.adsr.sustain * (1.0 - release_time / voice.adsr.release)
                    else:
                        envelope[i] = 0.0
        
        # Применяем огибающую
        samples *= envelope
        
        # Применяем громкость канала и мастер-громкость
        final_volume = voice.velocity * self.channel_volumes[voice.channel] * self.master_volume
        samples *= final_volume
        
        # Конвертируем в 16-bit и воспроизводим
        samples_16bit = (samples * 32767).astype(np.int16)
        stereo_samples = np.column_stack((samples_16bit, samples_16bit))
        
        sound_array = pygame.sndarray.make_sound(stereo_samples)
        pygame.mixer.Sound.play(sound_array)
    
    def _generate_drum_sound(self, sound: DrumSound):
        """Генерация барабанного звука"""
        duration = int(sound.duration * SAMPLE_RATE)
        
        if sound.is_noise:
            # Генерируем шумовой звук
            samples = np.random.normal(0, sound.volume, duration)
        else:
            # Генерируем тональный звук
            t = np.linspace(0, sound.duration, duration)
            samples = sound.volume * np.sin(2 * np.pi * sound.frequency * t)
        
        # Применяем огибающую
        envelope = np.exp(-t * 5)  # Экспоненциальное затухание
        samples *= envelope
        
        # Конвертируем в 16-bit и воспроизводим
        samples_16bit = (samples * 32767).astype(np.int16)
        stereo_samples = np.column_stack((samples_16bit, samples_16bit))
        
        sound_array = pygame.sndarray.make_sound(stereo_samples)
        pygame.mixer.Sound.play(sound_array)
    
    def calculate_adsr_volume(self, voice: Voice) -> float:
        """Расчет громкости по ADSR огибающей"""
        current_time = time.time()
        elapsed = current_time - voice.start_time
        
        base_volume = voice.velocity
        adsr_volume = base_volume
        
        if voice.released:
            # Фаза Release
            release_elapsed = current_time - voice.release_time
            if release_elapsed >= voice.adsr.release:
                return 0.0  # Голос должен быть отключен
            
            # Линейное затухание в фазе Release
            adsr_volume = voice.adsr.sustain * (1.0 - release_elapsed / voice.adsr.release)
        else:
            # Фазы Attack, Decay, Sustain
            if elapsed < voice.adsr.attack:
                # Фаза Attack
                adsr_volume = base_volume * (elapsed / voice.adsr.attack)
            elif elapsed < voice.adsr.attack + voice.adsr.decay:
                # Фаза Decay
                decay_elapsed = elapsed - voice.adsr.attack
                adsr_volume = base_volume - ((base_volume - voice.adsr.sustain) * 
                                           decay_elapsed / voice.adsr.decay)
            else:
                # Фаза Sustain
                adsr_volume = voice.adsr.sustain
        
        # Применяем громкость канала и мастер-громкость
        adsr_volume *= self.channel_volumes[voice.channel] * self.master_volume
        
        return max(0.0, min(1.0, adsr_volume))
    
    def update_voices(self):
        """Обновление всех голосов"""
        for voice in self.voices:
            if voice.active:
                volume = self.calculate_adsr_volume(voice)
                
                if volume <= 0.0 and voice.released:
                    # Голос завершил фазу Release
                    voice.active = False
                elif volume > 0.0:
                    # Генерируем звук для активного голоса
                    self._generate_tone(voice.frequency, volume)
    
    def get_active_voices(self) -> int:
        """Получение количества активных голосов"""
        return sum(1 for voice in self.voices if voice.active)
    
    def is_channel_active(self, channel: int) -> bool:
        """Проверка активности канала"""
        return any(voice.active and voice.channel == channel for voice in self.voices)

class Sequencer:
    """Секвенсор для воспроизведения паттернов"""
    
    def __init__(self, synthesizer: Synthesizer):
        self.synthesizer = synthesizer
        self.bpm = 120
        self.volume = 0.8
        self.is_playing = False
        self.current_beat = 0
        self.last_beat_time = 0.0
        
        # Простой паттерн для демонстрации
        self.pattern = {
            'kick': [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0],
            'snare': [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0],
            'hihat': [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        }
        
        self.pattern_length = 16
    
    def set_bpm(self, bpm: int):
        """Установка темпа"""
        self.bpm = max(60, min(240, bpm))
    
    def set_volume(self, volume: float):
        """Установка громкости"""
        self.volume = max(0.0, min(1.0, volume))
    
    def play(self):
        """Запуск воспроизведения"""
        self.is_playing = True
        self.current_beat = 0
        self.last_beat_time = time.time()
        self._play_loop()
    
    def stop(self):
        """Остановка воспроизведения"""
        self.is_playing = False
        self.synthesizer.all_notes_off()
    
    def _play_loop(self):
        """Основной цикл воспроизведения"""
        while self.is_playing:
            current_time = time.time()
            beat_duration = 60.0 / (self.bpm * 4)  # Длительность 16-й ноты
            
            if current_time - self.last_beat_time >= beat_duration:
                self._play_beat()
                self.current_beat = (self.current_beat + 1) % self.pattern_length
                self.last_beat_time = current_time
            
            time.sleep(0.001)  # Небольшая задержка для снижения нагрузки на CPU
    
    def _play_beat(self):
        """Воспроизведение текущего такта"""
        # Kick drum
        if self.pattern['kick'][self.current_beat]:
            self.synthesizer.play_drum(DrumPreset.KICK, self.volume)
        
        # Snare drum
        if self.pattern['snare'][self.current_beat]:
            self.synthesizer.play_drum(DrumPreset.SNARE, self.volume * 0.8)
        
        # Hi-hat
        if self.pattern['hihat'][self.current_beat]:
            self.synthesizer.play_drum(DrumPreset.HIHAT, self.volume * 0.6)

def main():
    """Основная функция для демонстрации"""
    print("Python Synthesizer Demo")
    print("======================")
    
    # Создаем синтезатор
    synth = Synthesizer()
    
    # Создаем секвенсор
    sequencer = Sequencer(synth)
    
    print("Доступные команды:")
    print("1. play - запустить секвенсор")
    print("2. stop - остановить секвенсор")
    print("3. drum <type> - воспроизвести барабан")
    print("4. note <channel> <note> - воспроизвести ноту")
    print("5. bpm <value> - установить темп")
    print("6. volume <value> - установить громкость")
    print("7. quit - выход")
    print()
    
    while True:
        try:
            command = input("> ").strip().lower().split()
            
            if not command:
                continue
            
            if command[0] == "play":
                sequencer.play()
                print("Секвенсор запущен")
            
            elif command[0] == "stop":
                sequencer.stop()
                print("Секвенсор остановлен")
            
            elif command[0] == "drum" and len(command) > 1:
                drum_type = command[1].upper()
                try:
                    preset = DrumPreset(drum_type.lower())
                    synth.play_drum(preset)
                    print(f"Воспроизведен {drum_type}")
                except ValueError:
                    print("Неизвестный тип барабана")
            
            elif command[0] == "note" and len(command) > 2:
                try:
                    channel = int(command[1])
                    note = int(command[2])
                    synth.note_on(channel, note)
                    print(f"Воспроизведена нота {note} на канале {channel}")
                except ValueError:
                    print("Неверные параметры")
            
            elif command[0] == "bpm" and len(command) > 1:
                try:
                    bpm = int(command[1])
                    sequencer.set_bpm(bpm)
                    print(f"Темп установлен: {bpm} BPM")
                except ValueError:
                    print("Неверное значение BPM")
            
            elif command[0] == "volume" and len(command) > 1:
                try:
                    volume = float(command[1])
                    sequencer.set_volume(volume)
                    synth.set_master_volume(volume)
                    print(f"Громкость установлена: {volume}")
                except ValueError:
                    print("Неверное значение громкости")
            
            elif command[0] == "quit":
                sequencer.stop()
                break
            
            else:
                print("Неизвестная команда")
        
        except KeyboardInterrupt:
            sequencer.stop()
            break
        except Exception as e:
            print(f"Ошибка: {e}")
    
    print("До свидания!")

if __name__ == "__main__":
    main()
