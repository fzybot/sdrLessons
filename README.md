## Структура проекта

```md
"sdrLessons"
├── include
│   ├── ui                      # заголовочные файлы для работы UI
│   └── pluto_sdr_lib.h         # функции для работы с SDR
├── lib                         # функции по обработке сигналов (DSP)
│   ├── math_operations.h(cpp)  # математические операции
│   ├── modulation.h(cpp)       # модуляция
│   ├── pulse_shaping.h(cpp)    # согласованный фильтр
│   ├── time_sync.h(cpp)        # символьная синхронизация
│   └── freq_sync.h(cpp)        # частотная синхронизация
├── src
│   ├── ui                      # исполняемые файлы для работы UI
│   ├── pluto_sdr_lib.cpp       # реализация функций для работы с SDR
│   └── main.cpp                # ТОЧКА входа в программу
├── third_party                 # сторонние проекты\библиотеки и т.д.
│   ├── imgui
│   ├── implot
│   ├── libad9361-iio
│   ├── libiio
│   ├── SoapySDR
│   └── SoapyPlutoSDR
├── README.md
├── CMakeLists.txt
├── .gitignore
├── .gitmodules
└── и пр.
```

## Установка и запуск
### Сторонние зависимости
Установка зависимостей для `Dear ImGUI` (визуального интерфейса):
```bash
# OpenGL, SDL2, Glew
sudo apt install libsdl2-dev libgl1-mesa-dev libglew-dev
```

Установка зависимостей для работы `SDR`:
```bash
# Установка библиотеки для вычисления FFT
sudo apt-get install libfftw3-single3 libfftw3-dev
```


### Клонирование
```bash
git clone https://github.com/fzybot/sdrLessons.git
git submodule update --init --recursive # загружаем сабмодули
```

Компиляция и установка библиотек, необходимых для работы с **Adalm Pluto SDR** описана [здесь](https://github.com/fzybot/sdrLessons/blob/master/thirdparty/README.md) или в `/third_party/README.md`.


### Компиляция

```bash
mkdir build && cd build
cmake ../
make -j16
```
### Запуск двух SDR

Сначала нужно найти все устройства при помощи утилиты `SoapySDRUtil`:
```bash
SoapySDRUtil --find
```
пример вывода (подключены 2 `Adalm Pluto SDR`):
```bash
nsk@nsk-HVY-WXX9:~/dev/sdrLessons/build$ SoapySDRUtil --find
######################################################
##     Soapy SDR -- the SDR abstraction library     ##
######################################################

Found device 0
  device = PlutoSDR
  driver = plutosdr
  label = PlutoSDR #0 usb:1.4.5
  uri = usb:1.4.5

Found device 1
  device = PlutoSDR
  driver = plutosdr
  label = PlutoSDR #1 usb:3.4.5
  uri = usb:3.4.5

Found device 2
  device = PlutoSDR
  driver = plutosdr
  label = PlutoSDR #0 ip:pluto.local
  uri = ip:pluto.local
```

Далее, запускаем с аргументом `uri` устройства:
```bash
./main usb:1.4.5
```