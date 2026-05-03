/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: PlaneNavigationFilter_types.h
 *
 * Code generated for Simulink model 'PlaneNavigationFilter'.
 *
 * Model version                  : 1.113
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Sun May  3 16:41:48 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Custom Processor->Custom Processor
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef PlaneNavigationFilter_types_h_
#define PlaneNavigationFilter_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_EKFInitData_
#define DEFINED_TYPEDEF_FOR_EKFInitData_

typedef struct {
    real32_T roll_pitch_init[2];
    real32_T T0;
    real32_T P0;
} EKFInitData;

#endif

#ifndef DEFINED_TYPEDEF_FOR_EKFStage_
#define DEFINED_TYPEDEF_FOR_EKFStage_

typedef int8_T EKFStage;

/* enum EKFStage */
#define IDLE                           ((EKFStage)0)             /* Default value */
#define INIT                           ((EKFStage)1)
#define NOMINAL                        ((EKFStage)2)
#define DEGRADED                       ((EKFStage)3)
#endif

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

#ifndef typedef_matlabshared_tracking_internal_blocks__T
#define typedef_matlabshared_tracking_internal_blocks__T

typedef struct tag_0SSklavDaiRqGKETUwPKsG
    matlabshared_tracking_internal_blocks__T;

#endif                    /* typedef_matlabshared_tracking_internal_blocks__T */

#ifndef struct_tag_f81lBFSPl3q2qFFGaCCVaE
#define struct_tag_f81lBFSPl3q2qFFGaCCVaE

struct tag_f81lBFSPl3q2qFFGaCCVaE
{
    int32_T isInitialized;
};

#endif                                 /* struct_tag_f81lBFSPl3q2qFFGaCCVaE */

#ifndef typedef_matlabshared_tracking_internal_block_d_T
#define typedef_matlabshared_tracking_internal_block_d_T

typedef struct tag_f81lBFSPl3q2qFFGaCCVaE
    matlabshared_tracking_internal_block_d_T;

#endif                    /* typedef_matlabshared_tracking_internal_block_d_T */

/* Parameters (default storage) */
typedef struct P_PlaneNavigationFilter_T_ P_PlaneNavigationFilter_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_PlaneNavigationFilter_T RT_MODEL_PlaneNavigationFilter_T;

#endif                                 /* PlaneNavigationFilter_types_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
