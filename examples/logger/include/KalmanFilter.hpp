#pragma once

#include <Arduino.h>
#include <ArduinoEigenDense.h>

using namespace Eigen;

class KalmanFilter {

private:
	Vector2f x;     // Stato
	Vector2f u;     // Ingresso di controllo

	Matrix2f A;     // Matrice di transizione dello stato
	Matrix2f P;     // Covarianza dello stato
	RowVector2f H;  // Matrice di osservazione

	Matrix2f Q;     // Covarianze del rumore di processo
	Matrix2f Q_base;
	float R;        // Covarianza del rumore di misura

	//====== VARIANZA DEL RUMORE DI PROCESSO ======
	const float sigma_boost = 250.0;      // Boost fase
	const float sigma_coast = 7.0;        // Coast fase
	const float sigma_airbrakes = 75.0;   // Airbrakes fase
	const float sigma_freefall = 10000.0; // Free fall fase

	//====== VARIANZA DEL RUMORE DI MISURA DEL BAROMETRO ======
	const float sigma_bar_launch = 1.0;    // Launch fase
	const float sigma_bar_boost = 500.0;   // Boost fase 250-500
	const float sigma_bar_coast = 1.0;     // Coast fase
	const float sigma_bar_airbrake = 17.0; // Airbrakes fase
	const float sigma_bar_freefall = 3.0;  // Free fall fase

	/* Un'alternativa sarebbe questa:
	const float sigma_boost = 300.0;      // Boost fase
	const float sigma_coast = 6.0;        // Coast fase
	const float sigma_airbrakes = 100.0;  // Airbrakes fase
	const float sigma_freefall = 10000.0; // Free fall fase

	const float sigma_bar_launch = 1.0;    // Launch fase
	const float sigma_bar_boost = 3500.0;  // Boost fase
	const float sigma_bar_coast = 1.0;     // Coast fase
	const float sigma_bar_highdrag = 20.0; // Airbrakes fase
	const float sigma_bar_freefall = 3.0;  // Free fall fase*/

	const float dt = 0.02;                 // Tempo di campionamento
	static constexpr float g = 9.80665;    // Accelerazione gravitazionale

	// Valore di a che separa la fase di boost da quella di coast
	// !!! ancora da capire quanto vale !!! ancora da settare
	const float a_boost = 15;


public:
	KalmanFilter()
	{
		x << 0, 0;
		u << dt*dt/2.0, dt;

		A << 1, dt, 0, 1;
		P << sigma_bar_launch, 0, 0, 0.001;
		H << 1.0, 0.0;

		Q << 0, 0, 0, 0;
		Q_base(0,0) = dt*dt*dt*dt/4.0;
		Q_base(0,1) = dt*dt*dt/2.0;
		Q_base(1,0) = dt*dt*dt/2.0;
		Q_base(1,1) = dt*dt;

		R = sigma_bar_boost;
	}

	// ===== FUNZIONI PER LA SIMULAZIONE =====
	Vector2f getState() const { return x;}

	void getSigmaConstants(float &sb, float &sc, float &sa, float &sf) const
	{
		sb = sigma_boost;
		sc = sigma_coast;
		sa = sigma_airbrakes;
		sf = sigma_freefall;
	}

	void getSigmaBarConstants(float &rb, float &rc, float &ra, float &rf) const
	{
		rb = sigma_bar_boost;
		rc = sigma_bar_coast;
		ra = sigma_bar_airbrake;
		rf = sigma_bar_freefall;
	}


	void predict(float a, float alpha, bool airbrake_trigger)
	{
		//alpha è l'angolo di tilt rispetto alla verticale
		A(0,1) = dt * cos(alpha);

		if (!airbrake_trigger) {
			if (a > a_boost) {
				// Boost  !!! Bisogna valutare le condizioni per capire bene la logica del gain scheduling
				Q = sigma_boost*Q_base;
				R = sigma_bar_boost;
			} else {
				// Coast
				Q = sigma_coast*Q_base;
				R = sigma_bar_coast;
			}
		} else {
			// Airbrakes
			if (x(1)*cos(alpha) > 0) {
				// se la velocità verticale v=x(1)*cos(alpha) è positiva (cioè verso l'alto)
				Q = sigma_airbrakes*Q_base;
				R = sigma_bar_airbrake;
			} else {
				// Free fall
				Q = sigma_freefall*Q_base;
				R = sigma_bar_freefall;
			}
		}

		x = A*x + (a*cos(alpha) - g)*u;
		P = A*P*A.transpose() + Q;

	}

	void update(float h)
	{
		float S = H*P*H.transpose() + R;

		Vector2f K = P*H.transpose()/S;

		float z = h;

		x = x + K*(z - H*x);
		P = (Matrix2f::Identity() - K * H) * P;
	}
};