#ifndef AIRBRAKE_WRAPPER_H
#define AIRBRAKE_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void airbrake_init_sil(void);
float airbrake_step_sil(float altitudine, float vel_z, float tilt);
float airbrake_step_apo(float altitudine, float vel_z, float tilt);


#ifdef __cplusplus
}
#endif

#endif // AIRBRAKE_WRAPPER_H

