# MCU App (PlatformIO)

Progetto firmware PlatformIO per **ESP32-S3** con:
- profili di build per debug/release/production
- flusso opzionale di build basato su QEMU
- Standard C++20

## Cosa è attualmente implementato

- Modulo di logging in [src/logging.h](src/logging.h) e [src/logging.cpp](src/logging.cpp), che include:
  - livelli di log e prefissi
  - filtraggio dei log a compile-time in base al profilo di build
  - logging formattato tramite `std::format`
  - handler di log collegabili (predefinito: Arduino Serial)
- Script di supporto QEMU in [scripts/setup-qemu.py](scripts/setup-qemu.py), [scripts/merge-firmware.py](scripts/merge-firmware.py) e [scripts/skip-upload.py](scripts/skip-upload.py).

## Struttura del progetto

- [platformio.ini](platformio.ini): ambienti PlatformIO e hook degli script
- [src/](src): sorgenti firmware
- [scripts/](scripts): script Python di supporto per PlatformIO
- [qemu/](qemu): installazione/cache locale di QEMU

## Ambienti di build

Definiti in [platformio.ini](platformio.ini):

- `debug`: simboli di debug + `DEBUG=1`
- `release`: ottimizzato + `RELEASE=1`
- `production`: ottimizzato + `PRODUCTION=1`
- `qemu-debug`: flusso QEMU + flag debug
- `qemu-release`: flusso QEMU + flag release
- `qemu-production`: flusso QEMU + flag production

### Hook QEMU

Per gli ambienti QEMU, PlatformIO esegue:

- pre-build: [scripts/setup-qemu.py](scripts/setup-qemu.py)
  - scarica/estrae Espressif QEMU se manca
- post-build: [scripts/merge-firmware.py](scripts/merge-firmware.py)
  - unisce bootloader, partizioni e firmware in `qemu-image.bin`
- pre-upload: [scripts/skip-upload.py](scripts/skip-upload.py)
  - salta lo step di upload nei workflow con emulatore

## Prerequisiti

- Python 3
- PlatformIO Core (comando `pio`) oppure estensione PlatformIO IDE
- Toolchain compatibile con il pacchetto community `platform-espressif32` usato in [platformio.ini](platformio.ini)

## Comandi comuni

Esegui questi comandi dalla root del progetto.

Build per i profili hardware:

```sh
pio run -e debug
pio run -e release
pio run -e production
```

Build per i profili QEMU:

```sh
pio run -e qemu-debug
pio run -e qemu-release
pio run -e qemu-production
```

## Note

- Lo script QEMU supporta attualmente le convenzioni di naming host Linux, macOS e Windows.
- Il modulo di logging fa ampio uso dell'header e usa stato globale (`mcu::g_logBuffer`, `mcu::g_logHandlers`), utile per logging embedded semplice ma da rivedere per la concorrenza se più task loggano simultaneamente.
