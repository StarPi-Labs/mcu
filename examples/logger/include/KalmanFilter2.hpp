#pragma once

struct State {
    float altitude;
    float altitude_rate;
    float roll;
    float pitch;
    float yaw;
};

class KalmanFilter {
private:
	KalmanFilter();
	~KalmanFilter();
	int a;

public:
        static KalmanFilter& getInstance();

	// ===== FUNZIONI PER LA SIMULAZIONE =====

	// restituisce lo stato attuale (altitudine e velocità verticale)
	//   Vector2f: x(0) = altitudine, x(1) = velocità verticale
	State getState() const;
	void predict(float omega_x, float omega_y, float omega_z,
                           float acc_x, float acc_y, float acc_z);
	void update(float pressure_bar);
};
