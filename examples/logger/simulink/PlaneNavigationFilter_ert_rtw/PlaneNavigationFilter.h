/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: PlaneNavigationFilter.h
 *
 * Code generated for Simulink model 'PlaneNavigationFilter'.
 *
 * Model version                  : 1.103
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Sat May  2 21:06:10 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Custom Processor->Custom Processor
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef PlaneNavigationFilter_h_
#define PlaneNavigationFilter_h_
#ifndef PlaneNavigationFilter_COMMON_INCLUDES_
#define PlaneNavigationFilter_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                              /* PlaneNavigationFilter_COMMON_INCLUDES_ */

#include "PlaneNavigationFilter_types.h"
#include "rtGetInf.h"
#include "rtGetNaN.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
    real32_T a__1[864];
    real32_T y[864];
    real32_T fv[540];
    real32_T fv1[540];
    real32_T a__1_m[486];
    real32_T y_c[486];
    real32_T a__1_k[360];
    real32_T y_cx[360];
    real32_T a__1_b[342];
    real32_T y_p[342];
    real32_T A[324];
    real32_T y_cv[324];
    real32_T y_f[324];
    real32_T y_g[324];
    real32_T y_g1[324];
    real32_T P[324];                   /* '<S9>/MATLAB Function' */
    real32_T a__1_me[243];
    real32_T K[243];
    real32_T K_n[162];
    real32_T C[162];
    real32_T Sy[81];
    real32_T R[81];
    real32_T a__1_p[40];
    real32_T K_l[40];
    real32_T K_j[36];
    real32_T C_d[36];
    real32_T C_g[36];
    real32_T fv2[30];
    real32_T M[19];
    real32_T Pxy[18];
    real32_T dHdx[18];
    real32_T C_l[18];
    real32_T ekf_state_std[18];        /* '<Root>/Sqrt' */
    real32_T b_tau[18];
    real32_T work[18];
    real32_T b_tau_d[18];
    real32_T work_d[18];
    real32_T b_tau_l[18];
    real32_T work_o[18];
    real32_T b_tau_b[18];
    real32_T work_n[18];
    real32_T y_b[18];
    real32_T rtb_TmpSignalConversionAtMATLAB[9];
    real32_T b_tau_ln[9];
    real32_T work_h[9];
    real32_T R_ib[9];
    real32_T fv3[9];
    real32_T TmpSignalConversionAtMATLABSystemInp[6];
    real32_T Sy_b[4];
    real32_T R_d[4];
    real32_T fv4[3];
    real32_T b_tau_e[2];
    real32_T work_b[2];
    real32_T bkj;
    real32_T residue;
    real32_T t3;
    real32_T t4;
    real32_T t5;
    real32_T t6;
    real32_T t7;
    real32_T t8;
    real32_T t22;
    real32_T t23;
    real32_T t24;
    real32_T t25;
    real32_T t27;
    real32_T t28;
    real32_T t29;
    real32_T t30;
    real32_T t11;
    real32_T t12;
    real32_T t13;
    real32_T t14;
    real32_T t15;
    real32_T t16;
    real32_T t17;
    real32_T t31;
    real32_T t32;
    real32_T t33;
    real32_T t18;
    real32_T t19;
    real32_T t20;
    real32_T t21;
    real32_T t40;
    real32_T t41;
    real32_T t43;
    real32_T t50;
    real32_T t51;
    real32_T t54;
    real32_T t52;
    real32_T t53;
    real32_T atmp;
    real32_T beta1;
    real32_T atmp_j;
    real32_T beta1_f;
    real32_T atmp_a;
    real32_T beta1_j;
    real32_T atmp_jz;
    real32_T beta1_o;
    real32_T atmp_n;
    real32_T beta1_i;
    real32_T atmp_o;
    real32_T beta1_n;
    int32_T b_k;
    int32_T coffset;
    int32_T bkj_m;
    int32_T c_i;
    int32_T i;
    int32_T K_tmp;
    int32_T iaii;
    int32_T ii;
    int32_T iaii_c;
    int32_T ii_m;
    int32_T iaii_m;
    int32_T ii_j;
    int32_T iaii_h;
    int32_T ii_c;
    int32_T iaii_ct;
    int32_T ii_p;
    int32_T iaii_p;
    int32_T ii_a;
    boolean_T MATLABSystem_o3_f;       /* '<S6>/MATLAB System' */
} B_PlaneNavigationFilter_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
    real32_T Delay_DSTATE[18];         /* '<Root>/Delay' */
    real32_T P[324];                   /* '<S2>/DataStoreMemory - P' */
    real32_T x[18];                    /* '<S2>/DataStoreMemory - x' */
    uint32_T durationCounter_2;        /* '<Root>/EKF Logic' */
    uint32_T durationCounter_1;        /* '<Root>/EKF Logic' */
    uint8_T is_active_c3_PlaneNavigationFilter;/* '<Root>/EKF Logic' */
    uint8_T is_EKF_LOGIC;              /* '<Root>/EKF Logic' */
} DW_PlaneNavigationFilter_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
    boolean_T ARM_CMD;                 /* '<Root>/ARM_CMD' */
    boolean_T GPS_POS_EN;              /* '<Root>/GPS_POS_EN' */
    real32_T acc_b[3];                 /* '<Root>/acc_b' */
    real32_T omega_ib[3];              /* '<Root>/omega_ib' */
    real32_T xy_pos[2];                /* '<Root>/xy_pos' */
    real32_T baro_pressure;            /* '<Root>/baro_pressure' */
    EKFInitData init_input;            /* '<Root>/init_input' */
    boolean_T HAS_BARO_DATA;           /* '<Root>/HAS_BARO_DATA' */
} ExtU_PlaneNavigationFilter_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
    EKFNavigationState EkfState;       /* '<Root>/EkfState' */
    EKFNavigationState EkfStateStd;    /* '<Root>/EkfStateStd' */
    EKFStage EKF_STAGE;                /* '<Root>/EKF_STAGE' */
} ExtY_PlaneNavigationFilter_T;

