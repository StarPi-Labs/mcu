#pragma once

#include <Arduino.h>
#include <math.h>
#include "barometer.h"


#define P_VEGA_3000 68657

inline float compute_altitude_real(float P){
	float h = -1.30649404e-12 * P * P * P + 8.15083331e-07 * P * P - 0.209247123 * P + 14188.7711;

	return h;
}


// double T = 6.89568815e-13 * P * P * P - 1.58358303e-07 * P * P + 1.25701144e-02 * P - 65.9121021;


class AirBrake {
private:
	float alt_z;
	float vel_z;
	float theta_servo_rad;
	float attitude_rad;
	float predicted_apogee;

	float ground_pressure;
	float ground_temp_k;

	float current_pressure;

	float target_alt_asl = 2960;
	float ALT_ZERO = 160.0f;
	const float RHO_ZERO = 1.225f;

	const float TEMP_ZERO = 273.15f + 20.0f; // TODO: use real temp?, is it sea level?
	const float ROCKET_DIAMETER = 0.149f;
	const float FRONTAL_AREA = ROCKET_DIAMETER*ROCKET_DIAMETER*M_PI*0.25f;
	const float G0 = 9.80665f;
	const float ROCKET_MASS = 32.55f; // FIXME: change mass
	const float RAD_TO_DEG = 180.0f/M_PI;
	const float DEG_TO_RAD = M_PI/180.0f;
	// const float R = 1.0f; // FIXME: change arm radius
	// const float L = 1.0f; // FIXME: change arm radius

	const float kp = 0;
	const float ki = 0.05f;
	const float kd = 0;

	void input_values(float alt, float vel, float attitude, float pressure);
	float compute_rho(float P);
	float theta_to_extension();

	void compute_predicted_apogee_m();
	float predict_apogee(float alt, float vel, float attitude, float pressure);
	float PID(float error);

	void transfer_function(float theta_PID_rad);

	//68657 Pa -> 3000m
	//99370 Pa -> rampa 
	//101250 Pa -> mare

public:

	AirBrake();
	float theta_update(float alt, float vel, float attitude, float pressure);
	// void set_altitude_0(float ground_pressure);
	float get_altitude_asl(float pressure);
	// void set_altitude_target(float p_vega_3000);

	

};


// void AirBrake::set_altitude_0(float ground_pressure){
// 	ALT_ZERO = get_altitude();
// }

// void AirBrake::set_ground_pressure(float _ground_pressure) : ground_pressure(_ground_pressure){ }


//includi parametro temperatura
float AirBrake::get_altitude_asl(float pressure){
	float altitude = compute_altitude_real(pressure);//ground_temp_k/(15.0f + 273.15f) * compute_altitude(pressure, ground_pressure) + 145.0f;

	return altitude;
}

// void AirBrake::set_altitude_target(float p_vega_3000){
// 	target_alt_asl = get_altitude(p_vega_3000);
// }


AirBrake::AirBrake() {
	target_alt_asl = get_altitude_asl(P_VEGA_3000);
}


//voglio altezza in funzione della pressione del modello di te

float AirBrake::compute_rho(float P)
{
	float rho = -2.76286193e-15 * P * P * P + 6.15406912e-10 * P * P - 3.45945879e-05 * P + 1.24103798;

	return rho;//RHO_ZERO * powf(1 - (0.0065 * (alt + ALT_ZERO))/TEMP_ZERO, 4.257); //todo: sottrazione alt0??
}


float AirBrake::theta_to_extension()
{
	float theta_deg = theta_servo_rad*RAD_TO_DEG;
	// TODO: use lookup table
	// Modello cinematico di apertura
	// float extension = R*(1-cosf(theta_servo_rad)) + L*(1-sqrtf(1-(R*R)/(L*L))*sinf(theta_servo_rad)*sinf(theta_servo_rad));
	// Modello analitico da LUT
	float sk = sinf(M_PI*0.5f * powf(theta_deg/108.0f, 1.036));
	float extension = sk*sk*sk*sk;
	return extension;
}


void AirBrake::input_values(float alt, float vel, float attitude, float pressure) {
	alt_z = alt; 
	vel_z = vel; 
	attitude_rad = attitude; 
	current_pressure = pressure;
}


