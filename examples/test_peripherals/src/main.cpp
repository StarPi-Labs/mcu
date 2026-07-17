#include <Arduino.h>
#include <FreeRTOS.h>

#include "board.h"

static bool hs_1 = false;
static bool ls_1 = false;
static bool hs_2 = false;
static bool ls_2 = false;
static bool hs_3 = false;
static bool ls_3 = false;
static bool ls_4 = false;
static bool ls_5 = false;
static bool ls_6 = false;


void setup(void)
{

	Serial.begin(115200);
	while (!Serial) {
		delay(100);
	}
	Serial.println("Initialized");

	pinMode(PINT1_HS, OUTPUT);
	digitalWrite(PINT1_HS, LOW);
	pinMode(PINT1_LS, OUTPUT);
	digitalWrite(PINT1_LS, LOW);
	pinMode(PINT2_HS, OUTPUT);
	digitalWrite(PINT2_HS, LOW);
	pinMode(PINT2_LS, OUTPUT);
	digitalWrite(PINT2_LS, LOW);
	pinMode(PINT3_HS, OUTPUT);
	digitalWrite(PINT3_HS, LOW);
	pinMode(PINT3_LS, OUTPUT);
	digitalWrite(PINT3_LS, LOW);
	pinMode(PINT4_LS, OUTPUT);
	digitalWrite(PINT4_LS, LOW);
	pinMode(PINT5_LS, OUTPUT);
	digitalWrite(PINT5_LS, LOW);
	pinMode(PINT6_LS, OUTPUT);
	digitalWrite(PINT6_LS, LOW);
}

void loop(void)
{
	// Only do anything if a character has actually arrived
	if (Serial.available() > 0) {
		char incomingByte = Serial.read();

		switch (incomingByte) {
			case '1':
				ls_1 = !ls_1;
				Serial.printf("PINT1_LS: %d\n", ls_1); // Added \n for cleaner formatting
				digitalWrite(PINT1_LS, ls_1);
				break;
			case '2':
				ls_2 = !ls_2;
				Serial.printf("PINT2_LS: %d\n", ls_2);
				//digitalWrite(PINT2_LS, ls_2);
				if (ls_2) {
					analogWrite(PINT2_LS, 255/2);
				} else {
					analogWrite(PINT2_LS, 0);
				}
				break;
			case '3':
				ls_3 = !ls_3;
				Serial.printf("PINT3_LS: %d\n", ls_3);
				digitalWrite(PINT3_LS, ls_3);
				break;
			case '4':
				ls_4 = !ls_4;
				Serial.printf("PINT4_LS: %d\n", ls_4);
				digitalWrite(PINT4_LS, ls_4);
				break;
			case '5':
				ls_5 = !ls_5;
				Serial.printf("PINT5_LS: %d\n", ls_5);
				digitalWrite(PINT5_LS, ls_5);
				break;
			case '6':
				ls_6 = !ls_6;
				Serial.printf("PINT6_LS: %d\n", ls_6);
				digitalWrite(PINT6_LS, ls_6);
				break;
			case 'a':
				hs_1 = !hs_1;
				Serial.printf("PINT1_HS: %d\n", hs_1);
				digitalWrite(PINT1_HS, hs_1);
				break;
			case 'b':
				hs_2 = !hs_2;
				Serial.printf("PINT2_HS: %d\n", hs_2);
				digitalWrite(PINT2_HS, hs_2);
				break;
			case 'c':
				hs_3 = !hs_3;
				Serial.printf("PINT3_HS: %d\n", hs_3);
				digitalWrite(PINT3_HS, hs_3);
				break;
			
			// Ignore newline and carriage return characters sent by the serial monitor
			case '\n':
			case '\r':
				break;

			default: 
				// Optional: Alert you if you typed the wrong key
				Serial.printf("Unknown command: %c\n", incomingByte);
				break;
		}
	}
}

void loop1(void)
{
	delay(10000);
	Serial.println("Reset");
	digitalWrite(PINT1_HS, LOW);
	digitalWrite(PINT1_LS, LOW);
	digitalWrite(PINT2_HS, LOW);
	digitalWrite(PINT2_LS, LOW);
	digitalWrite(PINT3_HS, LOW);
	digitalWrite(PINT3_LS, LOW);
	digitalWrite(PINT4_LS, LOW);
	digitalWrite(PINT5_LS, LOW);
	digitalWrite(PINT6_LS, LOW);
}

