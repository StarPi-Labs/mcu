#include "airbrake_wrapper.h"
#include "airbrake_pid.h"

static AirbrakePID controllore;

extern "C" {

void airbrake_init_sil(void) {
    controllore.initialize();
}

float airbrake_step_sil(float altitudine, float vel_z, float tilt) {
    controllore.step(altitudine, vel_z, tilt);

    float angolo = controllore.getTargetAngle();
    
    return angolo;
}

float airbrake_step_apo(float altitudine, float vel_z, float tilt) {
    controllore.step(altitudine, vel_z, tilt);

    float apogeo = controllore.getEstimatedApogee();

    return apogeo;
}
}