void AirBrake::compute_predicted_apogee_m()
{
	float v_axis = vel_z / cosf(attitude_rad);
	float x_geom = theta_to_extension();

	float cd_min = 0.21371 * x_geom * x_geom - 2.4482 * 0.001 * x_geom + 0.38143;
	float k = (0.5 * compute_rho(current_pressure) * cd_min * FRONTAL_AREA)/ROCKET_MASS;

	predicted_apogee = alt_z + 0.5/k * log(1+(k*(vel_z)*(vel_z))/G0);
	
}


float AirBrake::predict_apogee(float alt, float vel, float attitude_rad, float pressure)
{
	input_values(alt, vel, attitude_rad, pressure);
	compute_predicted_apogee_m();
	return predicted_apogee;
}


float AirBrake::PID(float error){
	static long last_time = millis();
	float dt = (millis() - last_time) / 1000.0f;

	static float error_int = 0;

	// se passato abbastanza tempo
	error_int += (dt < 1/10.0f) ? error * dt : 0;

	float theta_pid = kp * error + ki * error_int;// + kd * error_der;

	last_time = millis();

	float theta_pid_clamped = min(108, max(theta_pid, 0));
	return theta_pid_clamped;
}

/*
float AirBrake::transfer_function(float theta_PID_rad){
	static long last_time = millis();
	float dt = (millis() - last_time) / 1000;

	float num_s[2] = {64.93, -0.811625};
	float den_s[4] = {64.93, 22.561625, 1.271875, 0.0125};

	static float y_last = theta_PID_rad;
	
	
	y_last = theta_PID_rad;


	float y_dddot = theta_PID_rad + 

	static float y_dot;
}
*/

//scritta da gemini -> controlla
void AirBrake::transfer_function(float theta_PID_rad) {
    // 1. Calcolo del dt preciso in secondi
    static unsigned long last_time = micros();
    unsigned long now = micros();
    float dt = (now - last_time) / 1000000.0f;
    last_time = now;

    // Protezione per il primo ciclo o se il microcontrollore si blocca per troppo tempo
    if (dt <= 0.0f || dt > 0.01f) {
        dt = 0.001f; 
    }

    // 2. Definizione dei coefficienti originali tramite array
    // Polinomio: array[0] + array[1]*s + array[2]*s^2 + array[3]*s^3
    float num_s[2] = {64.93f, -0.811625f};
    float den_s[4] = {64.93f, 22.561625f, 1.271875f, 0.0125f};

    // 3. Normalizzazione nel codice (dividiamo tutto per il grado massimo den_s[3])
    float norm_factor = den_s[3];

    for (int i = 0; i < 2; i++) {
        num_s[i] /= norm_factor;
    }
    for (int i = 0; i < 4; i++) {
        den_s[i] /= norm_factor;
    }

    // 4. Array per gli stati del sistema (memoria tra un ciclo e l'altro)
    // x[0] = stato base, x[1] = derivata prima, x[2] = derivata seconda
    static float x[3] = {0.0f, 0.0f, 0.0f};
    float x_dot[3];

    // 5. Calcolo delle derivate degli stati (Spazio degli Stati)
    x_dot[0] = x[1];
    x_dot[1] = x[2];
    x_dot[2] = theta_PID_rad - (den_s[0] * x[0]) - (den_s[1] * x[1]) - (den_s[2] * x[2]);

    // 6. Integrazione numerica (stato_attuale = stato_precedente + derivata * dt)
    for (int i = 0; i < 3; i++) {
        x[i] += x_dot[i] * dt;
    }

    // 7. Calcolo dell'output combinando gli stati con il numeratore normalizzato
    float theta_EFF_rad = (num_s[0] * x[0]) + (num_s[1] * x[1]);


	theta_servo_rad = theta_EFF_rad;
}


//aggiusta target

float AirBrake::theta_update(float alt, float vel, float attitude, float pressure){
	input_values(alt, vel, attitude, pressure);
	float theta_pid = petali.PID(predicted_apogee - target_alt_asl); 

	transfer_function(theta_pid);
	compute_predicted_apogee_m();

	return theta_pid;
}


