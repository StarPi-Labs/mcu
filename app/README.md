# MCU App (PlatformIO)

PlatformIO firmware project for **ESP32-S3** with:
- build profiles for debug/release/production
- optional QEMU-based build flow
- C++26 standard
- FreeRTOS-based synchronization primitives
- comprehensive test suite

## What Is Currently Implemented

### Logging Module
Located in [lib/core/](lib/core/), comprising:
- [Logging.h](lib/core/Logging.h) and [Logging.cpp](lib/core/Logging.cpp)
- log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL, NONE)
- compile-time log filtering based on the build profile
- formatted logging via `std::format`
- pluggable log handlers (default: Arduino Serial)
- double-buffering for efficient concurrent logging
- optional timestamp support

### FreeRTOS Synchronization Primitives
Located in [lib/freeRTOS/](lib/freeRTOS/):

- **Mutex**: Standard mutual exclusion lock with BasicLockable, Lockable, and TimedLockable concepts
  - `lock()` - blocking acquisition
  - `try_lock()` - non-blocking attempt
  - `try_lock_for()` and `try_lock_until()` - timed waits
  
- **Semaphore (CountingSemaphore)**: Fixed-capacity counting semaphore
  - `acquire()` - blocks until a token is available
  - `try_acquire()` - non-blocking attempt
  - `try_acquire_for()` and `try_acquire_until()` - timed waits
  - `release()` - returns a token
  - Template-based capacity configuration
  
- **ConditionVariable**: Synchronization primitive for wait/notify patterns
  - Template support for custom wait queue implementations
  - `wait()` - blocks until notified
  - `wait_for()` and `wait_until()` - timed waits
  - `notify_one()` and `notify_all()` - wakeup operations
  
- **Helpers**: Intrusive doubly-linked list in [helpers/List.h](lib/freeRTOS/helpers/List.h)

## Project Structure

- [platformio.ini](platformio.ini): PlatformIO environments and script hooks
- [src/](src): firmware sources
- [lib/](lib): reusable libraries
  - [core/](lib/core): logging module
  - [freeRTOS/](lib/freeRTOS): synchronization primitives
- [test/](test): test suite for libraries
  - [core/](test/core): logging tests
  - [freeRTOS/](test/freeRTOS): synchronization primitive tests
- [scripts/](scripts): PlatformIO support Python scripts
- [qemu/](qemu): local QEMU installation/cache
- [include/](include): (reserved for public headers)

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

## Test Suite

Comprehensive tests for core libraries located in [test/](test/):

### Core Tests
- **Logging** ([test/core/test_logging/](test/core/test_logging/)): Tests for the logging module including buffering, formatting, and handler behavior

### FreeRTOS Tests
- **Mutex** ([test/freeRTOS/test_mutex/](test/freeRTOS/test_mutex/)): Tests for mutex locking and unlocking
- **Semaphore** ([test/freeRTOS/test_semaphore/](test/freeRTOS/test_semaphore/)): Tests for counting semaphore acquire/release
- **ConditionVariable** ([test/freeRTOS/test_condition_variable/](test/freeRTOS/test_condition_variable/)): Tests for condition variable wait/notify patterns
- **List Helpers** ([test/freeRTOS/helpers/test_list/](test/freeRTOS/helpers/test_list/)): Tests for intrusive list implementation

Tests use the Unity testing framework and can be run via PlatformIO test environments.

## Prerequisites

- Python 3.8+
- PlatformIO Core (`pio` command) or PlatformIO IDE extension
- Toolchain compatible with the community `platform-espressif32` package used in [platformio.ini](platformio.ini)
- FreeRTOS kernel (included via PlatformIO)

## Common Commands

Run these commands from the project root.

### Build

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

### Testing

Run tests for the entire test suite:

```sh
pio test
```

Run tests for a specific environment:

```sh
pio test -e debug
pio test -e qemu-debug
```

Run tests for a specific test project:

```sh
pio test -f test_logging
pio test -f test_mutex
pio test -f test_semaphore
pio test -f test_condition_variable
pio test -f test_list
```

## Notes

- The QEMU script currently supports Linux, macOS, and Windows host naming conventions.
- The logging module uses double-buffering and condition variables for safe concurrent logging from multiple FreeRTOS tasks.
- All synchronization primitives (Mutex, Semaphore, ConditionVariable) are built on top of FreeRTOS kernel objects for maximum reliability and performance.
- The test suite uses the Unity testing framework and is designed to run on both hardware (ESP32-S3) and QEMU emulator.