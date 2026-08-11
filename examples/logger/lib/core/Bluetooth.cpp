#include "Bluetooth.hpp"

#include <NimBLEDevice.h>

constexpr size_t LOG_MESSAGE_BUFFER_SIZE = 256;

static const NimBLEUUID
    LOG_SERVICE_UUID("53cfc3a2-72dc-4bf0-805c-acc1f8ba9706");
static const NimBLEUUID
    LOG_CHARACTERISTIC_UUID("53cfc3a2-72dc-4bf1-805c-acc1f8ba9706");

static constexpr const char *DEVICE_NAME = "John StarPi's Rocket";

static NimBLECharacteristic *g_pLogCharacteristic = nullptr;

static void init_services(NimBLEServer *pServer) {
  assert(pServer && "NimBLE server pointer is null");
  assert(!g_pLogCharacteristic && "Log characteristic already initialized");

  // NOTE: docs don't specify to check for nullptr, asserting below for safety.

  NimBLEService *pLogService = pServer->createService(LOG_SERVICE_UUID);
  assert(pLogService && "Failed to create log service");

  g_pLogCharacteristic = pLogService->createCharacteristic(
      LOG_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  assert(g_pLogCharacteristic && "Failed to create log characteristic");

  // Set format to opaque struct
  NimBLE2904 *pFormatDescriptor = g_pLogCharacteristic->create2904();
  assert(pFormatDescriptor && "Failed to create format descriptor");
  pFormatDescriptor->setFormat(NimBLE2904::FORMAT_OPAQUE);

  // Set informational user descriptor
  const NimBLEUUID USER_DESC_UUID((uint16_t)0x2901);
  NimBLEDescriptor *pUserDescriptor = g_pLogCharacteristic->createDescriptor(
      USER_DESC_UUID, NIMBLE_PROPERTY::READ);
  assert(pUserDescriptor && "Failed to create user descriptor");
  pUserDescriptor->setValue(
      "See include/logger.h:logger_message_to_bytes(uint8_t*, size_t, "
      "LogMessage) for message format");
}

BLEStatus ble_init() {
  assert(!NimBLEDevice::isInitialized() && "NimBLE already initialized");

  if (!NimBLEDevice::init(DEVICE_NAME))
    return BLEStatus::DEVICE_INIT_FAIL;

  // NOTE: docs don't specify to check for nullptr, asserting below for safety.

  NimBLEServer *pServer = NimBLEDevice::createServer();
  assert(pServer && "Failed to create NimBLE server");

  init_services(pServer);

  if (!pServer->start())
    return BLEStatus::SERVER_START_FAIL;

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  assert(pAdvertising && "Failed to get NimBLE advertising object");

  pAdvertising->enableScanResponse(true);
  if (!pAdvertising->setName(DEVICE_NAME))
    return BLEStatus::ADVERT_SET_NAME_FAIL;

  if (!pAdvertising->start())
    return BLEStatus::ADVERT_START_FAIL;

  return BLEStatus::SUCCESS;
}

BLEStatus ble_send_log_message(LogMessage *message) {
  assert(NimBLEDevice::isInitialized() && "NimBLE not initialized");
  assert(message && "Log message pointer is null");
  assert(g_pLogCharacteristic && "Log characteristic not initialized");

  static uint8_t message_buffer[LOG_MESSAGE_BUFFER_SIZE];

  size_t bytes_written =
      logger_message_to_bytes(message_buffer, LOG_MESSAGE_BUFFER_SIZE, message);

  g_pLogCharacteristic->setValue(message_buffer, bytes_written);
  if (!g_pLogCharacteristic->notify())
    return BLEStatus::NOTIFY_FAIL;

  return BLEStatus::SUCCESS;
}

BLEStatus ble_destroy() {
  assert(NimBLEDevice::isInitialized() && "NimBLE not initialized");

  return NimBLEDevice::deinit(true) ? BLEStatus::SUCCESS
                                    : BLEStatus::DESTROY_FAIL;
}
