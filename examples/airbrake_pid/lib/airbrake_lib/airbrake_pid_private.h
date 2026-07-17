
#ifndef airbrake_pid_private_h_
#define airbrake_pid_private_h_
#include "rtwtypes.h"
#include "airbrake_pid_types.h"
#include "airbrake_pid.h"

extern real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
extern real_T look1_binlcapw(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
extern real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const
  real_T bp1[], const real_T table[], const uint32_T maxIndex[], uint32_T stride);

#endif                                 // airbrake_pid_private_h_

