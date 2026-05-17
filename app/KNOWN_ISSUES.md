# Known Issues

## QEMU: Assert Messages Not Displayed

**Issue**: `assert()` statements don't display their condition message in QEMU.

**Symptom**: When an assertion fails in QEMU, the condition string is not printed to the console, only the halt/reboot occurs.

**Root Cause**: Faulty SPI emulation in QEMU prevents reading string literals from memory.

**Status**: Works correctly on physical ESP32-S3 board.

**Affected**: Test assertions and any code using `assert(false && "message")` patterns when running in QEMU environments.

**Workaround**: The assertion still triggers and halts/reboots correctly; only the condition message text is lost. This is a QEMU limitation, not an issue with the application code.

**Note**: `env:qemu-production` inherits `PRODUCTION` mode, so standard `assert()` is removed entirely there due to `NDEBUG`.


### Booting with the SD Card inserted prevents the board from functioning

> NOTE: This is only for rev 1 of the board and for N8 (wihtout PSRAM) modules,
>       DO NOT change these fuses on other hardware revisions and/or esp modules.

This is an hardware issue, fixed by burning the following eFUSEs:
VDD_SPI_FORCE, VDD_SPI_XPD, VDD_SPI_TIEH to 1. This can be done with espefuse.py
[link](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/espefuse/burn-efuse-cmd.html)
by specifying each fuse by hand, or by [setting spi_vdd](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/espefuse/set-flash-voltage-cmd.html)
to 3.3V which does the same.
