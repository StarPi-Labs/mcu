#include <Arduino.h>
#include <RadioLib.h>
#include "float16.h"


// Same pin assignments as flight computer
#define RADIO_CS_PIN 14 // NSS
#define RADIO_IRQ_PIN 7 // DIO1
#define RADIO_RST_PIN 6 // RST
#define SCK_PIN 12
#define SPI_MISO_PIN 13
#define SPI_MOSI_PIN 11
#define RADIO_BUSY_PIN 5

#define LOCKUP(x) do{Serial.println(x);delay(1000);}while(true)


Module radio_module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN); 
SX1262 radio(&radio_module);


volatile bool operation_done = false;
int rx_state = RADIOLIB_ERR_NONE;

enum LoRaPayloadType : uint8_t {
	PAYLOAD_1,
	PAYLOAD_2,
	PAYLOAD_3,
	PAYLOAD_STR
};

//payload da trasmettere
// TODO: utlizza meglio lo spazio
typedef union {
	struct {
		uint64_t timestamp;
		LoRaPayloadType type;
		union {
			struct {
				float16 roll, pitch, yaw;
				float16 vert_acc;
				float16 airbrake_angle;
			} p1;
			struct {
				float16 temp1, temp2, temp3;
				uint8_t para_state;
			} p2;
			struct {
				float16 pos_dx, pos_dy, pos_dz;
			} p3;
			char str[10];
		};
		uint8_t err_flag;
	} data;
	uint8_t bytes[sizeof(data)];
} LoRaPayload;

void operation_done_cb(void);
void print_transmission_data(const LoRaPayload* packet = nullptr);
void check_state(int state);



void setup() {
  Serial.begin(115200);
  while (!Serial);
  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }


  radio.setPacketReceivedAction(operation_done_cb);
  
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  check_state(state);
}

void loop() {
  // check if the flag is set
  if(!operation_done) return;

  operation_done = false;


  LoRaPayload packet{0};
  int state = radio.readData(packet.bytes, sizeof(LoRaPayload));


  if (state == RADIOLIB_ERR_NONE) {
    // packet was successfully received
    print_transmission_data(&packet);
  }
  else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);

  }
  
}


void operation_done_cb(void) {
	operation_done = true;
}

void check_state(int state) {
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }
}


void print_transmission_data(const LoRaPayload* packet){
  if (packet == nullptr) return;

  // Header (Timestamp, Type, Signal)
  Serial.print(F("[LORA] "));
  Serial.printf("T: %10llu | ", packet->data.timestamp);
  Serial.println();

  const char* type_str = "UNKNOWN";
  switch(packet->data.type) {
      case PAYLOAD_1:   type_str = "P1"; break;
      case PAYLOAD_2:   type_str = "P2"; break;
      case PAYLOAD_3:   type_str = "P3";    break;
      case PAYLOAD_STR: type_str = "STR";    break;
  }

  Serial.printf("TYPE: %-3s | ERR: 0x%02X | RSSI: %4d | SNR: %5.2f | FREQ_ERR: %5.2f\n", 
                type_str, packet->data.err_flag, radio.getRSSI(), radio.getSNR(), radio.getFrequencyError());

  // Payload
  Serial.print(F("\t> "));

  switch(packet->data.type) {
      case PAYLOAD_1:
          Serial.printf("ROLL: %6.2f | PIT: %6.2f | YAW: %6.2f | V-ACC: %5.2f | AB-ANG: %5.2f", 
                packet->data.p1.roll.toFloat(), 
                packet->data.p1.pitch.toFloat(), 
                packet->data.p1.yaw.toFloat(), 
                packet->data.p1.vert_acc.toFloat(), 
                packet->data.p1.airbrake_angle.toFloat());
          break;
          
      case PAYLOAD_2:
          Serial.printf("T1: %5.2f | T2: %5.2f | T3: %5.2f | PARA_STATE: %u", 
              packet->data.p2.temp1.toFloat(), 
              packet->data.p2.temp2.toFloat(), 
              packet->data.p2.temp3.toFloat(), 
              packet->data.p2.para_state);
          break;
          
      case PAYLOAD_3:
          Serial.printf("X: %8.2f | Y: %8.2f | Z: %8.2f", 
                packet->data.p3.pos_dx.toFloat(), 
                packet->data.p3.pos_dy.toFloat(), 
                packet->data.p3.pos_dz.toFloat());
          break;
          
      case PAYLOAD_STR:
          Serial.printf("\"%s\"", packet->data.str);
          break;
  }
  Serial.println(); 
  Serial.println(F("  -------------------------------------------------------------------------"));


  
}

