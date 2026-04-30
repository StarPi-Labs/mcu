#pragma once

struct ObservedState {
    float altitude;
    float altitude_rate;
    float roll;
    float pitch;
    float yaw;
};

enum KFState {
    IDLE = 0,
    INIT,
    NOMINAL,
    DEGRADED,
};

class KalmanFilter {
private:
	KalmanFilter();
	~KalmanFilter();
	int a;

public:
        static KalmanFilter& getInstance();

	// ===== FUNZIONI PER LA SIMULAZIONE =====

	ObservedState getState() const;
        KFState getKFState() const;
	void predict(float omega_x, float omega_y, float omega_z,
                           float acc_x, float acc_y, float acc_z);
	void update(float pressure_bar, float temperature_kelvin);
};
