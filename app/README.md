# MCU App (PlatformIO)

PlatformIO firmware project for **ESP32-S3** with:
- build profiles for debug/release/production
- optional QEMU-based build flow
- C++20 standard

## What Is Currently Implemented

- Logging module in [src/logging.h](src/logging.h) and [src/logging.cpp](src/logging.cpp), including:
  - log levels and prefixes
  - compile-time log filtering based on the build profile
  - formatted logging via `std::format`
  - pluggable log handlers (default: Arduino Serial)
- QEMU support scripts in [scripts/setup-qemu.py](scripts/setup-qemu.py), [scripts/merge-firmware.py](scripts/merge-firmware.py), and [scripts/skip-upload.py](scripts/skip-upload.py).

## Project Structure

- [platformio.ini](platformio.ini): PlatformIO environments and script hooks
- [src/](src): firmware sources
- [scripts/](scripts): PlatformIO support Python scripts
- [qemu/](qemu): local QEMU installation/cache

## Build Environments

Defined in [platformio.ini](platformio.ini):

- `debug`: debug symbols + `DEBUG=1`
- `release`: optimized + `RELEASE=1`
- `production`: optimized + `PRODUCTION=1`
- `qemu-debug`: QEMU flow + debug flags
- `qemu-release`: QEMU flow + release flags
- `qemu-production`: QEMU flow + production flags

### Hook QEMU

For QEMU environments, PlatformIO runs:

- pre-build: [scripts/setup-qemu.py](scripts/setup-qemu.py)
  - downloads/extracts Espressif QEMU if missing
- post-build: [scripts/merge-firmware.py](scripts/merge-firmware.py)
  - merges bootloader, partitions, and firmware into `qemu-image.bin`
- pre-upload: [scripts/skip-upload.py](scripts/skip-upload.py)
  - skips the upload step in emulator workflows

## Prerequisites

- Python 3
- PlatformIO Core (`pio` command) or PlatformIO IDE extension
- Toolchain compatible with the community `platform-espressif32` package used in [platformio.ini](platformio.ini)

## Common Commands

Run these commands from the project root.

Build for hardware profiles:

```sh
pio run -e debug
pio run -e release
pio run -e production
```

Build for QEMU profiles:

```sh
pio run -e qemu-debug
pio run -e qemu-release
pio run -e qemu-production
```

## Note

- The QEMU script currently supports Linux, macOS, and Windows host naming conventions.
- The logging module makes extensive use of the header and global state (`mcu::g_logBuffer`, `mcu::g_logHandlers`), which is useful for simple embedded logging but should be reviewed for concurrency if multiple tasks log simultaneously.
