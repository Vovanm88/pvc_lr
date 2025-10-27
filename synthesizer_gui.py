#!/usr/bin/env python3
"""
GUI версия синтезатора мелодии с настройками ADSR и барабанов
"""

import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time
import numpy as np
import pygame
from synthesizer import Synthesizer, DrumPreset, ADSR

class SynthesizerGUI:
    """GUI интерфейс для синтезатора"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Python Synthesizer")
        self.root.geometry("800x600")
        
        # Создаем синтезатор
        self.synthesizer = Synthesizer()
        self.sequencer_thread = None
        self.is_playing = False
        
        self.setup_ui()
        
    def setup_ui(self):
        """Настройка пользовательского интерфейса"""
        # Создаем notebook для вкладок
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill='both', expand=True, padx=10, pady=10)
        
        # Вкладка синтезатора
        self.setup_synthesizer_tab(notebook)
        
        # Вкладка барабанов
        self.setup_drums_tab(notebook)
        
        # Вкладка секвенсора
        self.setup_sequencer_tab(notebook)
        
        # Вкладка настроек
        self.setup_settings_tab(notebook)
    
    def setup_synthesizer_tab(self, notebook):
        """Вкладка синтезатора"""
        synth_frame = ttk.Frame(notebook)
        notebook.add(synth_frame, text="Синтезатор")
        
        # Громкость
        ttk.Label(synth_frame, text="Мастер громкость:").grid(row=0, column=0, sticky='w', padx=5, pady=5)
        self.master_volume_var = tk.DoubleVar(value=0.8)
        master_volume_scale = ttk.Scale(synth_frame, from_=0.0, to=1.0, 
                                       variable=self.master_volume_var, 
                                       orient='horizontal', length=200)
        master_volume_scale.grid(row=0, column=1, padx=5, pady=5)
        
        # ADSR настройки
        adsr_frame = ttk.LabelFrame(synth_frame, text="ADSR Огибающая")
        adsr_frame.grid(row=1, column=0, columnspan=2, sticky='ew', padx=5, pady=5)
        
        # Attack
        ttk.Label(adsr_frame, text="Attack (сек):").grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.attack_var = tk.DoubleVar(value=0.05)
        attack_scale = ttk.Scale(adsr_frame, from_=0.001, to=2.0, 
                                variable=self.attack_var, orient='horizontal', length=150)
        attack_scale.grid(row=0, column=1, padx=5, pady=2)
        ttk.Label(adsr_frame, textvariable=self.attack_var).grid(row=0, column=2, padx=5, pady=2)
        
        # Decay
        ttk.Label(adsr_frame, text="Decay (сек):").grid(row=1, column=0, sticky='w', padx=5, pady=2)
        self.decay_var = tk.DoubleVar(value=0.1)
        decay_scale = ttk.Scale(adsr_frame, from_=0.001, to=2.0, 
                               variable=self.decay_var, orient='horizontal', length=150)
        decay_scale.grid(row=1, column=1, padx=5, pady=2)
        ttk.Label(adsr_frame, textvariable=self.decay_var).grid(row=1, column=2, padx=5, pady=2)
        
        # Sustain
        ttk.Label(adsr_frame, text="Sustain:").grid(row=2, column=0, sticky='w', padx=5, pady=2)
        self.sustain_var = tk.DoubleVar(value=0.7)
        sustain_scale = ttk.Scale(adsr_frame, from_=0.0, to=1.0, 
                                variable=self.sustain_var, orient='horizontal', length=150)
        sustain_scale.grid(row=2, column=1, padx=5, pady=2)
        ttk.Label(adsr_frame, textvariable=self.sustain_var).grid(row=2, column=2, padx=5, pady=2)
        
        # Release
        ttk.Label(adsr_frame, text="Release (сек):").grid(row=3, column=0, sticky='w', padx=5, pady=2)
        self.release_var = tk.DoubleVar(value=0.2)
        release_scale = ttk.Scale(adsr_frame, from_=0.001, to=2.0, 
                                 variable=self.release_var, orient='horizontal', length=150)
        release_scale.grid(row=3, column=1, padx=5, pady=2)
        ttk.Label(adsr_frame, textvariable=self.release_var).grid(row=3, column=2, padx=5, pady=2)
        
        # Кнопки управления
        control_frame = ttk.Frame(synth_frame)
        control_frame.grid(row=2, column=0, columnspan=2, pady=10)
        
        ttk.Button(control_frame, text="Применить ADSR", 
                  command=self.apply_adsr).pack(side='left', padx=5)
        ttk.Button(control_frame, text="Применить громкость", 
                  command=self.apply_volume).pack(side='left', padx=5)
        ttk.Button(control_frame, text="Все ноты выключить", 
                  command=self.synthesizer.all_notes_off).pack(side='left', padx=5)
    
    def setup_drums_tab(self, notebook):
        """Вкладка барабанов"""
        drums_frame = ttk.Frame(notebook)
        notebook.add(drums_frame, text="Барабаны")
        
        # Список барабанов
        drums_list = [
            ("Kick", DrumPreset.KICK),
            ("Snare", DrumPreset.SNARE),
            ("Hi-hat", DrumPreset.HIHAT),
            ("Crash", DrumPreset.CRASH),
            ("Ride", DrumPreset.RIDE),
            ("Tom High", DrumPreset.TOM_HIGH),
            ("Tom Mid", DrumPreset.TOM_MID),
            ("Tom Low", DrumPreset.TOM_LOW)
        ]
        
        for i, (name, preset) in enumerate(drums_list):
            ttk.Button(drums_frame, text=name, 
                      command=lambda p=preset: self.play_drum(p)).grid(
                row=i//2, column=i%2, padx=5, pady=5, sticky='ew')
        
        # Настройки барабанов
        drums_settings = ttk.LabelFrame(drums_frame, text="Настройки барабанов")
        drums_settings.grid(row=4, column=0, columnspan=2, sticky='ew', padx=5, pady=5)
        
        ttk.Label(drums_settings, text="Громкость барабанов:").grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.drums_volume_var = tk.DoubleVar(value=0.8)
        drums_volume_scale = ttk.Scale(drums_settings, from_=0.0, to=1.0, 
                                      variable=self.drums_volume_var, orient='horizontal', length=200)
        drums_volume_scale.grid(row=0, column=1, padx=5, pady=2)
    
    def setup_sequencer_tab(self, notebook):
        """Вкладка секвенсора"""
        seq_frame = ttk.Frame(notebook)
        notebook.add(seq_frame, text="Секвенсор")
        
        # Управление воспроизведением
        control_frame = ttk.Frame(seq_frame)
        control_frame.grid(row=0, column=0, columnspan=2, pady=10)
        
        self.play_button = ttk.Button(control_frame, text="▶ Играть", 
                                     command=self.toggle_playback)
        self.play_button.pack(side='left', padx=5)
        
        ttk.Button(control_frame, text="⏹ Стоп", 
                  command=self.stop_playback).pack(side='left', padx=5)
        
        # Настройки темпа
        tempo_frame = ttk.LabelFrame(seq_frame, text="Настройки темпа")
        tempo_frame.grid(row=1, column=0, columnspan=2, sticky='ew', padx=5, pady=5)
        
        ttk.Label(tempo_frame, text="BPM:").grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.bpm_var = tk.IntVar(value=120)
        bpm_scale = ttk.Scale(tempo_frame, from_=60, to=240, 
                             variable=self.bpm_var, orient='horizontal', length=200)
        bpm_scale.grid(row=0, column=1, padx=5, pady=2)
        ttk.Label(tempo_frame, textvariable=self.bpm_var).grid(row=0, column=2, padx=5, pady=2)
        
        # Простой паттерн
        pattern_frame = ttk.LabelFrame(seq_frame, text="Паттерн")
        pattern_frame.grid(row=2, column=0, columnspan=2, sticky='ew', padx=5, pady=5)
        
        # Kick pattern
        ttk.Label(pattern_frame, text="Kick:").grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.kick_pattern_vars = []
        for i in range(16):
            var = tk.BooleanVar()
            self.kick_pattern_vars.append(var)
            ttk.Checkbutton(pattern_frame, variable=var).grid(row=0, column=i+1, padx=1, pady=2)
        
        # Snare pattern
        ttk.Label(pattern_frame, text="Snare:").grid(row=1, column=0, sticky='w', padx=5, pady=2)
        self.snare_pattern_vars = []
        for i in range(16):
            var = tk.BooleanVar()
            self.snare_pattern_vars.append(var)
            ttk.Checkbutton(pattern_frame, variable=var).grid(row=1, column=i+1, padx=1, pady=2)
        
        # Hi-hat pattern
        ttk.Label(pattern_frame, text="Hi-hat:").grid(row=2, column=0, sticky='w', padx=5, pady=2)
        self.hihat_pattern_vars = []
        for i in range(16):
            var = tk.BooleanVar()
            self.hihat_pattern_vars.append(var)
            ttk.Checkbutton(pattern_frame, variable=var).grid(row=2, column=i+1, padx=1, pady=2)
        
        # Кнопки паттерна
        pattern_buttons = ttk.Frame(pattern_frame)
        pattern_buttons.grid(row=3, column=0, columnspan=17, pady=5)
        
        ttk.Button(pattern_buttons, text="Очистить все", 
                  command=self.clear_pattern).pack(side='left', padx=5)
        ttk.Button(pattern_buttons, text="Заполнить все", 
                  command=self.fill_pattern).pack(side='left', padx=5)
        ttk.Button(pattern_buttons, text="Стандартный бит", 
                  command=self.set_standard_beat).pack(side='left', padx=5)
    
    def setup_settings_tab(self, notebook):
        """Вкладка настроек"""
        settings_frame = ttk.Frame(notebook)
        notebook.add(settings_frame, text="Настройки")
        
        # Настройки каналов
        channels_frame = ttk.LabelFrame(settings_frame, text="Громкость каналов")
        channels_frame.grid(row=0, column=0, sticky='ew', padx=5, pady=5)
        
        self.channel_volume_vars = []
        for i in range(8):  # Показываем только первые 8 каналов
            ttk.Label(channels_frame, text=f"Канал {i}:").grid(row=i, column=0, sticky='w', padx=5, pady=2)
            var = tk.DoubleVar(value=1.0)
            self.channel_volume_vars.append(var)
            scale = ttk.Scale(channels_frame, from_=0.0, to=1.0, 
                            variable=var, orient='horizontal', length=150)
            scale.grid(row=i, column=1, padx=5, pady=2)
            ttk.Label(channels_frame, textvariable=var).grid(row=i, column=2, padx=5, pady=2)
        
        # Эффекты
        effects_frame = ttk.LabelFrame(settings_frame, text="Эффекты")
        effects_frame.grid(row=1, column=0, sticky='ew', padx=5, pady=5)
        
        ttk.Label(effects_frame, text="Reverb:").grid(row=0, column=0, sticky='w', padx=5, pady=2)
        self.reverb_var = tk.DoubleVar(value=0.0)
        reverb_scale = ttk.Scale(effects_frame, from_=0.0, to=1.0, 
                                variable=self.reverb_var, orient='horizontal', length=150)
        reverb_scale.grid(row=0, column=1, padx=5, pady=2)
        
        ttk.Label(effects_frame, text="Chorus:").grid(row=1, column=0, sticky='w', padx=5, pady=2)
        self.chorus_var = tk.DoubleVar(value=0.0)
        chorus_scale = ttk.Scale(effects_frame, from_=0.0, to=1.0, 
                                variable=self.chorus_var, orient='horizontal', length=150)
        chorus_scale.grid(row=1, column=1, padx=5, pady=2)
        
        # Кнопки
        buttons_frame = ttk.Frame(settings_frame)
        buttons_frame.grid(row=2, column=0, pady=10)
        
        ttk.Button(buttons_frame, text="Применить настройки", 
                  command=self.apply_settings).pack(side='left', padx=5)
        ttk.Button(buttons_frame, text="Сбросить", 
                  command=self.reset_settings).pack(side='left', padx=5)
    
    def apply_adsr(self):
        """Применение ADSR настроек"""
        adsr = ADSR(
            attack=self.attack_var.get(),
            decay=self.decay_var.get(),
            sustain=self.sustain_var.get(),
            release=self.release_var.get()
        )
        self.synthesizer.set_adsr(0, adsr)  # Применяем к каналу 0
        print(f"ADSR применен: A={adsr.attack:.3f}, D={adsr.decay:.3f}, S={adsr.sustain:.3f}, R={adsr.release:.3f}")
    
    def apply_volume(self):
        """Применение громкости"""
        self.synthesizer.set_master_volume(self.master_volume_var.get())
        print(f"Громкость установлена: {self.master_volume_var.get():.2f}")
    
    def play_drum(self, preset: DrumPreset):
        """Воспроизведение барабана"""
        volume = self.drums_volume_var.get()
        self.synthesizer.play_drum(preset, volume)
        print(f"Воспроизведен {preset.value}")
    
    def toggle_playback(self):
        """Переключение воспроизведения"""
        if self.is_playing:
            self.stop_playback()
        else:
            self.start_playback()
    
    def start_playback(self):
        """Запуск воспроизведения"""
        self.is_playing = True
        self.play_button.config(text="⏸ Пауза")
        self.sequencer_thread = threading.Thread(target=self._sequencer_loop, daemon=True)
        self.sequencer_thread.start()
        print("Секвенсор запущен")
    
    def stop_playback(self):
        """Остановка воспроизведения"""
        self.is_playing = False
        self.play_button.config(text="▶ Играть")
        self.synthesizer.all_notes_off()
        print("Секвенсор остановлен")
    
    def _sequencer_loop(self):
        """Основной цикл секвенсора"""
        current_beat = 0
        last_beat_time = time.time()
        
        while self.is_playing:
            current_time = time.time()
            beat_duration = 60.0 / (self.bpm_var.get() * 4)  # 16-я нота
            
            if current_time - last_beat_time >= beat_duration:
                self._play_beat(current_beat)
                current_beat = (current_beat + 1) % 16
                last_beat_time = current_time
            
            time.sleep(0.001)
    
    def _play_beat(self, beat: int):
        """Воспроизведение такта"""
        # Kick
        if self.kick_pattern_vars[beat].get():
            self.synthesizer.play_drum(DrumPreset.KICK, self.drums_volume_var.get())
        
        # Snare
        if self.snare_pattern_vars[beat].get():
            self.synthesizer.play_drum(DrumPreset.SNARE, self.drums_volume_var.get() * 0.8)
        
        # Hi-hat
        if self.hihat_pattern_vars[beat].get():
            self.synthesizer.play_drum(DrumPreset.HIHAT, self.drums_volume_var.get() * 0.6)
    
    def clear_pattern(self):
        """Очистка паттерна"""
        for var in self.kick_pattern_vars + self.snare_pattern_vars + self.hihat_pattern_vars:
            var.set(False)
        print("Паттерн очищен")
    
    def fill_pattern(self):
        """Заполнение паттерна"""
        for var in self.kick_pattern_vars + self.snare_pattern_vars + self.hihat_pattern_vars:
            var.set(True)
        print("Паттерн заполнен")
    
    def set_standard_beat(self):
        """Установка стандартного бита"""
        self.clear_pattern()
        
        # Kick на 1, 5, 9, 13
        for i in [0, 4, 8, 12]:
            self.kick_pattern_vars[i].set(True)
        
        # Snare на 3, 7, 11, 15
        for i in [2, 6, 10, 14]:
            self.snare_pattern_vars[i].set(True)
        
        # Hi-hat на все
        for var in self.hihat_pattern_vars:
            var.set(True)
        
        print("Стандартный бит установлен")
    
    def apply_settings(self):
        """Применение настроек"""
        # Громкость каналов
        for i, var in enumerate(self.channel_volume_vars):
            self.synthesizer.set_channel_volume(i, var.get())
        
        # Эффекты
        self.synthesizer.reverb_level = self.reverb_var.get()
        self.synthesizer.chorus_level = self.chorus_var.get()
        
        print("Настройки применены")
    
    def reset_settings(self):
        """Сброс настроек"""
        self.master_volume_var.set(0.8)
        self.attack_var.set(0.05)
        self.decay_var.set(0.1)
        self.sustain_var.set(0.7)
        self.release_var.set(0.2)
        self.drums_volume_var.set(0.8)
        self.bpm_var.set(120)
        
        for var in self.channel_volume_vars:
            var.set(1.0)
        
        self.reverb_var.set(0.0)
        self.chorus_var.set(0.0)
        
        self.clear_pattern()
        print("Настройки сброшены")
    
    def run(self):
        """Запуск GUI"""
        try:
            self.root.mainloop()
        except KeyboardInterrupt:
            self.stop_playback()
            self.root.quit()

def main():
    """Основная функция"""
    print("Запуск Python Synthesizer GUI...")
    
    try:
        app = SynthesizerGUI()
        app.run()
    except Exception as e:
        print(f"Ошибка: {e}")
        messagebox.showerror("Ошибка", f"Произошла ошибка: {e}")

if __name__ == "__main__":
    main()
