#include <Arduino.h>
#include <RadioLib.h>
#include "float16.h"


#define RADIO_CS_PIN A4 // NSS
#define RADIO_IRQ_PIN A0 // DIO1
#define RADIO_RST_PIN A1 // RST
#define SCK_PIN A5
#define SPI_MISO_PIN A6
#define SPI_MOSI_PIN A7
#define RADIO_BUSY_PIN A2

#define LOCKUP(x) do{Serial.println(x);delay(1000);}while(true)


Module radio_module(RADIO_CS_PIN, RADIO_IRQ_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN); //nello stack anziché heap
SX1262 radio(&radio_module);

volatile bool operation_done = false;
int tx_state = RADIOLIB_ERR_NONE;

void operation_done_cb(void);
void start_transmission(uint8_t* data, size_t size);
bool is_transmission_done();
void check_state(int state);
void print_transmission_data();


//payload da trasmettere
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


void setup() {
	Serial.begin(115200);
	while(!Serial);
	Serial.print(F("[SX1262] Initializing ... "));
	
	int state = radio.begin();
	check_state(state);

	radio.setDio2AsRfSwitch(true); //
	//radio.setOutputPower(20); //da capire 
	radio.setBandwidth(500); //max 
	radio.setSpreadingFactor(7); //eventualmente da aumentare
	radio.setCodingRate(1); //1 bit su 8
	radio.forceLDRO(false);
	radio.implicitHeader(sizeof(Payload)); //PACCHETTI A LUNGHEZZA FISSA
	radio.setCRC(true);

	radio.setPacketSentAction(operation_done_cb);	
	
	//long timeOnAir_us = radio.getTimeOnAir(sizeof(Payload));
	//float freqHz = 1000000.0 / timeOnAir_us;

	//primo pacchetto
	start_transmission(Payload{0}.bytes, sizeof(Payload));
}


void loop() {
	Payload dataPacket{0}; //pacchetto da riempire
  
  	if (is_transmission_done()) {
		operation_done = false;

		print_transmission_data();
		
		start_transmission(dataPacket.bytes, sizeof(dataPacket));
	}
}


void start_transmission(uint8_t* data, size_t size) {
	tx_state = radio.startTransmit(data, size);
}

void operation_done_cb(void) {
	operation_done = true;
}

bool is_transmission_done() {
	if (operation_done) {
		if (tx_state == RADIOLIB_ERR_NONE) {
			//Serial.println(F("transmission finished!"));
		} else {
			Serial.print(F("failed, code "));
			Serial.println(tx_state);
		}

		radio.finishTransmit();
	}

	return operation_done;
}

void check_state(int state) {
		if (state == RADIOLIB_ERR_NONE) {
		Serial.println(F("success!"));
	} else {
		Serial.print(F("failed, code "));
		Serial.println(state);
		LOCKUP(state);
	}

}

void print_transmission_data(){
	static uint32_t packet_count = 0;
  	
	Serial.print("Sent packet: ");
	Serial.println(++packet_count);
}