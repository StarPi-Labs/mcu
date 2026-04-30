/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: PlaneNavigationFilter_types.h
 *
 * Code generated for Simulink model 'PlaneNavigationFilter'.
 *
 * Model version                  : 1.65
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Thu Apr 30 13:25:11 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Custom Processor->Custom Processor
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef PlaneNavigationFilter_types_h_
#define PlaneNavigationFilter_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_EKFNavigationState_
#define DEFINED_TYPEDEF_FOR_EKFNavigationState_

typedef struct {
  real32_T vel_body[3];
  real32_T rpy[3];
  real32_T g;
  real32_T h;
  real32_T acc_bias[3];
  real32_T gyro_bias[3];
  real32_T P0;
  real32_T T0;
  real32_T xy[2];
} EKFNavigationState;

#endif

#ifndef struct_tag_0SSklavDaiRqGKETUwPKsG
#define struct_tag_0SSklavDaiRqGKETUwPKsG

struct tag_0SSklavDaiRqGKETUwPKsG
{
  int32_T isInitialized;
};

#endif                                 /* struct_tag_0SSklavDaiRqGKETUwPKsG */

#ifndef typedef_matlabshared_tracking_interna_T
#define typedef_matlabshared_tracking_interna_T

typedef struct tag_0SSklavDaiRqGKETUwPKsG matlabshared_tracking_interna_T;

#endif                             /* typedef_matlabshared_tracking_interna_T */

#ifndef struct_tag_f81lBFSPl3q2qFFGaCCVaE
#define struct_tag_f81lBFSPl3q2qFFGaCCVaE

struct tag_f81lBFSPl3q2qFFGaCCVaE
{
  int32_T isInitialized;
};

#endif                                 /* struct_tag_f81lBFSPl3q2qFFGaCCVaE */

#ifndef typedef_matlabshared_tracking_inter_d_T
#define typedef_matlabshared_tracking_inter_d_T

typedef struct tag_f81lBFSPl3q2qFFGaCCVaE matlabshared_tracking_inter_d_T;

#endif                             /* typedef_matlabshared_tracking_inter_d_T */

/* Parameters (default storage) */
typedef struct P_PlaneNavigationFilter_T_ P_PlaneNavigationFilter_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_PlaneNavigationFilter_T RT_MODEL_PlaneNavigationFilte_T;

#endif                                 /* PlaneNavigationFilter_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
