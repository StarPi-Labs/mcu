#pragma once

#include <Arduino.h>
#include <ArduinoEigenDense.h>

using namespace Eigen;

class KalmanFilter {

private:
	Vector2f x;     // Stato x[0]: altitudine, x[1]: velocità verticale
	Vector2f u;     // Ingresso di controllo

	Matrix2f A;     // Matrice di transizione dello stato
	Matrix2f P;     // Covarianza dello stato
	RowVector2f H;  // Matrice di osservazione

	Matrix2f Q;     // Covarianze del rumore di processo
	Matrix2f Q_base;
	float R;        // Covarianza del rumore di misura

	//====== VARIANZA DEL RUMORE DI PROCESSO ======
	const float sigma_boost     = 7.0; // Boost fase
	const float sigma_coast     = 7.0; // Coast fase
	const float sigma_airbrakes = 7.0; // Airbrakes fase
	const float sigma_freefall  = 7.0; // Free fall fase

	//====== VARIANZA DEL RUMORE DI MISURA DEL BAROMETRO ======
	const float sigma_bar_launch   = 1.5; // Launch fase
	const float sigma_bar_boost    = 1.5; // Boost fase 250-500
	const float sigma_bar_coast    = 1.5; // Coast fase
	const float sigma_bar_airbrake = 1.5; // Airbrakes fase
	const float sigma_bar_freefall = 1.5; // Free fall fase

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

	const float dt = 0.01;                 // Tempo di campionamento, in secondi
	float g;    // Accelerazione gravitazionale
	static constexpr float g0 = 9.80665;

	// Valore di a che separa la fase di boost da quella di coast
	// !!! ancora da capire quanto vale !!! ancora da settare
	const float a_boost = 15;

	//====== FILTRO PASSA-BASSO SULL'ACCELERAZIONE ======
	// Filtro IIR del primo ordine: a_f += alpha_lp*(a_in - a_f)
	// con alpha_lp = dt/(RC + dt), RC = 1/(2*pi*f_cut).
	// A 100 Hz di campionamento, f_cut = 20 Hz dà alpha_lp ~ 0.56 (filtro leggero).
	// Più f_cut è basso, più il filtro è pesante (e più introduce ritardo).
	const float f_cut_acc = 20.0;          // Frequenza di taglio, in Hz
	float alpha_lp;                        // Coefficiente del filtro (calcolato nel costruttore)
	float a_filt;                          // Accelerazione verticale filtrata (senza gravità)


public:
	KalmanFilter()
	{
		g = g0; // valore di default, sovrascrivibile con setG()

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

		// Coefficiente del filtro passa-basso
		const float RC = 1.0f/(2.0f*PI*f_cut_acc);
		alpha_lp = dt/(RC + dt);

		// In rampa il razzo è fermo: a_misurata = g, quindi a - g = 0
		a_filt = 0.0;
	}

	void setG(float g_cal)
	{
		g = g_cal;
	}

	// ===== FUNZIONI PER LA SIMULAZIONE =====

	// restituisce lo stato attuale
	//   Vector3f: [0] = altitudine, [1] = velocità verticale,
	//             [2] = accelerazione verticale filtrata (senza gravità, cioè a - g)
	// Nota: l'accelerazione NON fa parte dello stato del Kalman, è solo filtrata
	// con un passa-basso e restituita in uscita.
	Vector3f getState() const
	{
		Vector3f s;
		s << x(0), x(1), a_filt;
		return s;
	}

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


	// predizione dello stato a partire dall'accelerazione misurata, dall'angolo
	// di tilt e dallo stato attuale dell'airbrake (triggerato o no)
	// Va chiamata a ogni ciclo (dt): aggiorna anche il filtro passa-basso
	// sull'accelerazione.
	//   a : accelerazione verticale misurata in G, rispetto l'asse normale alla scheda
	//   alpha : angolo di tilt rispetto alla verticale (in radianti)
	//   airbrake_trigger : se true, si è in fase di airbrakes
	void predict(float a, float alpha, bool airbrake_trigger)
	{
		//alpha è l'angolo di tilt rispetto alla verticale
		A(0,1) = dt;

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

		x = A*x + (a - g)*g0*u;
		P = A*P*A.transpose() + Q;

		// Filtro passa-basso sull'accelerazione (senza gravità)
		a_filt += alpha_lp*((a - g) - a_filt);
	}

	// aggiornamento dello stato a partire da una misura di altitudine
	//   h : altitudine misurata (in metri)
	void update(float h)
	{
		float S = H*P*H.transpose() + R;
		Vector2f K = P*H.transpose()/S;
		float z = h;

		x = x + K*(z - H*x);

		// Forma standard
		// P = (Matrix2f::Identity() - K * H) * P;

		// Forma di Joseph per mantenere la covarianza simmetrica e definita positiva
		Matrix2f I_KH = Matrix2f::Identity() - K*H;
		P = I_KH * P * I_KH.transpose() + K*R*K.transpose();
	}
};
