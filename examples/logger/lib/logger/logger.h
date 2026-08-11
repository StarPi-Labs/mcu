#pragma once

#include <FreeRTOS.h>
#include <time.h>

#define LOG_TIMEOUT 1
#define LOG_MAX_CONSUMERS 8

constexpr size_t LOG_MESSAGE_MINIMUM_SIZE = 109;

enum MessagePayloadType : uint16_t {
  P_NONE = 1 << 0,
  P_BOOL = 1 << 1,
  P_FLOAT = 1 << 2,
  P_DOUBLE = 1 << 3,
  P_INT = 1 << 4,
  P_LONG = 1 << 5,
  P_FVEC2 = 1 << 6,
  P_FVEC3 = 1 << 7,
  P_STRING = 1 << 8,
};

enum SourceSubsystem : uint8_t {
  S_OTHER = 1 << 0,
  S_IMU = 1 << 1,
  S_BARO = 1 << 2,
  S_GPS = 1 << 3,
  S_LORA = 1 << 4,
  S_SD = 1 << 5,
};

enum MessageType : uint8_t {
  T_ACCELLERATION = 1 << 0,
  T_GYRO = 1 << 1,
  T_ALT_SPEED = 1 << 2,
  T_PRESSURE = 1 << 3,
  T_TEMPERATURE = 1 << 4,
  T_GPS = 1 << 5,
  T_SYSLOG = 1 << 6,
  T_ORIENTATION = 1 << 7,
};

typedef struct {
  uint64_t timestamp; //< UNIX timestamp in microseconds
  MessagePayloadType payload_type;
  SourceSubsystem src;
  MessageType type;
  union {
    bool b;
    float f;
    double d;
    int i;
    long l;
    struct {
      float x, y;
    } fv2;
    struct {
      float x, y, z;
    } fv3;
    const char *s;
  } payload;
} LogMessage;

typedef struct {
  uint32_t payload_filter;
  uint32_t type_filter;
  TaskHandle_t task_handle;
  QueueHandle_t msg_queue;
} LogConsumer;

bool logger_register_consumer(TaskHandle_t task_handle, QueueHandle_t msg_queue,
                              uint32_t payload_filter, uint32_t type_filter);
bool logger_sort_message(LogMessage *msg);
size_t logger_message_to_str(const char **str, LogMessage *msg);
/// @brief Serializes a LogMessage into a byte array.
///
/// The order follows the LogMessage struct:
///  - timestamp: 64B
///  - payload_type: 16B
///  - src: 8B
///  - type: 8B
///  - payload: variable
///
/// The payload is serialized based on the payload_type:
///  - P_NONE: 0B
///  - P_BOOL: 1B
///  - P_FLOAT: 4B
///  - P_DOUBLE: 8B
///  - P_INT: 4B
///  - P_LONG: 8B
///  - P_FVEC2: 8B
///  - P_FVEC3: 12B
///  - P_STRING: variable, up to payload_string_max_length + 1 ('\0')
///
/// @param dest The destination byte array to write the serialized message to.
/// @param payload_string_max_length The maximum length of the payload string,
/// if the message contains a string payload (without null terminator).
/// @param msg The LogMessage to serialize.
/// @note dest must be large enough to hold at maximum
/// 96B + max{12B, payload_string_max_length + 1B}
/// @note byte order is little-endian for multi-byte fields (timestamp,
/// payload_type, payload).
/// @return The number of bytes written to the destination array.
size_t logger_message_to_bytes(uint8_t *dest, size_t payload_string_max_length,
                               LogMessage *msg);
void log(SourceSubsystem src, MessageType type, bool b);
void log(SourceSubsystem src, MessageType type, float f);
void log(SourceSubsystem src, MessageType type, double d);
void log(SourceSubsystem src, MessageType type, int i);
void log(SourceSubsystem src, MessageType type, long l);
void log(SourceSubsystem src, MessageType type, float x, float y);
void log(SourceSubsystem src, MessageType type, float x, float y, float z);
void log(SourceSubsystem src, MessageType type, const char *s);

bool logger_init(void);

uint64_t now_us(void);
uint64_t now_ms(void);

#define TO_XSTR(s) TO_STR(s)
#define TO_STR(s) #s
