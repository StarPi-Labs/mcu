#include <Arduino.h>
#include <TinyGPS++.h>
#include <time.h>

#include "board.h"
#include "gps.h"


TinyGPSPlus parser;


void gps_setup(void)
{
	// Some messages take very long to transmit (>1s checked with
	// an oscilloscope), so we set a large buffer to avoid loosing data and
	// on update set a timeout as to not slow down other tasks
	Serial2.setTxBufferSize(GPS_SERIAL_BUFFER_SIZE);
	Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
	// TODO: This module looses all configuration at startup, this would be the
	// place to reload them
}


// Update the GPSData structure, if new data is available then the correct members
// will be updated, otherwise they will be left as-is
void gps_update(GPSData *data)
{
	// Read available data for a certain amount of time, basically try to
	// exhaust the serial buffer but dont't wait for long transmissions to
	// complete since this function is called periodically
	uint32_t start = millis();
	bool got_data = false;
	while (Serial2.available() > 0 && millis() - start < GPS_SERIAL_READ_TIMEOUT_MS) {
		parser.encode(Serial2.read());
		got_data = true;
		continue;
	}

	if (got_data == false || data == NULL) return;

	// Update time
	if (parser.time.isUpdated() || parser.date.isUpdated()) {
		if (parser.time.isValid() && parser.date.isValid()) {
			struct tm t = {
				.tm_sec = parser.time.second(),
				.tm_min = parser.time.minute(),
				.tm_hour = parser.time.hour(),
				.tm_mday = parser.date.day(),
				.tm_mon = parser.date.month() - 1, // tm_mon is 0-11
				.tm_year = parser.date.year() - 1900, // tm_year is years since 1900
			};
			data->unix_time = mktime(&t);
		}
	}

	// Update number of satellites
	if (parser.satellites.isUpdated() && parser.satellites.isValid()) {
		data->num_sat = parser.satellites.value();
	}

	// If the lock is strong update all position values
	if (data->num_sat >= GPS_MIN_SATELLITES) {
		// Update latitude and longitude
		if (parser.location.isUpdated() && parser.location.isValid()) {
			data->lat = parser.location.lat();
			data->lon = parser.location.lng();
		}
		// Update altitude
		if (parser.altitude.isUpdated() && parser.altitude.isValid()) {
			data->alt = parser.altitude.meters();
		}
		// Update ground speed
		if (parser.speed.isUpdated() && parser.speed.isValid()) {
			data->kmh = parser.speed.kmph();
		}
	}
}