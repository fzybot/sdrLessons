## Установка

```bash
git clone https://github.com/fzybot/sdrLessons.git
```

### Запуск

Сначала нужно найти все устройства при помощи утилиты `SoapySDRUtil`:
```bash
SoapySDRUtil --find
```
пример вывода (подключены 2 SDR):
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