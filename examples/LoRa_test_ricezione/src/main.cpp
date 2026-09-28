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

typedef union {
	struct { //dimensioni da capire (così si trasmette 40 pacchetti al secondo)
		float16 AccX;
		float16 AccY;
		float16 AccZ;
		float16 GyroX;
		float16 GyroY;
		float16 GyroZ;
		float16 EulerPitch;
		float16 EulerRoll;
		float16 EulerYaw;
		float16 AccX1;
		float16 AccY1;
		float16 AccZ1;
		float16 GyroX1;
		float16 GyroY1;
		float16 GyroZ1;
		float16 EulerPitch1;
		float16 EulerRoll1;
		float16 EulerYaw1;
	} values;
	uint8_t bytes[sizeof(values)];
} Payload;


void operation_done_cb(void);
void print_transmission_data(Payload* packet = nullptr);
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


  Payload packet{0};
  int state = radio.readData(packet.bytes, sizeof(Payload));


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


void print_transmission_data(Payload* packet){
  Serial.println(F("[SX1262] Received packet!"));
  if (packet == nullptr) return;


  // print RSSI (Received Signal Strength Indicator)
  Serial.print(F("[SX1262] RSSI:\t\t"));
  Serial.print(radio.getRSSI());
  Serial.println(F(" dBm"));

  // print SNR (Signal-to-Noise Ratio)
  Serial.print(F("[SX1262] SNR:\t\t"));
  Serial.print(radio.getSNR());
  Serial.println(F(" dB"));

  // print frequency error
  Serial.print(F("[SX1262] Frequency error:\t"));
  Serial.print(radio.getFrequencyError());
  Serial.println(F(" Hz"));


  Serial.print(F("[SX1262] AccX:\t\t"));
  Serial.println(packet->values.AccX.toFloat());
  Serial.print(F("[SX1262] AccY:\t\t"));
  Serial.println(packet->values.AccY.toFloat());
  Serial.print(F("[SX1262] AccZ:\t\t"));
  Serial.println(packet->values.AccZ.toFloat());
  Serial.print(F("[SX1262] GyroX:\t\t"));
  Serial.println(packet->values.GyroX.toFloat());
  Serial.print(F("[SX1262] GyroY:\t\t"));
  Serial.println(packet->values.GyroY.toFloat());
  Serial.print(F("[SX1262] GyroZ:\t\t"));
  Serial.println(packet->values.GyroZ.toFloat());
  Serial.print(F("[SX1262] EulerPitch:\t"));
  Serial.println(packet->values.EulerPitch.toFloat());
  Serial.print(F("[SX1262] EulerRoll:\t"));
  Serial.println(packet->values.EulerRoll.toFloat());
  Serial.print(F("[SX1262] EulerYaw:\t"));
  Serial.println(packet->values.EulerYaw.toFloat());

  //ecc.
  
  
}

