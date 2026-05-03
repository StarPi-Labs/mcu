#pragma once

#include <Arduino.h>
#include <MadgwickAHRS.h>
#include <cmath>
#include <new>

#include "board.h"

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
    // Altitude/velocity state: x[0]=h, x[1]=v
    float x[2];
    // 2x2 covariance, row-major: P[0]=P00, P[1]=P01, P[2]=P10, P[3]=P11
    float P[4];
    float R;

    mutable Madgwick madgwick;

    // Reference pressure stored on first update() call
    float P0;
    bool P0_set;

    static constexpr float dt    = 1.0f / IMU_TASK_HZ;
    static constexpr float g0    = 9.80665f;

    // Process noise gain scheduling (ported from old KalmanFilter.hpp)
    static constexpr float sigma_boost     = 7.0f;
    static constexpr float sigma_coast     = 7.0f;
    static constexpr float sigma_airbrakes = 7.0f;
    static constexpr float sigma_freefall  = 7.0f;

    // Measurement noise
    static constexpr float sigma_bar = 1.5f;

    // Threshold separating boost from coast (vertical accel in m/s²)
    static constexpr float a_boost = 15.0f * g0;


    public:
    KalmanFilter()
    {
        x[0] = 0.0f; x[1] = 0.0f;
        P[0] = sigma_bar; P[1] = 0.0f;
        P[2] = 0.0f;      P[3] = 0.001f;
        R = sigma_bar;
        P0 = 101325.0f;
        P0_set = false;
        madgwick.begin(IMU_TASK_HZ);
    }

    ~KalmanFilter() {}

    ObservedState getState() const
    {
        return {
            .altitude      = x[0],
            .altitude_rate = 0.0f,
            .roll          = madgwick.getRoll(),
            .pitch         = madgwick.getPitch(),
            .yaw           = madgwick.getYaw(),
        };
    }

    KFState getKFState() const { return NOMINAL; }

    // omega_* in rad/s, acc_* in m/s²
    void predict(float omega_x, float omega_y, float omega_z,
                 float acc_x,   float acc_y,   float acc_z)
    {
        // Madgwick normalises acceleration internally; pass in g
        madgwick.updateIMU(omega_x, omega_y, omega_z,
                           acc_x / g0, acc_y / g0, acc_z / g0);

        // Gain scheduling on vertical acceleration magnitude
        float a_vert = acc_z;
        float sigma;
        if (a_vert > a_boost) {
            sigma = sigma_boost;
        } else if (x[1] > 0.0f) {
            sigma = sigma_coast;
        } else if (x[1] * cosf(madgwick.getPitchRadians()) > 0.0f) {
            sigma = sigma_airbrakes;
        } else {
            sigma = sigma_freefall;
        }

        float dt2 = dt*dt, dt3 = dt2*dt, dt4 = dt3*dt;
        float Q00 = sigma * dt4 / 4.0f;
        float Q01 = sigma * dt3 / 2.0f;
        float Q11 = sigma * dt2;

        // x = A*x,  A = [[1, dt], [0, 1]]
        x[0] += dt * x[1];
        // x[1] unchanged (constant-velocity model; baro corrects via update)

        // P = A*P*A^T + Q
        float p00 = P[0] + dt*(P[1] + P[2]) + dt2*P[3] + Q00;
        float p01 = P[1] + dt*P[3] + Q01;
        float p10 = P[2] + dt*P[3] + Q01;
        float p11 = P[3] + Q11;
        P[0] = p00; P[1] = p01; P[2] = p10; P[3] = p11;
    }

    // pressure in same unit as P0 (any consistent unit); temperature unused here
    void update(float pressure, float temperature_kelvin)
    {
        (void)temperature_kelvin;

        if (!P0_set) {
            P0 = pressure;
            P0_set = true;
        }

        // Barometric altitude relative to launch site
        float h = 44330.0f * (1.0f - powf(pressure / P0, 0.190295f));

        // Kalman update, H = [1, 0]
        float S  = P[0] + R;
        float K0 = P[0] / S;
        float K1 = P[2] / S;
        float innov = h - x[0];
        x[0] += K0 * innov;
        x[1] += K1 * innov;

        // P = (I - K*H)*P
        float p00 = (1.0f - K0) * P[0];
        float p01 = (1.0f - K0) * P[1];
        float p10 = P[2] - K1 * P[0];
        float p11 = P[3] - K1 * P[1];
        P[0] = p00; P[1] = p01; P[2] = p10; P[3] = p11;
    }
};
