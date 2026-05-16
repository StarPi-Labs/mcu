# Known Issues

## QEMU: Assert Messages Not Displayed

**Issue**: `assert()` statements don't display their condition message in QEMU.

**Symptom**: When an assertion fails in QEMU, the condition string is not printed to the console, only the halt/reboot occurs.

**Root Cause**: Faulty SPI emulation in QEMU prevents reading string literals from memory.

**Status**: Works correctly on physical ESP32-S3 board.

**Affected**: Test assertions and any code using `assert(false && "message")` patterns when running in QEMU environments.

**Workaround**: The assertion still triggers and halts/reboots correctly; only the condition message text is lost. This is a QEMU limitation, not an issue with the application code.

**Note**: `env:qemu-production` inherits `PRODUCTION` mode, so standard `assert()` is removed entirely there due to `NDEBUG`.
