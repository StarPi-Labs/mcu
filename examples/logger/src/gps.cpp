#include <Arduino.h>
#include <TinyGPS++.h>

#include "board.h"
#include "gps.h"


TinyGPSPlus parser;


// Source: https://www.andrews.edu/~tzs/timeconv/timealgorithm.html
/* This below is the "correct" implementation, however at this time
 * 2026/05/30 any new GPS time sees a constant number of leap seconds, as such
 * the implementation can be simplified to a simple offset
// List of leap seconds
static const uint32_t leap_seconds[] = {
	46828800, 78364801, 109900802, 173059203, 252028804, 315187205, 346723206,
	393984007, 425520008, 457056009, 504489610, 551750411, 599184012, 820108813,
	914803214, 1025136015, 1119744016, 1167264017
};


// Count number of leap seconds that have passed
static uint32_t countleaps(uint32_t gpsTime)
{
	uint32_t nleaps = 0; // number of leap seconds prior to gpsTime
	for (int i = 0; i < sizeof(leap_seconds)/sizeof(leap_seconds[0]); i++) {
		if (gpsTime >= leap_seconds[i]) {
			nleaps++;
		}
	}
	return nleaps;
}

// Convert GPS Time to Unix Time
static uint32_t gps2unix(uint32_t gpsTime)
{
	// Add offset in seconds
	uint32_t unixTime = gpsTime + 315964800;
	unixTime -= countleaps(gpsTime);
	// FIXME: omitted half-second round-up if current gps time is a leap second
	return unixTime;
}
*/

// Convert GPS Time to Unix Time
static uint32_t gps2unix(uint32_t gpsTime)
{
	// TODO
	return gpsTime;
}


void gps_setup(void)
{
	Serial2.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
	// TODO: This module looses all configuration at startup, this would be the
	// place to reload them
}


// Update the GPSData structure, if new data is available then the correct members
// will be updated, otherwise they will be left as-is
void gps_update(GPSData *data)
{
	// First read the serial and then return on error to not loose data
	while (Serial2.available() > 0) {
		parser.encode(Serial2.read());
	}
	
	if (data == NULL) return;

	// Update time
	if (parser.time.isUpdated() && parser.time.isValid()) {
		data->unix_time = gps2unix(parser.time.value());
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