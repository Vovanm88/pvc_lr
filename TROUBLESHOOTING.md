# Устранение неполадок Python Synthesizer

## Проблема: Мелодии и ноты не воспроизводятся

### Возможные причины и решения:

#### 1. Отсутствуют зависимости
**Симптомы:** Ошибки импорта, "ModuleNotFoundError"

**Решение:**
```bash
pip install numpy pygame
```

#### 2. Проблемы с pygame
**Симптомы:** Ошибки инициализации звука

**Решения:**
- Убедитесь, что pygame установлен: `pip install pygame`
- Проверьте версию: `python -c "import pygame; print(pygame.version.ver)"`
- Переустановите pygame: `pip uninstall pygame && pip install pygame`

#### 3. Проблемы со звуковой картой
**Симптомы:** Тишина, ошибки воспроизведения

**Решения:**
- Проверьте громкость системы
- Убедитесь, что звуковая карта работает
- Попробуйте другой аудио драйвер

#### 4. Проблемы с numpy
**Симптомы:** Ошибки при генерации звука

**Решение:**
```bash
pip install numpy
```

### Пошаговая диагностика:

#### Шаг 1: Быстрый тест
```bash
python quick_test.py
```

#### Шаг 2: Тест pygame
```bash
python test_sound.py
```

#### Шаг 3: Проверка зависимостей
```python
import numpy
import pygame
print("Все зависимости установлены")
```

### Специфичные проблемы:

#### Windows:
- Убедитесь, что установлен Visual C++ Redistributable
- Проверьте настройки звука в панели управления

#### Linux:
- Установите ALSA: `sudo apt-get install libasound2-dev`
- Проверьте права доступа к звуку

#### macOS:
- Убедитесь, что установлен Xcode Command Line Tools
- Проверьте настройки звука в System Preferences

### Отладка:

#### Включение отладочной информации:
```python
import pygame
pygame.mixer.pre_init(frequency=44100, size=-16, channels=2, buffer=512)
pygame.mixer.init()
print(f"Pygame mixer initialized: {pygame.mixer.get_init()}")
```

#### Проверка генерации звука:
```python
import numpy as np
import pygame

# Простой тест
sample_rate = 44100
duration = 1.0
frequency = 440

t = np.linspace(0, duration, int(duration * sample_rate))
samples = 0.5 * np.sin(2 * np.pi * frequency * t)
samples_16bit = (samples * 32767).astype(np.int16)
stereo_samples = np.column_stack((samples_16bit, samples_16bit))

sound = pygame.sndarray.make_sound(stereo_samples)
sound.play()
```

### Альтернативные решения:

#### Если pygame не работает:
1. Попробуйте `pip install pygame==2.1.0`
2. Используйте `pip install pygame --upgrade`
3. Переустановите: `pip uninstall pygame && pip install pygame`

#### Если numpy не работает:
1. Установите через conda: `conda install numpy`
2. Используйте pip с флагом: `pip install numpy --force-reinstall`

### Контакты для поддержки:

Если проблемы не решаются:
1. Проверьте версии Python (требуется 3.7+)
2. Убедитесь, что все зависимости установлены
3. Попробуйте в виртуальном окружении:
   ```bash
   python -m venv synth_env
   source synth_env/bin/activate  # Linux/Mac
   # или
   synth_env\Scripts\activate  # Windows
   pip install -r requirements.txt
   ```

