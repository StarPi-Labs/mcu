#pragma once

#include <Arduino.h>
#include <math.h>

class AirBrake {
private:
	float alt_z;
	float vel_z;
	float out_theta_rad;
	float attitude_deg;
	float predicted_apogee;

	const float TARGET_ALT = 3000;

	const float ALT_ZERO = 145.0f;
	const float RHO_ZERO = 1.225f;
	const float TEMP_ZERO = 273.15f + 20.0f; // TODO: use real temp?, is it sea level?
	const float ROCKET_DIAMETER = 0.149f;
	const float FRONTAL_AREA = ROCKET_DIAMETER*ROCKET_DIAMETER*M_PI*0.25f;
	const float G0 = 9.80665f;
	const float ROCKET_MASS = 1.0f; // FIXME: change mass
	//const float RAD_TO_DEG = 180.0f/M_PI;
	//const float DEG_TO_RAD = M_PI/180.0f;
	const float R = 1.0f; // FIXME: change arm radius
	const float L = 1.0f; // FIXME: change arm radius

	const float kp = 0;
	const float ki = 0.05f;
	const float kd = 0;

	float compute_rho(float alt);
	float theta_to_extension();
	void compute_predicted_apogee_m();
	void input_values(float alt, float vel, float attitude);
	float predict_apogee(float alt, float vel, float attitude);
	float transfer_function(float theta_PID_rad);

	float PID(float error);

public:
};


float AirBrake::compute_rho(float alt)
{
	return RHO_ZERO * powf(1 - (0.0065 * (alt - ALT_ZERO))/TEMP_ZERO, 4.257);
}


float AirBrake::theta_to_extension()
{
	float theta_deg = out_theta_rad*RAD_TO_DEG;
	// TODO: use lookup table
	// Modello cinematico di apertura
	// float extension = R*(1-cosf(out_theta_rad)) + L*(1-sqrtf(1-(R*R)/(L*L))*sinf(out_theta_rad)*sinf(out_theta_rad));
	// Modello analitico da LUT
	float sk = sinf(M_PI*0.5f * powf(theta_deg/108.0f, 1.036));
	float extension = sk*sk*sk*sk;
	return extension;
}


void AirBrake::input_values(float alt, float vel, float attitude)
{
	alt_z = alt;
	vel_z = vel;
	attitude_deg = attitude;
}


void AirBrake::compute_predicted_apogee_m()
{
	float v_axis = vel_z / cosf(attitude_deg*DEG_TO_RAD);
	float x_geom = theta_to_extension();

	float cd_min = 0.21371 * x_geom * x_geom - 2.4482 * 0.001 * x_geom + 0.38143;
	float k = (0.5 * compute_rho(alt_z) * cd_min * FRONTAL_AREA)/ROCKET_MASS;

	if (vel_z > 0 && k > 1e-12) {
		predicted_apogee = alt_z + 0.5/k * log(1+((k*vel_z)*(k*vel_z))/G0);
	} else {
		// FIXME: WHY????
		predicted_apogee = 3000.0f;
	}
}


float AirBrake::predict_apogee(float alt, float vel, float attitude)
{
	input_values(alt, vel, attitude);
	compute_predicted_apogee_m();
	return predicted_apogee;
}



float AirBrake::PID(float error){
	static long last_time = millis();
	float dt = (millis() - last_time) / 1000;

	static float error_int = millis();

	// se passato abbastanza tempo
	error_int += (dt < 1/10) ? error * dt : 0;

	float theta_out = kp * error + ki * error_int;// + kd * error_der;

	return theta_out;
}


float AirBrake::transfer_function(float theta_PID_rad){
	static long last_time = millis();
	float dt = (millis() - last_time) / 1000;

	float num_s[2] = {64.93, -0.811625};
	float den_s[4] = {64.93, 22.561625, 1.271875, 0.0125};

	static float y_last = theta_PID_rad;
	float 
	
	y_last = theta_PID_rad;


	float y_dddot = theta_PID_rad + 

	static float y_dot
}



