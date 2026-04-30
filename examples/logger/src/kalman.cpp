#include "KalmanFilter2.hpp"

#include <new>
extern "C" {
#include "PlaneNavigationFilter.h"
}


KalmanFilter::KalmanFilter() {
    PlaneNavigationFilter_initialize();
}

KalmanFilter::~KalmanFilter() {
    //PlaneNavigationFilter_terminate();
}

KalmanFilter& KalmanFilter::getInstance() {
    static KalmanFilter* pt = nullptr;
    static unsigned char kf_mem[sizeof(KalmanFilter)];
    if (!pt) {
        pt = new (kf_mem) KalmanFilter();
    }
    return *pt;
}

State KalmanFilter::getState() const {
    return State{
        .altitude = PlaneNavigationFilter_Y.EkfState.h,
        .altitude_rate = 0,
        .roll = PlaneNavigationFilter_Y.EkfState.rpy[0],
        .pitch = PlaneNavigationFilter_Y.EkfState.rpy[1],
        .yaw = PlaneNavigationFilter_Y.EkfState.rpy[2],
    };
}

void KalmanFilter::predict(float omega_x, float omega_y, float omega_z,
                           float acc_x, float acc_y, float acc_z) 
{
    PlaneNavigationFilter_U.ARM_CMD = true;
    PlaneNavigationFilter_U.GPS_POS_EN = false;

    PlaneNavigationFilter_U.acc_b[0] = omega_x;
    PlaneNavigationFilter_U.acc_b[1] = omega_y;
    PlaneNavigationFilter_U.acc_b[2] = omega_z;
    PlaneNavigationFilter_U.omega_ib[0] = acc_x;
    PlaneNavigationFilter_U.omega_ib[1] = acc_y;
    PlaneNavigationFilter_U.omega_ib[2] = acc_z;

    PlaneNavigationFilter_U.BARO_EN = false;

    PlaneNavigationFilter_step();
}

void KalmanFilter::update(float pressure_bar) {
    PlaneNavigationFilter_U.baro_pressure = pressure_bar;
    PlaneNavigationFilter_U.BARO_EN = true;
}
