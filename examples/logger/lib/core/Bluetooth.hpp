#pragma once

#include "logger.h"
#include <cstdint>

/// @brief Represents the status of BLE operations.
enum class BLEStatus : uint8_t {
  SUCCESS,
  DEVICE_INIT_FAIL,
  SERVER_START_FAIL,
  ADVERT_SET_NAME_FAIL,
  ADVERT_START_FAIL,
  NOTIFY_FAIL,
  DESTROY_FAIL,
};

/// @brief Initializes the BLE stack and starts advertising the device.
/// @param ble_context Pointer to a BLEContext_t structure to hold BLE context.
/// @pre The BLE stack must not be initialized before calling this function.
/// @return BLEStatus indicating the result of the initialization.
BLEStatus ble_init();

/// @brief Sends a log message over BLE.
/// @param message The log message to send.
/// @param ble_context Pointer to a BLEContext_t structure to hold BLE context.
/// @pre The BLE stack must be initialized before calling this function.
/// @pre The log message must be non-null.
/// @return BLEStatus indicating the result of the operation.
BLEStatus ble_send_log_message(LogMessage *message);

/// @brief Destroys the BLE stack and frees associated resources.
/// @param ble_context Pointer to a BLEContext_t structure to hold BLE context.
/// @pre The BLE stack must be initialized before calling this function.
/// @return BLEStatus indicating the result of the destruction.
BLEStatus ble_destroy();