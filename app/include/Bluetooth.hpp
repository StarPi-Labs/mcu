#pragma once

#include <NimBLEDevice.h>
#include <cstdint>
#include <logger.h>

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

typedef struct {
  void (*on_sensor_calibration)(void *context);
  void *context;
} BLECallbacks_t;

/// @brief Initializes the BLE stack and starts advertising the device.
/// @param callbacks A BLECallbacks_t structure containing the callback
/// functions for handling BLE events.
/// @pre The BLE stack must not be initialized before calling this function.
/// @pre The callbacks structure must be valid and contain the necessary
/// non-null callback functions.
/// @return BLEStatus indicating the result of the initialization.
BLEStatus ble_init(const BLECallbacks_t &callbacks);

/// @brief Sends a log message over BLE.
/// @param message The log message to send.
/// @pre The log message must be non-null.
/// @pre The BLE stack must be initialized before calling this function.
/// @return BLEStatus indicating the result of the operation.
BLEStatus ble_send_log_message(LogMessage &message);

/// @brief Sets the BLE callbacks for handling events.
/// @param callbacks Pointer to a BLECallbacks_t structure containing the
/// callback functions.
/// @pre callbacks must be valid and contain the necessary non-null callback
/// functions.
void ble_set_callbacks(const BLECallbacks_t &callbacks);

/// @brief Gets the number of active BLE connections.
/// @return The number of active BLE connections or 0 if the BLE stack is not
/// initialized.
uint8_t ble_get_connection_count();

/// @brief Stops the BLE stack and frees associated resources,
/// if not initialized does nothing.
/// @return BLEStatus indicating the result of the destruction.
BLEStatus ble_destroy();