/* Parameters (default storage) */
struct P_PlaneNavigationFilter_T_ {
    real32_T ekf_std_init_thresholds[18];/* Variable: ekf_std_init_thresholds
                                          * Referenced by: '<Root>/EKF Logic'
                                          */
    real32_T initState[18];            /* Variable: initState
                                        * Referenced by: '<Root>/Delay'
                                        */
    real32_T Q_Value[900];             /* Expression: p.Q
                                        * Referenced by: '<S2>/Q'
                                        */
    real32_T R3_Value;                 /* Expression: p.R{3}
                                        * Referenced by: '<S2>/R3'
                                        */
    real32_T MeasurementFcn3Inputs_Value;
                              /* Computed Parameter: MeasurementFcn3Inputs_Value
                               * Referenced by: '<S2>/MeasurementFcn3Inputs'
                               */
    real32_T R2_Value[4];              /* Expression: p.R{2}
                                        * Referenced by: '<S2>/R2'
                                        */
    real32_T MeasurementFcn2Inputs_Value;
                              /* Computed Parameter: MeasurementFcn2Inputs_Value
                               * Referenced by: '<S2>/MeasurementFcn2Inputs'
                               */
    real32_T zerovel_Value[3];         /* Computed Parameter: zerovel_Value
                                        * Referenced by: '<S3>/zero vel'
                                        */
    real32_T zeroh_Value;              /* Computed Parameter: zeroh_Value
                                        * Referenced by: '<S3>/zero h'
                                        */
    real32_T R1_Value[81];             /* Expression: p.R{1}
                                        * Referenced by: '<S2>/R1'
                                        */
    real32_T MeasurementFcn1Inputs_Value;
                              /* Computed Parameter: MeasurementFcn1Inputs_Value
                               * Referenced by: '<S2>/MeasurementFcn1Inputs'
                               */
    real32_T DataStoreMemoryP_InitialValue[324];/* Expression: p.InitialCovariance
                                                 * Referenced by: '<S2>/DataStoreMemory - P'
                                                 */
    real32_T DataStoreMemoryx_InitialValue[18];/* Expression: p.InitialState
                                                * Referenced by: '<S2>/DataStoreMemory - x'
                                                */
    boolean_T yBlockOrdering_Y0;       /* Computed Parameter: yBlockOrdering_Y0
                                        * Referenced by: '<S6>/yBlockOrdering'
                                        */
    boolean_T yBlockOrdering_Y0_c;    /* Computed Parameter: yBlockOrdering_Y0_c
                                       * Referenced by: '<S7>/yBlockOrdering'
                                       */
    boolean_T yBlockOrdering_Y0_a;    /* Computed Parameter: yBlockOrdering_Y0_a
                                       * Referenced by: '<S8>/yBlockOrdering'
                                       */
    boolean_T BlockOrdering_Value;     /* Expression: true()
                                        * Referenced by: '<S2>/BlockOrdering'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_PlaneNavigationFilter_T {
    const char_T * volatile errorStatus;
};

/* Block parameters (default storage) */
extern P_PlaneNavigationFilter_T PlaneNavigationFilter_P;

/* Block signals (default storage) */
extern B_PlaneNavigationFilter_T PlaneNavigationFilter_B;

/* Block states (default storage) */
extern DW_PlaneNavigationFilter_T PlaneNavigationFilter_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_PlaneNavigationFilter_T PlaneNavigationFilter_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_PlaneNavigationFilter_T PlaneNavigationFilter_Y;

/* Model entry point functions */
extern void PlaneNavigationFilter_initialize(void);
extern void PlaneNavigationFilter_step(void);
extern void PlaneNavigationFilter_terminate(void);

/* Real-time Model object */
extern RT_MODEL_PlaneNavigationFilter_T *const PlaneNavigationFilter_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S2>/checkMeasurementFcn1Signals' : Unused code path elimination
 * Block '<S2>/checkMeasurementFcn2Signals' : Unused code path elimination
 * Block '<S2>/checkMeasurementFcn3Signals' : Unused code path elimination
 * Block '<S2>/checkStateTransitionFcnSignals' : Unused code path elimination
 * Block '<S2>/DataTypeConversion_Enable1' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_Enable2' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_Enable3' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_Q' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_R1' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_R2' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_R3' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_uMeas1' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_uMeas2' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_uMeas3' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_uState' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_y1' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_y2' : Eliminate redundant data type conversion
 * Block '<S2>/DataTypeConversion_y3' : Eliminate redundant data type conversion
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'PlaneNavigationFilter'
 * '<S1>'   : 'PlaneNavigationFilter/EKF Logic'
 * '<S2>'   : 'PlaneNavigationFilter/Extended Kalman Filter'
 * '<S3>'   : 'PlaneNavigationFilter/From Init Bus'
 * '<S4>'   : 'PlaneNavigationFilter/To State Bus'
 * '<S5>'   : 'PlaneNavigationFilter/To State Bus1'
 * '<S6>'   : 'PlaneNavigationFilter/Extended Kalman Filter/Correct1'
 * '<S7>'   : 'PlaneNavigationFilter/Extended Kalman Filter/Correct2'
 * '<S8>'   : 'PlaneNavigationFilter/Extended Kalman Filter/Correct3'
 * '<S9>'   : 'PlaneNavigationFilter/Extended Kalman Filter/Output'
 * '<S10>'  : 'PlaneNavigationFilter/Extended Kalman Filter/Predict'
 * '<S11>'  : 'PlaneNavigationFilter/Extended Kalman Filter/Output/MATLAB Function'
 * '<S12>'  : 'PlaneNavigationFilter/To State Bus/Subsystem'
 * '<S13>'  : 'PlaneNavigationFilter/To State Bus1/Subsystem'
 */
#endif                                 /* PlaneNavigationFilter_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
