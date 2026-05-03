/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: PlaneNavigationFilter.c
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

#include "PlaneNavigationFilter.h"
#include "rtwtypes.h"
#include <math.h>
#include <string.h>
#include "PlaneNavigationFilter_types.h"
#include "PlaneNavigationFilter_private.h"
#include "rt_nonfinite.h"

/* Named constants for Chart: '<Root>/EKF Logic' */
#define PlaneNavigationFilter_IN_EKF_ACTIVE ((uint8_T)1U)
#define PlaneNavigationFilter_IN_EKF_INIT ((uint8_T)2U)
#define PlaneNavigationFilter_IN_IDLE  ((uint8_T)3U)

/* Block signals (default storage) */
B_PlaneNavigationFilter_T PlaneNavigationFilter_B;

/* Block states (default storage) */
DW_PlaneNavigationFilter_T PlaneNavigationFilter_DW;

/* External inputs (root inport signals with default storage) */
ExtU_PlaneNavigationFilter_T PlaneNavigationFilter_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_PlaneNavigationFilter_T PlaneNavigationFilter_Y;

/* Real-time model */
static RT_MODEL_PlaneNavigationFilter_T PlaneNavigationFilter_M_;
RT_MODEL_PlaneNavigationFilter_T *const PlaneNavigationFilter_M =
    &PlaneNavigationFilter_M_;

/* Forward declaration for local functions */
static real32_T PlaneNavigationFilter_xnrm2(int32_T n, const real32_T x[243],
    int32_T ix0);
static void PlaneNavigationFilter_qr(const real32_T A[243], real32_T Q[243],
    real32_T R[81]);
static void PlaneNavigationFilter_trisolve(const real32_T A[81], real32_T B[162]);
static void PlaneNavigationFilter_trisolve_d(const real32_T A[81], real32_T B
    [162]);
static real32_T PlaneNavigationFilter_xnrm2_d(int32_T n, const real32_T x[486],
    int32_T ix0);
static void PlaneNavigationFilter_qr_d(const real32_T A[486], real32_T Q[486],
    real32_T R[324]);
static real32_T PlaneNavigationFilter_xnrm2_dk(int32_T n, const real32_T x[40],
    int32_T ix0);
static void PlaneNavigationFilter_qr_dk(const real32_T A[40], real32_T Q[40],
    real32_T R[4]);
static void PlaneNavigationFilter_trisolve_dk(const real32_T A[4], real32_T B[36]);
static void PlaneNavigationFilter_trisolve_dk0(const real32_T A[4], real32_T B
    [36]);
static real32_T PlaneNavigationFilter_xnrm2_dk0(int32_T n, const real32_T x[360],
    int32_T ix0);
static void PlaneNavigationFilter_qr_dk0(const real32_T A[360], real32_T Q[360],
    real32_T R[324]);
static real32_T PlaneNavigationFilter_xnrm2_dk0c(int32_T n, const real32_T x[19],
    int32_T ix0);
static void EKFCorrectorAdditive_getMeasurementJacob(real32_T Rs, const real32_T
    x[18], const real32_T S[324], real32_T *zEstimated, real32_T Pxy[18],
    real32_T *Sy, real32_T dHdx[18], real32_T *Rsqrt);
static void PlaneNavigationFilter_trisolve_dk0c(real32_T A, real32_T B[18]);
static real32_T PlaneNavigationFilter_xnrm2_dk0ct(int32_T n, const real32_T x
    [342], int32_T ix0);
static void PlaneNavigationFilter_qr_dk0c(const real32_T A[342], real32_T Q[342],
    real32_T R[324]);
static real32_T PlaneNavigationFilter_xnrm2_dk0ctr(int32_T n, const real32_T x
    [864], int32_T ix0);
static void PlaneNavigationFilter_qr_dk0ct(const real32_T A[864], real32_T Q[864],
    real32_T R[324]);
static void PlaneNavigation_nav_state_trans_dt_100Hz(const real32_T state[18],
    const real32_T w[30], const real32_T u[6], real32_T x_next[18]);
int32_T div_nde_s32_floor(int32_T numerator, int32_T denominator)
{
    return (((numerator < 0) != (denominator < 0)) && (numerator % denominator
             != 0) ? -1 : 0) + numerator / denominator;
}

static real32_T PlaneNavigationFilter_xnrm2(int32_T n, const real32_T x[243],
    int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
    return y;
}

real32_T rt_hypotf_snf(real32_T u0, real32_T u1)
{
    real32_T a;
    real32_T b;
    real32_T y;
    a = fabsf(u0);
    b = fabsf(u1);
    if (a < b) {
        a /= b;
        y = sqrtf(a * a + 1.0F) * b;
    } else if (a > b) {
        b /= a;
        y = sqrtf(b * b + 1.0F) * a;
    } else if (rtIsNaNF(b)) {
        y = (rtNaNF);
    } else {
        y = a * 1.41421354F;
    }

    return y;
}

static void PlaneNavigationFilter_qr(const real32_T A[243], real32_T Q[243],
    real32_T R[81])
{
    int32_T b;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;
    for (i = 0; i < 9; i++) {
        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_B.b_tau_ln[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    memcpy(&Q[0], &A[0], 243U * sizeof(real32_T));
    for (i = 0; i < 9; i++) {
        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_B.work_h[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    for (PlaneNavigationFilter_B.iaii_ct = 0; PlaneNavigationFilter_B.iaii_ct <
            9; PlaneNavigationFilter_B.iaii_ct++) {
        PlaneNavigationFilter_B.ii_p = PlaneNavigationFilter_B.iaii_ct * 27 +
            PlaneNavigationFilter_B.iaii_ct;
        i = PlaneNavigationFilter_B.ii_p + 2;
        PlaneNavigationFilter_B.atmp_n = Q[PlaneNavigationFilter_B.ii_p];
        PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct] = 0.0F;
        PlaneNavigationFilter_B.beta1_i = PlaneNavigationFilter_xnrm2(26 -
            PlaneNavigationFilter_B.iaii_ct, Q, PlaneNavigationFilter_B.ii_p + 2);
        if (PlaneNavigationFilter_B.beta1_i != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii_p];
            PlaneNavigationFilter_B.beta1_i = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1_i);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1_i =
                    -PlaneNavigationFilter_B.beta1_i;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1_i) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b = PlaneNavigationFilter_B.ii_p -
                        PlaneNavigationFilter_B.iaii_ct;
                    for (lastv = i; lastv <= b + 27; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1_i *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp_n *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1_i) <
                                9.86076132E-32F) && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1_i = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp_n, PlaneNavigationFilter_xnrm2
                     (26 - PlaneNavigationFilter_B.iaii_ct, Q,
                      PlaneNavigationFilter_B.ii_p + 2));
                if (PlaneNavigationFilter_B.atmp_n >= 0.0F) {
                    PlaneNavigationFilter_B.beta1_i =
                        -PlaneNavigationFilter_B.beta1_i;
                }

                PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct]
                    = (PlaneNavigationFilter_B.beta1_i -
                       PlaneNavigationFilter_B.atmp_n) /
                    PlaneNavigationFilter_B.beta1_i;
                PlaneNavigationFilter_B.atmp_n = 1.0F /
                    (PlaneNavigationFilter_B.atmp_n -
                     PlaneNavigationFilter_B.beta1_i);
                for (lastv = i; lastv <= b + 27; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_n;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1_i *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp_n = PlaneNavigationFilter_B.beta1_i;
            } else {
                PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct]
                    = (PlaneNavigationFilter_B.beta1_i - c) /
                    PlaneNavigationFilter_B.beta1_i;
                PlaneNavigationFilter_B.atmp_n = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1_i);
                b = PlaneNavigationFilter_B.ii_p -
                    PlaneNavigationFilter_B.iaii_ct;
                for (lastv = i; lastv <= b + 27; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_n;
                }

                PlaneNavigationFilter_B.atmp_n = PlaneNavigationFilter_B.beta1_i;
            }
        }

        Q[PlaneNavigationFilter_B.ii_p] = PlaneNavigationFilter_B.atmp_n;
        if (PlaneNavigationFilter_B.iaii_ct + 1 < 9) {
            Q[PlaneNavigationFilter_B.ii_p] = 1.0F;
            knt = PlaneNavigationFilter_B.ii_p + 28;
            if (PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct]
                != 0.0F) {
                lastv = 27 - PlaneNavigationFilter_B.iaii_ct;
                i = PlaneNavigationFilter_B.ii_p -
                    PlaneNavigationFilter_B.iaii_ct;
                while ((lastv > 0) && (Q[i + 26] == 0.0F)) {
                    lastv--;
                    i--;
                }

                b = 8 - PlaneNavigationFilter_B.iaii_ct;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 27 + PlaneNavigationFilter_B.ii_p;
                    jA = lastc + 28;
                    do {
                        exitg1 = 0;
                        if (jA <= (lastc + lastv) + 27) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_h[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (27 * lastc + PlaneNavigationFilter_B.ii_p) + 28;
                    for (iac = knt; iac <= b; iac += 27) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii_p + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.ii_p) -
                                               28, 27);
                        PlaneNavigationFilter_B.work_h[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii_p;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_h[iac];
                        if (c != 0.0F) {
                            c *=
                                -PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_ct];
                            b = jA + 28;
                            knt = (lastv + jA) + 27;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_p + d)
                                               - jA) - 28] * c;
                            }
                        }

                        jA += 27;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii_p] = PlaneNavigationFilter_B.atmp_n;
        }
    }

    for (i = 0; i < 9; i++) {
        for (PlaneNavigationFilter_B.iaii_ct = 0;
                PlaneNavigationFilter_B.iaii_ct <= i;
                PlaneNavigationFilter_B.iaii_ct++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii_ct + 9 * i] = Q[27 * i +
                PlaneNavigationFilter_B.iaii_ct];
        }

        for (PlaneNavigationFilter_B.iaii_ct = i + 2;
                PlaneNavigationFilter_B.iaii_ct < 10;
                PlaneNavigationFilter_B.iaii_ct++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            R[(PlaneNavigationFilter_B.iaii_ct + 9 * i) - 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_B.work_h[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    for (i = 8; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii_ct = (i * 27 + i) + 27;
        if (i + 1 < 9) {
            Q[PlaneNavigationFilter_B.iaii_ct - 27] = 1.0F;
            knt = PlaneNavigationFilter_B.iaii_ct + 1;
            if (PlaneNavigationFilter_B.b_tau_ln[i] != 0.0F) {
                lastv = 27 - i;
                PlaneNavigationFilter_B.ii_p = (PlaneNavigationFilter_B.iaii_ct
                    - i) - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_p] == 0.0F))
                {
                    lastv--;
                    PlaneNavigationFilter_B.ii_p--;
                }

                b = 8 - i;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 27 + PlaneNavigationFilter_B.iaii_ct;
                    jA = lastc + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= lastc + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_h[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (27 * lastc + PlaneNavigationFilter_B.iaii_ct) + 1;
                    for (iac = knt; iac <= b; iac += 27) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii_ct + jA) - iac)
                                - 27] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii_ct)
                                               - 1, 27);
                        PlaneNavigationFilter_B.work_h[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_ln[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii_ct;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_h[iac];
                        if (c != 0.0F) {
                            c *= -PlaneNavigationFilter_B.b_tau_ln[i];
                            b = jA + 1;
                            knt = lastv + jA;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_ct
                                                + d) - jA) - 28] * c;
                            }
                        }

                        jA += 27;
                    }
                }
            }
        }

        b = PlaneNavigationFilter_B.iaii_ct - i;
        for (lastv = PlaneNavigationFilter_B.iaii_ct - 25; lastv <= b; lastv++)
        {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_ln[i];
        }

        Q[PlaneNavigationFilter_B.iaii_ct - 27] = 1.0F -
            PlaneNavigationFilter_B.b_tau_ln[i];
        for (PlaneNavigationFilter_B.ii_p = 0; PlaneNavigationFilter_B.ii_p < i;
             PlaneNavigationFilter_B.ii_p++) {
            Q[(PlaneNavigationFilter_B.iaii_ct - PlaneNavigationFilter_B.ii_p) -
                28] = 0.0F;
        }
    }
}

static void PlaneNavigationFilter_trisolve(const real32_T A[81], real32_T B[162])
{
    int32_T b_j;
    int32_T b_k;
    int32_T i;

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    for (b_j = 0; b_j < 18; b_j++) {
        int32_T jBcol;
        jBcol = 9 * b_j - 1;
        for (b_k = 0; b_k < 9; b_k++) {
            int32_T B_tmp;
            int32_T k;
            int32_T kAcol;
            real32_T B_0;
            k = b_k + 1;
            kAcol = b_k * 9 - 1;
            B_tmp = (b_k + jBcol) + 1;
            B_0 = B[B_tmp];
            if (B_0 != 0.0F) {
                B[B_tmp] = B_0 / A[(b_k + kAcol) + 1];
                for (i = k + 1; i < 10; i++) {
                    int32_T tmp;
                    tmp = i + jBcol;
                    B[tmp] -= A[i + kAcol] * B[B_tmp];
                }
            }
        }
    }

    /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
}

static void PlaneNavigationFilter_trisolve_d(const real32_T A[81], real32_T B
    [162])
{
    int32_T b_i;
    int32_T b_j;
    int32_T k;

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    for (b_j = 0; b_j < 18; b_j++) {
        int32_T jBcol;
        jBcol = 9 * b_j;
        for (k = 8; k >= 0; k--) {
            int32_T kAcol;
            int32_T tmp_0;
            real32_T tmp;
            kAcol = 9 * k;
            tmp_0 = k + jBcol;
            tmp = B[tmp_0];
            if (tmp != 0.0F) {
                B[tmp_0] = tmp / A[k + kAcol];
                for (b_i = 0; b_i < k; b_i++) {
                    int32_T tmp_1;
                    tmp_1 = b_i + jBcol;
                    B[tmp_1] -= A[b_i + kAcol] * B[tmp_0];
                }
            }
        }
    }

    /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
}

static real32_T PlaneNavigationFilter_xnrm2_d(int32_T n, const real32_T x[486],
    int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
    return y;
}

static void PlaneNavigationFilter_qr_d(const real32_T A[486], real32_T Q[486],
    real32_T R[324])
{
    int32_T b;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    memset(&PlaneNavigationFilter_B.b_tau_b[0], 0, 18U * sizeof(real32_T));
    memcpy(&Q[0], &A[0], 486U * sizeof(real32_T));
    memset(&PlaneNavigationFilter_B.work_n[0], 0, 18U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.iaii_h = 0; PlaneNavigationFilter_B.iaii_h < 18;
         PlaneNavigationFilter_B.iaii_h++) {
        PlaneNavigationFilter_B.ii_c = PlaneNavigationFilter_B.iaii_h * 27 +
            PlaneNavigationFilter_B.iaii_h;
        i = PlaneNavigationFilter_B.ii_c + 2;
        PlaneNavigationFilter_B.atmp_jz = Q[PlaneNavigationFilter_B.ii_c];
        PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h] = 0.0F;
        PlaneNavigationFilter_B.beta1_o = PlaneNavigationFilter_xnrm2_d(26 -
            PlaneNavigationFilter_B.iaii_h, Q, PlaneNavigationFilter_B.ii_c + 2);
        if (PlaneNavigationFilter_B.beta1_o != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii_c];
            PlaneNavigationFilter_B.beta1_o = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1_o);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1_o =
                    -PlaneNavigationFilter_B.beta1_o;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1_o) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b = PlaneNavigationFilter_B.ii_c -
                        PlaneNavigationFilter_B.iaii_h;
                    for (lastv = i; lastv <= b + 27; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1_o *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp_jz *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1_o) <
                                9.86076132E-32F) && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1_o = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp_jz,
                     PlaneNavigationFilter_xnrm2_d(26 -
                      PlaneNavigationFilter_B.iaii_h, Q,
                      PlaneNavigationFilter_B.ii_c + 2));
                if (PlaneNavigationFilter_B.atmp_jz >= 0.0F) {
                    PlaneNavigationFilter_B.beta1_o =
                        -PlaneNavigationFilter_B.beta1_o;
                }

                PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h] =
                    (PlaneNavigationFilter_B.beta1_o -
                     PlaneNavigationFilter_B.atmp_jz) /
                    PlaneNavigationFilter_B.beta1_o;
                PlaneNavigationFilter_B.atmp_jz = 1.0F /
                    (PlaneNavigationFilter_B.atmp_jz -
                     PlaneNavigationFilter_B.beta1_o);
                for (lastv = i; lastv <= b + 27; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_jz;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1_o *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp_jz =
                    PlaneNavigationFilter_B.beta1_o;
            } else {
                PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h] =
                    (PlaneNavigationFilter_B.beta1_o - c) /
                    PlaneNavigationFilter_B.beta1_o;
                PlaneNavigationFilter_B.atmp_jz = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1_o);
                b = PlaneNavigationFilter_B.ii_c -
                    PlaneNavigationFilter_B.iaii_h;
                for (lastv = i; lastv <= b + 27; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_jz;
                }

                PlaneNavigationFilter_B.atmp_jz =
                    PlaneNavigationFilter_B.beta1_o;
            }
        }

        Q[PlaneNavigationFilter_B.ii_c] = PlaneNavigationFilter_B.atmp_jz;
        if (PlaneNavigationFilter_B.iaii_h + 1 < 18) {
            Q[PlaneNavigationFilter_B.ii_c] = 1.0F;
            knt = PlaneNavigationFilter_B.ii_c + 28;
            if (PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h]
                    != 0.0F) {
                lastv = 27 - PlaneNavigationFilter_B.iaii_h;
                i = PlaneNavigationFilter_B.ii_c -
                    PlaneNavigationFilter_B.iaii_h;
                while ((lastv > 0) && (Q[i + 26] == 0.0F)) {
                    lastv--;
                    i--;
                }

                b = 17 - PlaneNavigationFilter_B.iaii_h;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 27 + PlaneNavigationFilter_B.ii_c;
                    jA = lastc + 28;
                    do {
                        exitg1 = 0;
                        if (jA <= (lastc + lastv) + 27) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_n[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (27 * lastc + PlaneNavigationFilter_B.ii_c) + 28;
                    for (iac = knt; iac <= b; iac += 27) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii_c + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.ii_c) -
                                               28, 27);
                        PlaneNavigationFilter_B.work_n[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii_c;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_n[iac];
                        if (c != 0.0F) {
                            c *=
                                -PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_h];
                            b = jA + 28;
                            knt = (lastv + jA) + 27;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_c + d)
                                               - jA) - 28] * c;
                            }
                        }

                        jA += 27;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii_c] = PlaneNavigationFilter_B.atmp_jz;
        }
    }

    for (i = 0; i < 18; i++) {
        for (PlaneNavigationFilter_B.iaii_h = 0; PlaneNavigationFilter_B.iaii_h <=
             i; PlaneNavigationFilter_B.iaii_h++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii_h + 18 * i] = Q[27 * i +
                PlaneNavigationFilter_B.iaii_h];
        }

        for (PlaneNavigationFilter_B.iaii_h = i + 2;
                PlaneNavigationFilter_B.iaii_h < 19;
                PlaneNavigationFilter_B.iaii_h++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            R[(PlaneNavigationFilter_B.iaii_h + 18 * i) - 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_B.work_n[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    for (i = 17; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii_h = (i * 27 + i) + 27;
        if (i + 1 < 18) {
            Q[PlaneNavigationFilter_B.iaii_h - 27] = 1.0F;
            knt = PlaneNavigationFilter_B.iaii_h + 1;
            if (PlaneNavigationFilter_B.b_tau_b[i] != 0.0F) {
                lastv = 27 - i;
                PlaneNavigationFilter_B.ii_c = (PlaneNavigationFilter_B.iaii_h -
                    i) - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_c] == 0.0F))
                {
                    lastv--;
                    PlaneNavigationFilter_B.ii_c--;
                }

                b = 17 - i;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 27 + PlaneNavigationFilter_B.iaii_h;
                    jA = lastc + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= lastc + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_n[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (27 * lastc + PlaneNavigationFilter_B.iaii_h) + 1;
                    for (iac = knt; iac <= b; iac += 27) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii_h + jA) - iac)
                                - 27] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii_h)
                                               - 1, 27);
                        PlaneNavigationFilter_B.work_n[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_b[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii_h;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_n[iac];
                        if (c != 0.0F) {
                            c *= -PlaneNavigationFilter_B.b_tau_b[i];
                            b = jA + 1;
                            knt = lastv + jA;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_h +
                                                d) - jA) - 28] * c;
                            }
                        }

                        jA += 27;
                    }
                }
            }
        }

        b = PlaneNavigationFilter_B.iaii_h - i;
        for (lastv = PlaneNavigationFilter_B.iaii_h - 25; lastv <= b; lastv++) {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_b[i];
        }

        Q[PlaneNavigationFilter_B.iaii_h - 27] = 1.0F -
            PlaneNavigationFilter_B.b_tau_b[i];
        for (PlaneNavigationFilter_B.ii_c = 0; PlaneNavigationFilter_B.ii_c < i;
             PlaneNavigationFilter_B.ii_c++) {
            Q[(PlaneNavigationFilter_B.iaii_h - PlaneNavigationFilter_B.ii_c) -
                28] = 0.0F;
        }
    }
}

static real32_T PlaneNavigationFilter_xnrm2_dk(int32_T n, const real32_T x[40],
    int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
    return y;
}

static void PlaneNavigationFilter_qr_dk(const real32_T A[40], real32_T Q[40],
    real32_T R[4])
{
    int32_T b_tmp;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    PlaneNavigationFilter_B.b_tau_e[0] = 0.0F;
    PlaneNavigationFilter_B.b_tau_e[1] = 0.0F;
    memcpy(&Q[0], &A[0], 40U * sizeof(real32_T));
    PlaneNavigationFilter_B.work_b[0] = 0.0F;
    PlaneNavigationFilter_B.work_b[1] = 0.0F;
    for (PlaneNavigationFilter_B.iaii_p = 0; PlaneNavigationFilter_B.iaii_p < 2;
         PlaneNavigationFilter_B.iaii_p++) {
        PlaneNavigationFilter_B.ii_a = PlaneNavigationFilter_B.iaii_p * 20 +
            PlaneNavigationFilter_B.iaii_p;
        i = PlaneNavigationFilter_B.ii_a + 2;
        PlaneNavigationFilter_B.atmp_o = Q[PlaneNavigationFilter_B.ii_a];
        PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p] = 0.0F;
        PlaneNavigationFilter_B.beta1_n = PlaneNavigationFilter_xnrm2_dk(19 -
            PlaneNavigationFilter_B.iaii_p, Q, PlaneNavigationFilter_B.ii_a + 2);
        if (PlaneNavigationFilter_B.beta1_n != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii_a];
            PlaneNavigationFilter_B.beta1_n = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1_n);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1_n =
                    -PlaneNavigationFilter_B.beta1_n;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1_n) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b_tmp = PlaneNavigationFilter_B.ii_a -
                        PlaneNavigationFilter_B.iaii_p;
                    for (lastv = i; lastv <= b_tmp + 20; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1_n *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp_o *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1_n) <
                                9.86076132E-32F) && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1_n = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp_o,
                     PlaneNavigationFilter_xnrm2_dk(19 -
                      PlaneNavigationFilter_B.iaii_p, Q,
                      PlaneNavigationFilter_B.ii_a + 2));
                if (PlaneNavigationFilter_B.atmp_o >= 0.0F) {
                    PlaneNavigationFilter_B.beta1_n =
                        -PlaneNavigationFilter_B.beta1_n;
                }

                PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p] =
                    (PlaneNavigationFilter_B.beta1_n -
                     PlaneNavigationFilter_B.atmp_o) /
                    PlaneNavigationFilter_B.beta1_n;
                PlaneNavigationFilter_B.atmp_o = 1.0F /
                    (PlaneNavigationFilter_B.atmp_o -
                     PlaneNavigationFilter_B.beta1_n);
                for (lastv = i; lastv <= b_tmp + 20; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_o;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1_n *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp_o = PlaneNavigationFilter_B.beta1_n;
            } else {
                PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p] =
                    (PlaneNavigationFilter_B.beta1_n - c) /
                    PlaneNavigationFilter_B.beta1_n;
                PlaneNavigationFilter_B.atmp_o = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1_n);
                knt = PlaneNavigationFilter_B.ii_a -
                    PlaneNavigationFilter_B.iaii_p;
                for (lastv = i; lastv <= knt + 20; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_o;
                }

                PlaneNavigationFilter_B.atmp_o = PlaneNavigationFilter_B.beta1_n;
            }
        }

        Q[PlaneNavigationFilter_B.ii_a] = PlaneNavigationFilter_B.atmp_o;
        if (PlaneNavigationFilter_B.iaii_p + 1 < 2) {
            Q[PlaneNavigationFilter_B.ii_a] = 1.0F;
            b_tmp = PlaneNavigationFilter_B.ii_a + 21;
            if (PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p]
                    != 0.0F) {
                lastv = 20 - PlaneNavigationFilter_B.iaii_p;
                i = PlaneNavigationFilter_B.ii_a -
                    PlaneNavigationFilter_B.iaii_p;
                while ((lastv > 0) && (Q[i + 19] == 0.0F)) {
                    lastv--;
                    i--;
                }

                knt = 1 - PlaneNavigationFilter_B.iaii_p;
                exitg2 = false;
                while ((!exitg2) && (knt > 0)) {
                    jA = PlaneNavigationFilter_B.ii_a + 21;
                    do {
                        exitg1 = 0;
                        if (jA <= (PlaneNavigationFilter_B.ii_a + lastv) + 20) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            knt = 0;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = knt - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        PlaneNavigationFilter_B.work_b[0] = 0.0F;
                    }

                    knt = (20 * lastc + PlaneNavigationFilter_B.ii_a) + 21;
                    for (iac = b_tmp; iac <= knt; iac += 20) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii_a + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.ii_a) -
                                               21, 20);
                        PlaneNavigationFilter_B.work_b[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii_a;
                    for (iac = 0; iac <= lastc; iac++) {
                        if (PlaneNavigationFilter_B.work_b[0] != 0.0F) {
                            c = PlaneNavigationFilter_B.work_b[0] *
                                -PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_p];
                            knt = jA + 21;
                            b_tmp = (lastv + jA) + 20;
                            for (d = knt; d <= b_tmp; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_a + d)
                                               - jA) - 21] * c;
                            }
                        }

                        jA += 20;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii_a] = PlaneNavigationFilter_B.atmp_o;
        }
    }

    for (PlaneNavigationFilter_B.ii_a = 0; PlaneNavigationFilter_B.ii_a < 2;
            PlaneNavigationFilter_B.ii_a++) {
        for (PlaneNavigationFilter_B.iaii_p = 0; PlaneNavigationFilter_B.iaii_p <=
             PlaneNavigationFilter_B.ii_a; PlaneNavigationFilter_B.iaii_p++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii_p + (PlaneNavigationFilter_B.ii_a <<
                1)] = Q[20 * PlaneNavigationFilter_B.ii_a +
                PlaneNavigationFilter_B.iaii_p];
        }

        if (PlaneNavigationFilter_B.ii_a + 2 <= 2) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            R[(PlaneNavigationFilter_B.ii_a << 1) + 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_B.work_b[PlaneNavigationFilter_B.ii_a] = 0.0F;
    }

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    for (i = 1; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii_p = (i * 20 + i) + 20;
        if (i + 1 < 2) {
            Q[PlaneNavigationFilter_B.iaii_p - 20] = 1.0F;
            b_tmp = PlaneNavigationFilter_B.iaii_p + 1;
            if (PlaneNavigationFilter_B.b_tau_e[i] != 0.0F) {
                lastv = 20 - i;
                PlaneNavigationFilter_B.ii_a = (PlaneNavigationFilter_B.iaii_p -
                    i) - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_a] == 0.0F))
                {
                    lastv--;
                    PlaneNavigationFilter_B.ii_a--;
                }

                knt = 1 - i;
                exitg2 = false;
                while ((!exitg2) && (knt > 0)) {
                    jA = PlaneNavigationFilter_B.iaii_p + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= PlaneNavigationFilter_B.iaii_p + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            knt = 0;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = knt - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        PlaneNavigationFilter_B.work_b[0] = 0.0F;
                    }

                    knt = (20 * lastc + PlaneNavigationFilter_B.iaii_p) + 1;
                    for (iac = b_tmp; iac <= knt; iac += 20) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii_p + jA) - iac)
                                - 20] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii_p)
                                               - 1, 20);
                        PlaneNavigationFilter_B.work_b[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_e[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii_p;
                    for (iac = 0; iac <= lastc; iac++) {
                        if (PlaneNavigationFilter_B.work_b[0] != 0.0F) {
                            c = PlaneNavigationFilter_B.work_b[0] *
                                -PlaneNavigationFilter_B.b_tau_e[i];
                            knt = jA + 1;
                            b_tmp = lastv + jA;
                            for (d = knt; d <= b_tmp; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_p +
                                                d) - jA) - 21] * c;
                            }
                        }

                        jA += 20;
                    }
                }
            }
        }

        knt = PlaneNavigationFilter_B.iaii_p - i;
        for (lastv = PlaneNavigationFilter_B.iaii_p - 18; lastv <= knt; lastv++)
        {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_e[i];
        }

        Q[PlaneNavigationFilter_B.iaii_p - 20] = 1.0F -
            PlaneNavigationFilter_B.b_tau_e[i];
        if (i - 1 >= 0) {
            Q[PlaneNavigationFilter_B.iaii_p - 21] = 0.0F;
        }
    }
}

static void PlaneNavigationFilter_trisolve_dk(const real32_T A[4], real32_T B[36])
{
    int32_T b_j;
    int32_T b_k;
    int32_T i;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    for (b_j = 0; b_j < 18; b_j++) {
        int32_T jBcol;
        jBcol = (b_j << 1) - 1;
        for (b_k = 0; b_k < 2; b_k++) {
            int32_T B_tmp;
            int32_T k;
            int32_T kAcol;
            real32_T B_0;
            k = b_k + 1;
            kAcol = (b_k << 1) - 1;
            B_tmp = (b_k + jBcol) + 1;
            B_0 = B[B_tmp];
            if (B_0 != 0.0F) {
                B[B_tmp] = B_0 / A[(b_k + kAcol) + 1];
                for (i = k + 1; i < 3; i++) {
                    B[jBcol + 2] -= A[kAcol + 2] * B[B_tmp];
                }
            }
        }
    }

    /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
}

static void PlaneNavigationFilter_trisolve_dk0(const real32_T A[4], real32_T B
    [36])
{
    int32_T b_i;
    int32_T b_j;
    int32_T k;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    for (b_j = 0; b_j < 18; b_j++) {
        int32_T jBcol;
        jBcol = b_j << 1;
        for (k = 1; k >= 0; k--) {
            int32_T kAcol;
            int32_T tmp_0;
            real32_T tmp;
            kAcol = k << 1;
            tmp_0 = k + jBcol;
            tmp = B[tmp_0];
            if (tmp != 0.0F) {
                B[tmp_0] = tmp / A[k + kAcol];
                for (b_i = 0; b_i < k; b_i++) {
                    B[jBcol] -= B[tmp_0] * A[kAcol];
                }
            }
        }
    }

    /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
}

static real32_T PlaneNavigationFilter_xnrm2_dk0(int32_T n, const real32_T x[360],
    int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
    return y;
}

static void PlaneNavigationFilter_qr_dk0(const real32_T A[360], real32_T Q[360],
    real32_T R[324])
{
    int32_T b;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    memset(&PlaneNavigationFilter_B.b_tau_l[0], 0, 18U * sizeof(real32_T));
    memcpy(&Q[0], &A[0], 360U * sizeof(real32_T));
    memset(&PlaneNavigationFilter_B.work_o[0], 0, 18U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.iaii_m = 0; PlaneNavigationFilter_B.iaii_m < 18;
         PlaneNavigationFilter_B.iaii_m++) {
        PlaneNavigationFilter_B.ii_j = PlaneNavigationFilter_B.iaii_m * 20 +
            PlaneNavigationFilter_B.iaii_m;
        i = PlaneNavigationFilter_B.ii_j + 2;
        PlaneNavigationFilter_B.atmp_a = Q[PlaneNavigationFilter_B.ii_j];
        PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m] = 0.0F;
        PlaneNavigationFilter_B.beta1_j = PlaneNavigationFilter_xnrm2_dk0(19 -
            PlaneNavigationFilter_B.iaii_m, Q, PlaneNavigationFilter_B.ii_j + 2);
        if (PlaneNavigationFilter_B.beta1_j != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii_j];
            PlaneNavigationFilter_B.beta1_j = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1_j);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1_j =
                    -PlaneNavigationFilter_B.beta1_j;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1_j) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b = PlaneNavigationFilter_B.ii_j -
                        PlaneNavigationFilter_B.iaii_m;
                    for (lastv = i; lastv <= b + 20; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1_j *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp_a *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1_j) <
                                9.86076132E-32F) && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1_j = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp_a,
                     PlaneNavigationFilter_xnrm2_dk0(19 -
                      PlaneNavigationFilter_B.iaii_m, Q,
                      PlaneNavigationFilter_B.ii_j + 2));
                if (PlaneNavigationFilter_B.atmp_a >= 0.0F) {
                    PlaneNavigationFilter_B.beta1_j =
                        -PlaneNavigationFilter_B.beta1_j;
                }

                PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m] =
                    (PlaneNavigationFilter_B.beta1_j -
                     PlaneNavigationFilter_B.atmp_a) /
                    PlaneNavigationFilter_B.beta1_j;
                PlaneNavigationFilter_B.atmp_a = 1.0F /
                    (PlaneNavigationFilter_B.atmp_a -
                     PlaneNavigationFilter_B.beta1_j);
                for (lastv = i; lastv <= b + 20; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_a;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1_j *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp_a = PlaneNavigationFilter_B.beta1_j;
            } else {
                PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m] =
                    (PlaneNavigationFilter_B.beta1_j - c) /
                    PlaneNavigationFilter_B.beta1_j;
                PlaneNavigationFilter_B.atmp_a = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1_j);
                b = PlaneNavigationFilter_B.ii_j -
                    PlaneNavigationFilter_B.iaii_m;
                for (lastv = i; lastv <= b + 20; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_a;
                }

                PlaneNavigationFilter_B.atmp_a = PlaneNavigationFilter_B.beta1_j;
            }
        }

        Q[PlaneNavigationFilter_B.ii_j] = PlaneNavigationFilter_B.atmp_a;
        if (PlaneNavigationFilter_B.iaii_m + 1 < 18) {
            Q[PlaneNavigationFilter_B.ii_j] = 1.0F;
            knt = PlaneNavigationFilter_B.ii_j + 21;
            if (PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m]
                    != 0.0F) {
                lastv = 20 - PlaneNavigationFilter_B.iaii_m;
                i = PlaneNavigationFilter_B.ii_j -
                    PlaneNavigationFilter_B.iaii_m;
                while ((lastv > 0) && (Q[i + 19] == 0.0F)) {
                    lastv--;
                    i--;
                }

                b = 17 - PlaneNavigationFilter_B.iaii_m;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 20 + PlaneNavigationFilter_B.ii_j;
                    jA = lastc + 21;
                    do {
                        exitg1 = 0;
                        if (jA <= (lastc + lastv) + 20) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_o[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (20 * lastc + PlaneNavigationFilter_B.ii_j) + 21;
                    for (iac = knt; iac <= b; iac += 20) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii_j + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.ii_j) -
                                               21, 20);
                        PlaneNavigationFilter_B.work_o[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii_j;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_o[iac];
                        if (c != 0.0F) {
                            c *=
                                -PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_m];
                            b = jA + 21;
                            knt = (lastv + jA) + 20;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_j + d)
                                               - jA) - 21] * c;
                            }
                        }

                        jA += 20;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii_j] = PlaneNavigationFilter_B.atmp_a;
        }
    }

    for (i = 0; i < 18; i++) {
        for (PlaneNavigationFilter_B.iaii_m = 0; PlaneNavigationFilter_B.iaii_m <=
             i; PlaneNavigationFilter_B.iaii_m++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii_m + 18 * i] = Q[20 * i +
                PlaneNavigationFilter_B.iaii_m];
        }

        for (PlaneNavigationFilter_B.iaii_m = i + 2;
                PlaneNavigationFilter_B.iaii_m < 19;
                PlaneNavigationFilter_B.iaii_m++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            R[(PlaneNavigationFilter_B.iaii_m + 18 * i) - 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_B.work_o[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    for (i = 17; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii_m = (i * 20 + i) + 20;
        if (i + 1 < 18) {
            Q[PlaneNavigationFilter_B.iaii_m - 20] = 1.0F;
            knt = PlaneNavigationFilter_B.iaii_m + 1;
            if (PlaneNavigationFilter_B.b_tau_l[i] != 0.0F) {
                lastv = 20 - i;
                PlaneNavigationFilter_B.ii_j = (PlaneNavigationFilter_B.iaii_m -
                    i) - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_j] == 0.0F))
                {
                    lastv--;
                    PlaneNavigationFilter_B.ii_j--;
                }

                b = 17 - i;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 20 + PlaneNavigationFilter_B.iaii_m;
                    jA = lastc + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= lastc + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_o[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (20 * lastc + PlaneNavigationFilter_B.iaii_m) + 1;
                    for (iac = knt; iac <= b; iac += 20) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii_m + jA) - iac)
                                - 20] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii_m)
                                               - 1, 20);
                        PlaneNavigationFilter_B.work_o[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_l[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii_m;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_o[iac];
                        if (c != 0.0F) {
                            c *= -PlaneNavigationFilter_B.b_tau_l[i];
                            b = jA + 1;
                            knt = lastv + jA;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_m +
                                                d) - jA) - 21] * c;
                            }
                        }

                        jA += 20;
                    }
                }
            }
        }

        b = PlaneNavigationFilter_B.iaii_m - i;
        for (lastv = PlaneNavigationFilter_B.iaii_m - 18; lastv <= b; lastv++) {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_l[i];
        }

        Q[PlaneNavigationFilter_B.iaii_m - 20] = 1.0F -
            PlaneNavigationFilter_B.b_tau_l[i];
        for (PlaneNavigationFilter_B.ii_j = 0; PlaneNavigationFilter_B.ii_j < i;
             PlaneNavigationFilter_B.ii_j++) {
            Q[(PlaneNavigationFilter_B.iaii_m - PlaneNavigationFilter_B.ii_j) -
                21] = 0.0F;
        }
    }
}

real32_T rt_powf_snf(real32_T u0, real32_T u1)
{
    real32_T y;
    if (rtIsNaNF(u0) || rtIsNaNF(u1)) {
        y = (rtNaNF);
    } else {
        real32_T tmp;
        real32_T tmp_0;
        tmp = fabsf(u0);
        tmp_0 = fabsf(u1);
        if (rtIsInfF(u1)) {
            if (tmp == 1.0F) {
                y = 1.0F;
            } else if (tmp > 1.0F) {
                if (u1 > 0.0F) {
                    y = (rtInfF);
                } else {
                    y = 0.0F;
                }
            } else if (u1 > 0.0F) {
                y = 0.0F;
            } else {
                y = (rtInfF);
            }
        } else if (tmp_0 == 0.0F) {
            y = 1.0F;
        } else if (tmp_0 == 1.0F) {
            if (u1 > 0.0F) {
                y = u0;
            } else {
                y = 1.0F / u0;
            }
        } else if (u1 == 2.0F) {
            y = u0 * u0;
        } else if ((u1 == 0.5F) && (u0 >= 0.0F)) {
            y = sqrtf(u0);
        } else if ((u0 < 0.0F) && (u1 > floorf(u1))) {
            y = (rtNaNF);
        } else {
            y = powf(u0, u1);
        }
    }

    return y;
}

static real32_T PlaneNavigationFilter_xnrm2_dk0c(int32_T n, const real32_T x[19],
    int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S8>/MATLAB System' */
    return y;
}

static void EKFCorrectorAdditive_getMeasurementJacob(real32_T Rs, const real32_T
    x[18], const real32_T S[324], real32_T *zEstimated, real32_T Pxy[18],
    real32_T *Sy, real32_T dHdx[18], real32_T *Rsqrt)
{
    int32_T c_i;
    int32_T i;
    int32_T knt;
    real32_T beta1;
    real32_T t2;
    real32_T t6;

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    /* BaroMeasJacSym */
    /*     H = BaroMeasJacSym(IN1) */
    /*     This function was generated by the Symbolic Math Toolbox version 24.2. */
    /*     10-Apr-2026 23:03:41 */
    t2 = 1.0F / x[15];
    beta1 = -(t2 * x[7] * 0.0065F) + 1.0F;
    t6 = rt_powf_snf(beta1, 4.25588F);
    dHdx[0] = 0.0F;
    dHdx[1] = 0.0F;
    dHdx[2] = 0.0F;
    dHdx[3] = 0.0F;
    dHdx[4] = 0.0F;
    dHdx[5] = 0.0F;
    dHdx[6] = 0.0F;
    dHdx[7] = t2 * t6 * x[14] * -0.0341632217F;
    dHdx[8] = 0.0F;
    dHdx[9] = 0.0F;
    dHdx[10] = 0.0F;
    dHdx[11] = 0.0F;
    dHdx[12] = 0.0F;
    dHdx[13] = 0.0F;
    dHdx[14] = rt_powf_snf(beta1, 5.25588F);
    dHdx[15] = t2 * t2 * t6 * x[7] * x[14] * 0.0341632217F;
    dHdx[16] = 0.0F;
    dHdx[17] = 0.0F;
    *Rsqrt = Rs;

    /*  g0 / (R * L0) */
    *zEstimated = rt_powf_snf(1.0F - 0.0065F * x[7] / x[15], 5.25588F) * x[14];
    for (i = 0; i < 18; i++) {
        Pxy[i] = 0.0F;
        for (c_i = 0; c_i < 18; c_i++) {
            t2 = 0.0F;
            for (knt = 0; knt < 18; knt++) {
                t2 += S[18 * knt + i] * S[18 * knt + c_i];
            }

            Pxy[i] += t2 * dHdx[c_i];
        }

        PlaneNavigationFilter_B.y_b[i] = 0.0F;
    }

    for (i = 0; i < 18; i++) {
        for (c_i = 0; c_i < 18; c_i++) {
            PlaneNavigationFilter_B.y_b[c_i] += S[c_i * 18 + i] * dHdx[i];
        }
    }

    memcpy(&PlaneNavigationFilter_B.M[0], &PlaneNavigationFilter_B.y_b[0], 18U *
           sizeof(real32_T));

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    PlaneNavigationFilter_B.M[18] = Rs;
    for (i = 0; i < 1; i++) {
        t2 = PlaneNavigationFilter_B.M[0];
        beta1 = PlaneNavigationFilter_xnrm2_dk0c(18, PlaneNavigationFilter_B.M,
            2);
        if (beta1 != 0.0F) {
            beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.M[0], beta1);
            if (PlaneNavigationFilter_B.M[0] >= 0.0F) {
                beta1 = -beta1;
            }

            if (fabsf(beta1) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    for (c_i = 2; c_i < 20; c_i++) {
                        PlaneNavigationFilter_B.M[c_i - 1] *= 1.01412048E+31F;
                    }

                    beta1 *= 1.01412048E+31F;
                    t2 *= 1.01412048E+31F;
                } while ((fabsf(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

                beta1 = rt_hypotf_snf(t2, PlaneNavigationFilter_xnrm2_dk0c(18,
                                       PlaneNavigationFilter_B.M, 2));
                if (t2 >= 0.0F) {
                    beta1 = -beta1;
                }

                t2 = 1.0F / (t2 - beta1);
                for (c_i = 2; c_i < 20; c_i++) {
                    PlaneNavigationFilter_B.M[c_i - 1] *= t2;
                }

                for (c_i = 0; c_i <= knt; c_i++) {
                    beta1 *= 9.86076132E-32F;
                }

                t2 = beta1;
            } else {
                t2 = 1.0F / (PlaneNavigationFilter_B.M[0] - beta1);
                for (c_i = 2; c_i < 20; c_i++) {
                    PlaneNavigationFilter_B.M[c_i - 1] *= t2;
                }

                t2 = beta1;
            }
        }

        PlaneNavigationFilter_B.M[0] = t2;
    }

    *Sy = PlaneNavigationFilter_B.M[0];
}

static void PlaneNavigationFilter_trisolve_dk0c(real32_T A, real32_T B[18])
{
    int32_T b_j;

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    for (b_j = 0; b_j < 18; b_j++) {
        real32_T B_0;
        B_0 = B[b_j];
        if (B_0 != 0.0F) {
            B[b_j] = B_0 / A;
        }
    }

    /* End of Start for MATLABSystem: '<S8>/MATLAB System' */
}

static real32_T PlaneNavigationFilter_xnrm2_dk0ct(int32_T n, const real32_T x
    [342], int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S8>/MATLAB System' */
    return y;
}

static void PlaneNavigationFilter_qr_dk0c(const real32_T A[342], real32_T Q[342],
    real32_T R[324])
{
    int32_T b;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    memset(&PlaneNavigationFilter_B.b_tau_d[0], 0, 18U * sizeof(real32_T));
    memcpy(&Q[0], &A[0], 342U * sizeof(real32_T));
    memset(&PlaneNavigationFilter_B.work_d[0], 0, 18U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.iaii_c = 0; PlaneNavigationFilter_B.iaii_c < 18;
         PlaneNavigationFilter_B.iaii_c++) {
        PlaneNavigationFilter_B.ii_m = PlaneNavigationFilter_B.iaii_c * 19 +
            PlaneNavigationFilter_B.iaii_c;
        i = PlaneNavigationFilter_B.ii_m + 2;
        PlaneNavigationFilter_B.atmp_j = Q[PlaneNavigationFilter_B.ii_m];
        PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c] = 0.0F;
        PlaneNavigationFilter_B.beta1_f = PlaneNavigationFilter_xnrm2_dk0ct(18 -
            PlaneNavigationFilter_B.iaii_c, Q, PlaneNavigationFilter_B.ii_m + 2);
        if (PlaneNavigationFilter_B.beta1_f != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii_m];
            PlaneNavigationFilter_B.beta1_f = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1_f);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1_f =
                    -PlaneNavigationFilter_B.beta1_f;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1_f) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b = PlaneNavigationFilter_B.ii_m -
                        PlaneNavigationFilter_B.iaii_c;
                    for (lastv = i; lastv <= b + 19; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1_f *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp_j *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1_f) <
                                9.86076132E-32F) && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1_f = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp_j,
                     PlaneNavigationFilter_xnrm2_dk0ct(18 -
                      PlaneNavigationFilter_B.iaii_c, Q,
                      PlaneNavigationFilter_B.ii_m + 2));
                if (PlaneNavigationFilter_B.atmp_j >= 0.0F) {
                    PlaneNavigationFilter_B.beta1_f =
                        -PlaneNavigationFilter_B.beta1_f;
                }

                PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c] =
                    (PlaneNavigationFilter_B.beta1_f -
                     PlaneNavigationFilter_B.atmp_j) /
                    PlaneNavigationFilter_B.beta1_f;
                PlaneNavigationFilter_B.atmp_j = 1.0F /
                    (PlaneNavigationFilter_B.atmp_j -
                     PlaneNavigationFilter_B.beta1_f);
                for (lastv = i; lastv <= b + 19; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_j;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1_f *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp_j = PlaneNavigationFilter_B.beta1_f;
            } else {
                PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c] =
                    (PlaneNavigationFilter_B.beta1_f - c) /
                    PlaneNavigationFilter_B.beta1_f;
                PlaneNavigationFilter_B.atmp_j = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1_f);
                b = PlaneNavigationFilter_B.ii_m -
                    PlaneNavigationFilter_B.iaii_c;
                for (lastv = i; lastv <= b + 19; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_j;
                }

                PlaneNavigationFilter_B.atmp_j = PlaneNavigationFilter_B.beta1_f;
            }
        }

        Q[PlaneNavigationFilter_B.ii_m] = PlaneNavigationFilter_B.atmp_j;
        if (PlaneNavigationFilter_B.iaii_c + 1 < 18) {
            Q[PlaneNavigationFilter_B.ii_m] = 1.0F;
            knt = PlaneNavigationFilter_B.ii_m + 20;
            if (PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c]
                    != 0.0F) {
                lastv = 19 - PlaneNavigationFilter_B.iaii_c;
                i = PlaneNavigationFilter_B.ii_m -
                    PlaneNavigationFilter_B.iaii_c;
                while ((lastv > 0) && (Q[i + 18] == 0.0F)) {
                    lastv--;
                    i--;
                }

                b = 17 - PlaneNavigationFilter_B.iaii_c;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 19 + PlaneNavigationFilter_B.ii_m;
                    jA = lastc + 20;
                    do {
                        exitg1 = 0;
                        if (jA <= (lastc + lastv) + 19) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_d[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (19 * lastc + PlaneNavigationFilter_B.ii_m) + 20;
                    for (iac = knt; iac <= b; iac += 19) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii_m + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.ii_m) -
                                               20, 19);
                        PlaneNavigationFilter_B.work_d[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii_m;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_d[iac];
                        if (c != 0.0F) {
                            c *=
                                -PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_c];
                            b = jA + 20;
                            knt = (lastv + jA) + 19;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_m + d)
                                               - jA) - 20] * c;
                            }
                        }

                        jA += 19;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii_m] = PlaneNavigationFilter_B.atmp_j;
        }
    }

    for (i = 0; i < 18; i++) {
        for (PlaneNavigationFilter_B.iaii_c = 0; PlaneNavigationFilter_B.iaii_c <=
             i; PlaneNavigationFilter_B.iaii_c++) {
            /* Start for MATLABSystem: '<S8>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii_c + 18 * i] = Q[19 * i +
                PlaneNavigationFilter_B.iaii_c];
        }

        for (PlaneNavigationFilter_B.iaii_c = i + 2;
                PlaneNavigationFilter_B.iaii_c < 19;
                PlaneNavigationFilter_B.iaii_c++) {
            /* Start for MATLABSystem: '<S8>/MATLAB System' */
            R[(PlaneNavigationFilter_B.iaii_c + 18 * i) - 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S8>/MATLAB System' */
        PlaneNavigationFilter_B.work_d[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S8>/MATLAB System' */
    for (i = 17; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii_c = (i * 19 + i) + 19;
        if (i + 1 < 18) {
            Q[PlaneNavigationFilter_B.iaii_c - 19] = 1.0F;
            knt = PlaneNavigationFilter_B.iaii_c + 1;
            if (PlaneNavigationFilter_B.b_tau_d[i] != 0.0F) {
                lastv = 19 - i;
                PlaneNavigationFilter_B.ii_m = (PlaneNavigationFilter_B.iaii_c -
                    i) - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_m] == 0.0F))
                {
                    lastv--;
                    PlaneNavigationFilter_B.ii_m--;
                }

                b = 17 - i;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 19 + PlaneNavigationFilter_B.iaii_c;
                    jA = lastc + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= lastc + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work_d[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (19 * lastc + PlaneNavigationFilter_B.iaii_c) + 1;
                    for (iac = knt; iac <= b; iac += 19) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii_c + jA) - iac)
                                - 19] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii_c)
                                               - 1, 19);
                        PlaneNavigationFilter_B.work_d[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau_d[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii_c;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work_d[iac];
                        if (c != 0.0F) {
                            c *= -PlaneNavigationFilter_B.b_tau_d[i];
                            b = jA + 1;
                            knt = lastv + jA;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_c +
                                                d) - jA) - 20] * c;
                            }
                        }

                        jA += 19;
                    }
                }
            }
        }

        b = PlaneNavigationFilter_B.iaii_c - i;
        for (lastv = PlaneNavigationFilter_B.iaii_c - 17; lastv <= b; lastv++) {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_d[i];
        }

        Q[PlaneNavigationFilter_B.iaii_c - 19] = 1.0F -
            PlaneNavigationFilter_B.b_tau_d[i];
        for (PlaneNavigationFilter_B.ii_m = 0; PlaneNavigationFilter_B.ii_m < i;
             PlaneNavigationFilter_B.ii_m++) {
            Q[(PlaneNavigationFilter_B.iaii_c - PlaneNavigationFilter_B.ii_m) -
                20] = 0.0F;
        }
    }
}

static real32_T PlaneNavigationFilter_xnrm2_dk0ctr(int32_T n, const real32_T x
    [864], int32_T ix0)
{
    int32_T k;
    real32_T y;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    y = 0.0F;
    if (n >= 1) {
        if (n == 1) {
            y = fabsf(x[ix0 - 1]);
        } else {
            int32_T kend;
            real32_T scale;
            scale = 1.29246971E-26F;
            kend = ix0 + n;
            for (k = ix0; k < kend; k++) {
                real32_T absxk;
                absxk = fabsf(x[k - 1]);
                if (absxk > scale) {
                    real32_T t;
                    t = scale / absxk;
                    y = y * t * t + 1.0F;
                    scale = absxk;
                } else {
                    real32_T t;
                    t = absxk / scale;
                    y += t * t;
                }
            }

            y = scale * sqrtf(y);
        }
    }

    /* End of Start for MATLABSystem: '<S10>/MATLAB System' */
    return y;
}

static void PlaneNavigationFilter_qr_dk0ct(const real32_T A[864], real32_T Q[864],
    real32_T R[324])
{
    int32_T b;
    int32_T d;
    int32_T exitg1;
    int32_T i;
    int32_T iac;
    int32_T jA;
    int32_T knt;
    int32_T lastc;
    int32_T lastv;
    real32_T c;
    boolean_T exitg2;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    memset(&PlaneNavigationFilter_B.b_tau[0], 0, 18U * sizeof(real32_T));
    memcpy(&Q[0], &A[0], 864U * sizeof(real32_T));
    memset(&PlaneNavigationFilter_B.work[0], 0, 18U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.iaii = 0; PlaneNavigationFilter_B.iaii < 18;
            PlaneNavigationFilter_B.iaii++) {
        PlaneNavigationFilter_B.ii = PlaneNavigationFilter_B.iaii * 48 +
            PlaneNavigationFilter_B.iaii;
        i = PlaneNavigationFilter_B.ii + 2;
        PlaneNavigationFilter_B.atmp = Q[PlaneNavigationFilter_B.ii];
        PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] = 0.0F;
        PlaneNavigationFilter_B.beta1 = PlaneNavigationFilter_xnrm2_dk0ctr(47 -
            PlaneNavigationFilter_B.iaii, Q, PlaneNavigationFilter_B.ii + 2);
        if (PlaneNavigationFilter_B.beta1 != 0.0F) {
            c = Q[PlaneNavigationFilter_B.ii];
            PlaneNavigationFilter_B.beta1 = rt_hypotf_snf(c,
                PlaneNavigationFilter_B.beta1);
            if (c >= 0.0F) {
                PlaneNavigationFilter_B.beta1 = -PlaneNavigationFilter_B.beta1;
            }

            if (fabsf(PlaneNavigationFilter_B.beta1) < 9.86076132E-32F) {
                knt = -1;
                do {
                    knt++;
                    b = PlaneNavigationFilter_B.ii -
                        PlaneNavigationFilter_B.iaii;
                    for (lastv = i; lastv <= b + 48; lastv++) {
                        Q[lastv - 1] *= 1.01412048E+31F;
                    }

                    PlaneNavigationFilter_B.beta1 *= 1.01412048E+31F;
                    PlaneNavigationFilter_B.atmp *= 1.01412048E+31F;
                } while ((fabsf(PlaneNavigationFilter_B.beta1) < 9.86076132E-32F)
                         && (knt + 1 < 20));

                PlaneNavigationFilter_B.beta1 = rt_hypotf_snf
                    (PlaneNavigationFilter_B.atmp,
                     PlaneNavigationFilter_xnrm2_dk0ctr(47 -
                      PlaneNavigationFilter_B.iaii, Q,
                      PlaneNavigationFilter_B.ii + 2));
                if (PlaneNavigationFilter_B.atmp >= 0.0F) {
                    PlaneNavigationFilter_B.beta1 =
                        -PlaneNavigationFilter_B.beta1;
                }

                PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] =
                    (PlaneNavigationFilter_B.beta1 -
                     PlaneNavigationFilter_B.atmp) /
                    PlaneNavigationFilter_B.beta1;
                PlaneNavigationFilter_B.atmp = 1.0F /
                    (PlaneNavigationFilter_B.atmp -
                     PlaneNavigationFilter_B.beta1);
                for (lastv = i; lastv <= b + 48; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp;
                }

                for (lastv = 0; lastv <= knt; lastv++) {
                    PlaneNavigationFilter_B.beta1 *= 9.86076132E-32F;
                }

                PlaneNavigationFilter_B.atmp = PlaneNavigationFilter_B.beta1;
            } else {
                PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] =
                    (PlaneNavigationFilter_B.beta1 - c) /
                    PlaneNavigationFilter_B.beta1;
                PlaneNavigationFilter_B.atmp = 1.0F / (c -
                    PlaneNavigationFilter_B.beta1);
                b = PlaneNavigationFilter_B.ii - PlaneNavigationFilter_B.iaii;
                for (lastv = i; lastv <= b + 48; lastv++) {
                    Q[lastv - 1] *= PlaneNavigationFilter_B.atmp;
                }

                PlaneNavigationFilter_B.atmp = PlaneNavigationFilter_B.beta1;
            }
        }

        Q[PlaneNavigationFilter_B.ii] = PlaneNavigationFilter_B.atmp;
        if (PlaneNavigationFilter_B.iaii + 1 < 18) {
            Q[PlaneNavigationFilter_B.ii] = 1.0F;
            knt = PlaneNavigationFilter_B.ii + 49;
            if (PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] !=
                    0.0F) {
                lastv = 48 - PlaneNavigationFilter_B.iaii;
                i = PlaneNavigationFilter_B.ii - PlaneNavigationFilter_B.iaii;
                while ((lastv > 0) && (Q[i + 47] == 0.0F)) {
                    lastv--;
                    i--;
                }

                b = 17 - PlaneNavigationFilter_B.iaii;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 48 + PlaneNavigationFilter_B.ii;
                    jA = lastc + 49;
                    do {
                        exitg1 = 0;
                        if (jA <= (lastc + lastv) + 48) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (48 * lastc + PlaneNavigationFilter_B.ii) + 49;
                    for (iac = knt; iac <= b; iac += 48) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[(PlaneNavigationFilter_B.ii + jA) - iac] *
                                Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii)
                                               - 49, 48);
                        PlaneNavigationFilter_B.work[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii]
                      == 0.0F)) {
                    jA = PlaneNavigationFilter_B.ii;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work[iac];
                        if (c != 0.0F) {
                            c *=
                                -PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii];
                            b = jA + 49;
                            knt = (lastv + jA) + 48;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii + d)
                                               - jA) - 49] * c;
                            }
                        }

                        jA += 48;
                    }
                }
            }

            Q[PlaneNavigationFilter_B.ii] = PlaneNavigationFilter_B.atmp;
        }
    }

    for (i = 0; i < 18; i++) {
        for (PlaneNavigationFilter_B.iaii = 0; PlaneNavigationFilter_B.iaii <= i;
             PlaneNavigationFilter_B.iaii++) {
            /* Start for MATLABSystem: '<S10>/MATLAB System' */
            R[PlaneNavigationFilter_B.iaii + 18 * i] = Q[48 * i +
                PlaneNavigationFilter_B.iaii];
        }

        for (PlaneNavigationFilter_B.iaii = i + 2; PlaneNavigationFilter_B.iaii <
             19; PlaneNavigationFilter_B.iaii++) {
            /* Start for MATLABSystem: '<S10>/MATLAB System' */
            R[(PlaneNavigationFilter_B.iaii + 18 * i) - 1] = 0.0F;
        }

        /* Start for MATLABSystem: '<S10>/MATLAB System' */
        PlaneNavigationFilter_B.work[i] = 0.0F;
    }

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    for (i = 17; i >= 0; i--) {
        PlaneNavigationFilter_B.iaii = (i * 48 + i) + 48;
        if (i + 1 < 18) {
            Q[PlaneNavigationFilter_B.iaii - 48] = 1.0F;
            knt = PlaneNavigationFilter_B.iaii + 1;
            if (PlaneNavigationFilter_B.b_tau[i] != 0.0F) {
                lastv = 48 - i;
                PlaneNavigationFilter_B.ii = (PlaneNavigationFilter_B.iaii - i)
                    - 1;
                while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii] == 0.0F)) {
                    lastv--;
                    PlaneNavigationFilter_B.ii--;
                }

                b = 17 - i;
                exitg2 = false;
                while ((!exitg2) && (b > 0)) {
                    lastc = (b - 1) * 48 + PlaneNavigationFilter_B.iaii;
                    jA = lastc + 1;
                    do {
                        exitg1 = 0;
                        if (jA <= lastc + lastv) {
                            if (Q[jA - 1] != 0.0F) {
                                exitg1 = 1;
                            } else {
                                jA++;
                            }
                        } else {
                            b--;
                            exitg1 = 2;
                        }
                    } while (exitg1 == 0);

                    if (exitg1 == 1) {
                        exitg2 = true;
                    }
                }

                lastc = b - 1;
            } else {
                lastv = 0;
                lastc = -1;
            }

            if (lastv > 0) {
                if (lastc + 1 != 0) {
                    if (lastc >= 0) {
                        memset(&PlaneNavigationFilter_B.work[0], 0, (uint32_T)
                               (lastc + 1) * sizeof(real32_T));
                    }

                    b = (48 * lastc + PlaneNavigationFilter_B.iaii) + 1;
                    for (iac = knt; iac <= b; iac += 48) {
                        c = 0.0F;
                        d = iac + lastv;
                        for (jA = iac; jA < d; jA++) {
                            c += Q[((PlaneNavigationFilter_B.iaii + jA) - iac) -
                                48] * Q[jA - 1];
                        }

                        jA = div_nde_s32_floor((iac -
                                                PlaneNavigationFilter_B.iaii) -
                                               1, 48);
                        PlaneNavigationFilter_B.work[jA] += c;
                    }
                }

                if (!(-PlaneNavigationFilter_B.b_tau[i] == 0.0F)) {
                    jA = PlaneNavigationFilter_B.iaii;
                    for (iac = 0; iac <= lastc; iac++) {
                        c = PlaneNavigationFilter_B.work[iac];
                        if (c != 0.0F) {
                            c *= -PlaneNavigationFilter_B.b_tau[i];
                            b = jA + 1;
                            knt = lastv + jA;
                            for (d = b; d <= knt; d++) {
                                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii + d)
                                               - jA) - 49] * c;
                            }
                        }

                        jA += 48;
                    }
                }
            }
        }

        b = PlaneNavigationFilter_B.iaii - i;
        for (lastv = PlaneNavigationFilter_B.iaii - 46; lastv <= b; lastv++) {
            Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau[i];
        }

        Q[PlaneNavigationFilter_B.iaii - 48] = 1.0F -
            PlaneNavigationFilter_B.b_tau[i];
        for (PlaneNavigationFilter_B.ii = 0; PlaneNavigationFilter_B.ii < i;
                PlaneNavigationFilter_B.ii++) {
            Q[(PlaneNavigationFilter_B.iaii - PlaneNavigationFilter_B.ii) - 49] =
                0.0F;
        }
    }
}

static void PlaneNavigation_nav_state_trans_dt_100Hz(const real32_T state[18],
    const real32_T w[30], const real32_T u[6], real32_T x_next[18])
{
    int32_T i;
    real32_T R_ib[2];
    real32_T cp_tmp;
    real32_T cr_tmp;
    real32_T cy;
    real32_T omega_corr_idx_1;
    real32_T omega_corr_idx_2;
    real32_T sp_tmp;
    real32_T sr_tmp;
    real32_T state_0;
    real32_T state_1;
    real32_T state_2;
    real32_T sy;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    /*  h = state(8); */
    /*  p0 = state(15); */
    /*  T0 = state(16); */
    /*  xy_local = state(17:18); */
    /*  Rotation matrix for intrinsic XYZ Euler angles: R = Rx(roll)*Ry(pitch)*Rz(yaw) */
    cr_tmp = cosf(state[3]);
    sr_tmp = sinf(state[3]);
    cp_tmp = cosf(state[4]);
    sp_tmp = sinf(state[4]);
    cy = cosf(state[5]);
    sy = sinf(state[5]);
    PlaneNavigationFilter_B.R_ib[0] = cp_tmp * cy;
    PlaneNavigationFilter_B.R_ib[3] = -cp_tmp * sy;
    PlaneNavigationFilter_B.R_ib[6] = sp_tmp;
    PlaneNavigationFilter_B.R_ib[1] = sr_tmp * sp_tmp * cy + cr_tmp * sy;
    PlaneNavigationFilter_B.R_ib[4] = -sr_tmp * sp_tmp * sy + cr_tmp * cy;
    PlaneNavigationFilter_B.R_ib[7] = -sr_tmp * cp_tmp;
    PlaneNavigationFilter_B.R_ib[2] = -cr_tmp * sp_tmp * cy + sr_tmp * sy;
    PlaneNavigationFilter_B.R_ib[5] = cr_tmp * sp_tmp * sy + sr_tmp * cy;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    cy = cr_tmp * cp_tmp;
    PlaneNavigationFilter_B.R_ib[8] = cy;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    sy = (u[3] + w[21]) - state[11];
    omega_corr_idx_1 = (u[4] + w[22]) - state[12];
    omega_corr_idx_2 = (u[5] + w[23]) - state[13];

    /*  Horizontal position in NED inertial frame: xy_dot = R_ib(1:2,:) * v_body */
    /*  x = North, y = East. Yaw = 0 when body x-axis points North (NED convention). */
    /*  The initial yaw is unknown and estimated by the filter. */
    /*  Altitude is already tracked by h (state 8); z is therefore redundant. */
    PlaneNavigationFilter_B.fv3[0] = -0.0F;
    PlaneNavigationFilter_B.fv3[3] = omega_corr_idx_2;
    PlaneNavigationFilter_B.fv3[6] = -omega_corr_idx_1;
    PlaneNavigationFilter_B.fv3[1] = -omega_corr_idx_2;
    PlaneNavigationFilter_B.fv3[4] = -0.0F;
    PlaneNavigationFilter_B.fv3[7] = sy;
    PlaneNavigationFilter_B.fv3[2] = omega_corr_idx_1;
    PlaneNavigationFilter_B.fv3[5] = -sy;
    PlaneNavigationFilter_B.fv3[8] = -0.0F;
    state_0 = state[1];
    state_1 = state[0];
    state_2 = state[2];
    for (i = 0; i < 3; i++) {
        PlaneNavigationFilter_B.fv4[i] = (PlaneNavigationFilter_B.fv3[i + 3] *
            state_0 + PlaneNavigationFilter_B.fv3[i] * state_1) +
            PlaneNavigationFilter_B.fv3[i + 6] * state_2;
    }

    state_0 = state[1];
    state_1 = state[0];
    state_2 = state[2];
    for (i = 0; i < 2; i++) {
        R_ib[i] = ((PlaneNavigationFilter_B.R_ib[i + 3] * state_0 +
                    PlaneNavigationFilter_B.R_ib[i] * state_1) +
                   PlaneNavigationFilter_B.R_ib[i + 6] * state_2) + w[i + 16];
    }

    state_0 = omega_corr_idx_1 * sr_tmp + omega_corr_idx_2 * cr_tmp;
    x_next[3] = ((state_0 * tanf(state[4]) + sy) + w[3]) * 0.01F + state[3];
    x_next[4] = ((omega_corr_idx_1 * cr_tmp - omega_corr_idx_2 * sr_tmp) + w[4])
        * 0.01F + state[4];
    x_next[5] = (1.0F / cp_tmp * state_0 + w[5]) * 0.01F + state[5];
    x_next[6] = w[6] * 0.01F + state[6];
    x_next[7] = (-((state[0] * PlaneNavigationFilter_B.R_ib[2] + state[1] *
                    PlaneNavigationFilter_B.R_ib[5]) + state[2] * cy) + w[7]) *
        0.01F + state[7];
    x_next[0] = (((((state[6] * -sp_tmp + PlaneNavigationFilter_B.fv4[0]) + u[0])
                   + w[18]) - state[8]) + w[0]) * 0.01F + state[0];
    x_next[8] = (w[8] + w[24]) * 0.01F + state[8];
    x_next[11] = (w[11] + w[27]) * 0.01F + state[11];
    x_next[1] = (((((sr_tmp * cp_tmp * state[6] + PlaneNavigationFilter_B.fv4[1])
                    + u[1]) + w[19]) - state[9]) + w[1]) * 0.01F + state[1];
    x_next[9] = (w[9] + w[25]) * 0.01F + state[9];
    x_next[12] = (w[12] + w[28]) * 0.01F + state[12];
    x_next[2] = (((((cy * state[6] + PlaneNavigationFilter_B.fv4[2]) + u[2]) +
                   w[20]) - state[10]) + w[2]) * 0.01F + state[2];
    x_next[10] = (w[10] + w[26]) * 0.01F + state[10];
    x_next[13] = (w[13] + w[29]) * 0.01F + state[13];
    x_next[14] = w[14] * 0.01F + state[14];
    x_next[15] = w[15] * 0.01F + state[15];
    x_next[16] = R_ib[0] * 0.01F + state[16];
    x_next[17] = R_ib[1] * 0.01F + state[17];
}

/* Model step function */
void PlaneNavigationFilter_step(void)
{
    int8_T A;
    int8_T A_0;
    boolean_T ekf_good;
    boolean_T rtb_INIT_MEAS_EN;
    boolean_T tmp;
    static const int8_T A_1[162] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 };

    static const int8_T A_2[36] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 };

    static const int8_T tmp_0[162] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0 };

    static const int8_T tmp_1[36] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

    static const real32_T tmp_2[164] = { 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F };

    static const real32_T tmp_3[160] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F };

    static const real32_T tmp_4[47] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
        0.01F, 0.0F, 0.0F, 0.0F, 0.0F };

    boolean_T exitg1;

    /* Outputs for Enabled SubSystem: '<S2>/Correct1' incorporates:
     *  EnablePort: '<S6>/Enable'
     */
    /* Outputs for Atomic SubSystem: '<S2>/Output' */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        for (PlaneNavigationFilter_B.coffset = 0;
                PlaneNavigationFilter_B.coffset < 18;
                PlaneNavigationFilter_B.coffset++) {
            /* MATLAB Function: '<S9>/MATLAB Function' incorporates:
             *  DataStoreRead: '<S9>/Data Store Read1'
             *  MATLABSystem: '<S6>/MATLAB System'
             * */
            PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
                PlaneNavigationFilter_B.i] = PlaneNavigationFilter_DW.P[18 *
                PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
        }
    }

    /* End of Outputs for SubSystem: '<S2>/Correct1' */

    /* MATLAB Function: '<S9>/MATLAB Function' incorporates:
     *  DataStoreRead: '<S9>/Data Store Read1'
     */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        for (PlaneNavigationFilter_B.coffset = 0;
                PlaneNavigationFilter_B.coffset < 18;
                PlaneNavigationFilter_B.coffset++) {
            PlaneNavigationFilter_B.t8 = 0.0F;
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
                    PlaneNavigationFilter_B.b_k +
                    PlaneNavigationFilter_B.coffset] *
                    PlaneNavigationFilter_B.A[18 * PlaneNavigationFilter_B.i +
                    PlaneNavigationFilter_B.b_k];
            }

            PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.coffset + 18 *
                PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
        }
    }

    /* End of Outputs for SubSystem: '<S2>/Output' */

    /* BusCreator generated from: '<Root>/EkfState' incorporates:
     *  DataStoreRead: '<S9>/Data Store Read'
     *  Outport: '<Root>/EkfState'
     */
    PlaneNavigationFilter_Y.EkfState.g = PlaneNavigationFilter_DW.x[6];
    PlaneNavigationFilter_Y.EkfState.h = PlaneNavigationFilter_DW.x[7];
    PlaneNavigationFilter_Y.EkfState.vel_body[0] = PlaneNavigationFilter_DW.x[0];
    PlaneNavigationFilter_Y.EkfState.rpy[0] = PlaneNavigationFilter_DW.x[3];
    PlaneNavigationFilter_Y.EkfState.acc_bias[0] = PlaneNavigationFilter_DW.x[8];
    PlaneNavigationFilter_Y.EkfState.gyro_bias[0] = PlaneNavigationFilter_DW.x
        [11];
    PlaneNavigationFilter_Y.EkfState.vel_body[1] = PlaneNavigationFilter_DW.x[1];
    PlaneNavigationFilter_Y.EkfState.rpy[1] = PlaneNavigationFilter_DW.x[4];
    PlaneNavigationFilter_Y.EkfState.acc_bias[1] = PlaneNavigationFilter_DW.x[9];
    PlaneNavigationFilter_Y.EkfState.gyro_bias[1] = PlaneNavigationFilter_DW.x
        [12];
    PlaneNavigationFilter_Y.EkfState.vel_body[2] = PlaneNavigationFilter_DW.x[2];
    PlaneNavigationFilter_Y.EkfState.rpy[2] = PlaneNavigationFilter_DW.x[5];
    PlaneNavigationFilter_Y.EkfState.acc_bias[2] = PlaneNavigationFilter_DW.x[10];
    PlaneNavigationFilter_Y.EkfState.gyro_bias[2] = PlaneNavigationFilter_DW.x
        [13];
    PlaneNavigationFilter_Y.EkfState.P0 = PlaneNavigationFilter_DW.x[14];
    PlaneNavigationFilter_Y.EkfState.T0 = PlaneNavigationFilter_DW.x[15];
    PlaneNavigationFilter_Y.EkfState.xy[0] = PlaneNavigationFilter_DW.x[16];
    PlaneNavigationFilter_Y.EkfState.xy[1] = PlaneNavigationFilter_DW.x[17];
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        /* Sqrt: '<Root>/Sqrt' incorporates:
         *  S-Function (sdspdiag2): '<Root>/Extract Diagonal'
         */
        PlaneNavigationFilter_B.ekf_state_std[PlaneNavigationFilter_B.i] = sqrtf
            (PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.i * 19]);
    }

    /* BusCreator generated from: '<Root>/EkfStateStd' incorporates:
     *  Outport: '<Root>/EkfStateStd'
     */
    PlaneNavigationFilter_Y.EkfStateStd.g =
        PlaneNavigationFilter_B.ekf_state_std[6];
    PlaneNavigationFilter_Y.EkfStateStd.h =
        PlaneNavigationFilter_B.ekf_state_std[7];
    PlaneNavigationFilter_Y.EkfStateStd.vel_body[0] =
        PlaneNavigationFilter_B.ekf_state_std[0];
    PlaneNavigationFilter_Y.EkfStateStd.rpy[0] =
        PlaneNavigationFilter_B.ekf_state_std[3];
    PlaneNavigationFilter_Y.EkfStateStd.acc_bias[0] =
        PlaneNavigationFilter_B.ekf_state_std[8];
    PlaneNavigationFilter_Y.EkfStateStd.gyro_bias[0] =
        PlaneNavigationFilter_B.ekf_state_std[11];
    PlaneNavigationFilter_Y.EkfStateStd.vel_body[1] =
        PlaneNavigationFilter_B.ekf_state_std[1];
    PlaneNavigationFilter_Y.EkfStateStd.rpy[1] =
        PlaneNavigationFilter_B.ekf_state_std[4];
    PlaneNavigationFilter_Y.EkfStateStd.acc_bias[1] =
        PlaneNavigationFilter_B.ekf_state_std[9];
    PlaneNavigationFilter_Y.EkfStateStd.gyro_bias[1] =
        PlaneNavigationFilter_B.ekf_state_std[12];
    PlaneNavigationFilter_Y.EkfStateStd.vel_body[2] =
        PlaneNavigationFilter_B.ekf_state_std[2];
    PlaneNavigationFilter_Y.EkfStateStd.rpy[2] =
        PlaneNavigationFilter_B.ekf_state_std[5];
    PlaneNavigationFilter_Y.EkfStateStd.acc_bias[2] =
        PlaneNavigationFilter_B.ekf_state_std[10];
    PlaneNavigationFilter_Y.EkfStateStd.gyro_bias[2] =
        PlaneNavigationFilter_B.ekf_state_std[13];
    PlaneNavigationFilter_Y.EkfStateStd.P0 =
        PlaneNavigationFilter_B.ekf_state_std[14];
    PlaneNavigationFilter_Y.EkfStateStd.T0 =
        PlaneNavigationFilter_B.ekf_state_std[15];
    PlaneNavigationFilter_Y.EkfStateStd.xy[0] =
        PlaneNavigationFilter_B.ekf_state_std[16];
    PlaneNavigationFilter_Y.EkfStateStd.xy[1] =
        PlaneNavigationFilter_B.ekf_state_std[17];

    /* Chart: '<Root>/EKF Logic' incorporates:
     *  Inport: '<Root>/ARM_CMD'
     */
    if (PlaneNavigationFilter_DW.is_active_c3_PlaneNavigationFilter == 0) {
        PlaneNavigationFilter_DW.is_active_c3_PlaneNavigationFilter = 1U;
        ekf_good = true;
        PlaneNavigationFilter_B.b_k = 0;
        exitg1 = false;
        while ((!exitg1) && (PlaneNavigationFilter_B.b_k < 18)) {
            if (!(PlaneNavigationFilter_DW.Delay_DSTATE[PlaneNavigationFilter_B.b_k]
                  <=
                    PlaneNavigationFilter_P.ekf_std_init_thresholds[PlaneNavigationFilter_B.b_k]))
            {
                ekf_good = false;
                exitg1 = true;
            } else {
                PlaneNavigationFilter_B.b_k++;
            }
        }

        if (!ekf_good) {
            PlaneNavigationFilter_DW.durationCounter_2 = 0U;
            PlaneNavigationFilter_DW.durationCounter_1 = 0U;
        }

        PlaneNavigationFilter_DW.is_EKF_LOGIC = PlaneNavigationFilter_IN_IDLE;
        rtb_INIT_MEAS_EN = false;

        /* Outport: '<Root>/EKF_STAGE' */
        PlaneNavigationFilter_Y.EKF_STAGE = IDLE;
    } else {
        ekf_good = true;
        PlaneNavigationFilter_B.i = 0;
        exitg1 = false;
        while ((!exitg1) && (PlaneNavigationFilter_B.i < 18)) {
            if (!(PlaneNavigationFilter_DW.Delay_DSTATE[PlaneNavigationFilter_B.i]
                  <=
                    PlaneNavigationFilter_P.ekf_std_init_thresholds[PlaneNavigationFilter_B.i]))
            {
                ekf_good = false;
                exitg1 = true;
            } else {
                PlaneNavigationFilter_B.i++;
            }
        }

        tmp = !ekf_good;
        if (tmp) {
            PlaneNavigationFilter_DW.durationCounter_2 = 0U;
            PlaneNavigationFilter_DW.durationCounter_1 = 0U;
        }

        switch (PlaneNavigationFilter_DW.is_EKF_LOGIC) {
          case PlaneNavigationFilter_IN_EKF_ACTIVE:
            rtb_INIT_MEAS_EN = false;
            if (tmp) {
                PlaneNavigationFilter_DW.durationCounter_2 = 0U;
            }

            if (PlaneNavigationFilter_DW.durationCounter_2 > 100U) {
                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = NOMINAL;
            } else {
                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = DEGRADED;
            }
            break;

          case PlaneNavigationFilter_IN_EKF_INIT:
            if (tmp) {
                PlaneNavigationFilter_DW.durationCounter_1 = 0U;
            }

            if (PlaneNavigationFilter_DW.durationCounter_1 > 100U) {
                PlaneNavigationFilter_DW.durationCounter_2 = 0U;
                PlaneNavigationFilter_DW.is_EKF_LOGIC =
                    PlaneNavigationFilter_IN_EKF_ACTIVE;
                rtb_INIT_MEAS_EN = false;

                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = DEGRADED;
            } else {
                rtb_INIT_MEAS_EN = true;

                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = INIT;
            }
            break;

          default:
            /* case IN_IDLE: */
            if (PlaneNavigationFilter_U.ARM_CMD) {
                PlaneNavigationFilter_DW.durationCounter_1 = 0U;
                PlaneNavigationFilter_DW.is_EKF_LOGIC =
                    PlaneNavigationFilter_IN_EKF_INIT;
                rtb_INIT_MEAS_EN = true;

                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = INIT;
            } else {
                rtb_INIT_MEAS_EN = false;

                /* Outport: '<Root>/EKF_STAGE' */
                PlaneNavigationFilter_Y.EKF_STAGE = IDLE;
            }
            break;
        }
    }

    if (ekf_good) {
        PlaneNavigationFilter_DW.durationCounter_2++;
        PlaneNavigationFilter_DW.durationCounter_1++;
    } else {
        PlaneNavigationFilter_DW.durationCounter_2 = 0U;
        PlaneNavigationFilter_DW.durationCounter_1 = 0U;
    }

    /* End of Chart: '<Root>/EKF Logic' */

    /* Outputs for Enabled SubSystem: '<S2>/Correct1' incorporates:
     *  EnablePort: '<S6>/Enable'
     */
    /* Logic: '<Root>/AND' incorporates:
     *  Inport: '<Root>/HAS_BARO_DATA'
     */
    if (rtb_INIT_MEAS_EN && PlaneNavigationFilter_U.HAS_BARO_DATA) {
        /* MATLABSystem: '<S6>/MATLAB System' incorporates:
         *  Constant: '<S2>/BlockOrdering'
         */
        PlaneNavigationFilter_B.MATLABSystem_o3_f =
            PlaneNavigationFilter_P.BlockOrdering_Value;

        /* MATLABSystem: '<S6>/MATLAB System' incorporates:
         *  DataStoreRead: '<S6>/Data Store ReadP'
         */
        /*  vx, vy, vz */
        /*  rpy */
        /*  h */
        /*  T0 */
        /*  P0 */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
            memset(&PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.coffset
                   + 1], 0, 18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.bkj_m = A_1[PlaneNavigationFilter_B.b_k *
                    9 + PlaneNavigationFilter_B.i];
                for (PlaneNavigationFilter_B.c_i = 0;
                        PlaneNavigationFilter_B.c_i < 18;
                        PlaneNavigationFilter_B.c_i++) {
                    PlaneNavigationFilter_B.K_tmp =
                        (PlaneNavigationFilter_B.coffset +
                         PlaneNavigationFilter_B.c_i) + 1;
                    PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.K_tmp] +=
                        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i *
                        18 + PlaneNavigationFilter_B.b_k] * (real32_T)
                        PlaneNavigationFilter_B.bkj_m;
                }
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
             *  Constant: '<S2>/R1'
             * */
            memcpy(&PlaneNavigationFilter_B.K[PlaneNavigationFilter_B.i * 27],
                   &PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i * 18],
                   18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 9;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.K[(PlaneNavigationFilter_B.coffset + 27 *
                    PlaneNavigationFilter_B.i) + 18] =
                    PlaneNavigationFilter_P.R1_Value[9 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }
        }

        /* MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_qr(PlaneNavigationFilter_B.K,
            PlaneNavigationFilter_B.a__1_me, PlaneNavigationFilter_B.R);
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 9;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.Sy[PlaneNavigationFilter_B.coffset + 9 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.R[9 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
                 *  DataStoreRead: '<S6>/Data Store ReadP'
                 */
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 18;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
                        PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset] *
                        PlaneNavigationFilter_B.A[18 * PlaneNavigationFilter_B.i
                        + PlaneNavigationFilter_B.b_k];
                }

                PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                /* Start for MATLABSystem: '<S6>/MATLAB System' */
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 18;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.P[18 *
                        PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset] * (real32_T)tmp_0[18 *
                        PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.b_k];
                }

                PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i + 9 *
                    PlaneNavigationFilter_B.coffset] =
                    PlaneNavigationFilter_B.t8;
            }
        }

        /* MATLABSystem: '<S6>/MATLAB System' */
        memcpy(&PlaneNavigationFilter_B.C[0], &PlaneNavigationFilter_B.K_n[0],
               162U * sizeof(real32_T));

        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_trisolve(PlaneNavigationFilter_B.Sy,
            PlaneNavigationFilter_B.C);
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 9;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.R[PlaneNavigationFilter_B.coffset + 9 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.Sy[9 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }
        }

        PlaneNavigationFilter_trisolve_d(PlaneNavigationFilter_B.R,
            PlaneNavigationFilter_B.C);

        /* MATLABSystem: '<S6>/MATLAB System' */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.coffset + 18
                    * PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.C[9 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 162;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            PlaneNavigationFilter_B.C[PlaneNavigationFilter_B.i] =
                -PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i];
        }

        /* MATLABSystem: '<S6>/MATLAB System' incorporates:
         *  DataStoreRead: '<S6>/Data Store ReadP'
         */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 9;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.C[18 *
                        PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset] * (real32_T)A_1[9 *
                        PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.b_k];
                }

                PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
            }
        }

        for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k < 18;
                PlaneNavigationFilter_B.b_k++) {
            PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_k +
                PlaneNavigationFilter_B.b_k;
            PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
            memset(&PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.coffset
                   + 1], 0, 18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.bkj =
                    PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_k * 18 +
                    PlaneNavigationFilter_B.i];
                for (PlaneNavigationFilter_B.c_i = 0;
                        PlaneNavigationFilter_B.c_i < 18;
                        PlaneNavigationFilter_B.c_i++) {
                    PlaneNavigationFilter_B.bkj_m =
                        (PlaneNavigationFilter_B.coffset +
                         PlaneNavigationFilter_B.c_i) + 1;
                    PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.bkj_m] +=
                        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i *
                        18 + PlaneNavigationFilter_B.b_k] *
                        PlaneNavigationFilter_B.bkj;
                }
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 9;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
                 *  Constant: '<S2>/R1'
                 * */
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 9;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.K_n[18
                        * PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset] *
                        PlaneNavigationFilter_P.R1_Value[9 *
                        PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.b_k];
                }

                PlaneNavigationFilter_B.C[PlaneNavigationFilter_B.i + 9 *
                    PlaneNavigationFilter_B.coffset] =
                    PlaneNavigationFilter_B.t8;
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S6>/MATLAB System' */
            memcpy(&PlaneNavigationFilter_B.y_c[PlaneNavigationFilter_B.i * 27],
                   &PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.i * 18],
                   18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 9;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.y_c[(PlaneNavigationFilter_B.coffset +
                    27 * PlaneNavigationFilter_B.i) + 18] =
                    PlaneNavigationFilter_B.C[9 * PlaneNavigationFilter_B.i +
                    PlaneNavigationFilter_B.coffset];
            }
        }

        /* MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_qr_d(PlaneNavigationFilter_B.y_c,
            PlaneNavigationFilter_B.a__1_m, PlaneNavigationFilter_B.A);

        /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
         *  Constant: '<S3>/zero h'
         *  Constant: '<S3>/zero vel'
         *  DataStoreRead: '<S6>/Data Store ReadX'
         *  Inport: '<Root>/init_input'
         *  SignalConversion generated from: '<S6>/MATLAB System'
         */
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[0] =
            PlaneNavigationFilter_P.zerovel_Value[0] -
            PlaneNavigationFilter_DW.x[0];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[3] =
            PlaneNavigationFilter_U.init_input.rpy_init[0] -
            PlaneNavigationFilter_DW.x[3];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[1] =
            PlaneNavigationFilter_P.zerovel_Value[1] -
            PlaneNavigationFilter_DW.x[1];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[4] =
            PlaneNavigationFilter_U.init_input.rpy_init[1] -
            PlaneNavigationFilter_DW.x[4];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[2] =
            PlaneNavigationFilter_P.zerovel_Value[2] -
            PlaneNavigationFilter_DW.x[2];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[5] =
            PlaneNavigationFilter_U.init_input.rpy_init[2] -
            PlaneNavigationFilter_DW.x[5];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[6] =
            PlaneNavigationFilter_P.zeroh_Value - PlaneNavigationFilter_DW.x[7];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[7] =
            PlaneNavigationFilter_U.init_input.T0 - PlaneNavigationFilter_DW.x
            [15];
        PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[8] =
            PlaneNavigationFilter_U.init_input.P0 - PlaneNavigationFilter_DW.x
            [14];
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* DataStoreWrite: '<S6>/Data Store WriteP' incorporates:
             *  MATLABSystem: '<S6>/MATLAB System'
             * */
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.A[18 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }

            /* End of DataStoreWrite: '<S6>/Data Store WriteP' */

            /* DataStoreWrite: '<S6>/Data Store WriteX' incorporates:
             *  DataStoreRead: '<S6>/Data Store ReadX'
             *  MATLABSystem: '<S6>/MATLAB System'
             * */
            PlaneNavigationFilter_B.t8 = 0.0F;
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 9;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.K_n[18 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i]
                    * PlaneNavigationFilter_B.rtb_TmpSignalConversionAtMATLAB[PlaneNavigationFilter_B.coffset];
            }

            PlaneNavigationFilter_DW.x[PlaneNavigationFilter_B.i] +=
                PlaneNavigationFilter_B.t8;

            /* End of DataStoreWrite: '<S6>/Data Store WriteX' */
        }
    }

    /* End of Logic: '<Root>/AND' */
    /* End of Outputs for SubSystem: '<S2>/Correct1' */

    /* Outputs for Enabled SubSystem: '<S2>/Correct2' incorporates:
     *  EnablePort: '<S7>/Enable'
     */
    /* Inport: '<Root>/GPS_POS_EN' */
    if (PlaneNavigationFilter_U.GPS_POS_EN) {
        /* MATLABSystem: '<S7>/MATLAB System' incorporates:
         *  DataStoreRead: '<S7>/Data Store ReadP'
         */
        /*  GPS measurement function: returns [north_m; east_m] in the local NED frame. */
        /*  */
        /*    state(17:18) is already the North/East displacement in metres relative */
        /*    to the reference origin.  Conversion from raw lat/lon to metres happens */
        /*    outside the EKF (preprocessing block) using latlon2xy. */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
            memset(&PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset
                   + 1], 0, 18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.bkj_m = A_2[(PlaneNavigationFilter_B.b_k
                    << 1) + PlaneNavigationFilter_B.i];
                for (PlaneNavigationFilter_B.c_i = 0;
                        PlaneNavigationFilter_B.c_i < 18;
                        PlaneNavigationFilter_B.c_i++) {
                    PlaneNavigationFilter_B.K_tmp =
                        (PlaneNavigationFilter_B.coffset +
                         PlaneNavigationFilter_B.c_i) + 1;
                    PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.K_tmp] +=
                        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i *
                        18 + PlaneNavigationFilter_B.b_k] * (real32_T)
                        PlaneNavigationFilter_B.bkj_m;
                }
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' incorporates:
             *  Constant: '<S2>/R2'
             * */
            memcpy(&PlaneNavigationFilter_B.K_l[PlaneNavigationFilter_B.i * 20],
                   &PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i * 18],
                   18U * sizeof(real32_T));
            PlaneNavigationFilter_B.K_l[20 * PlaneNavigationFilter_B.i + 18] =
                PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.i];
            PlaneNavigationFilter_B.K_l[20 * PlaneNavigationFilter_B.i + 19] =
                PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.i + 2];
        }

        /* MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_qr_dk(PlaneNavigationFilter_B.K_l,
            PlaneNavigationFilter_B.a__1_p, PlaneNavigationFilter_B.R_d);
        PlaneNavigationFilter_B.Sy_b[0] = PlaneNavigationFilter_B.R_d[0];
        PlaneNavigationFilter_B.Sy_b[1] = PlaneNavigationFilter_B.R_d[2];
        PlaneNavigationFilter_B.Sy_b[2] = PlaneNavigationFilter_B.R_d[1];
        PlaneNavigationFilter_B.Sy_b[3] = PlaneNavigationFilter_B.R_d[3];
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                /* Start for MATLABSystem: '<S7>/MATLAB System' incorporates:
                 *  DataStoreRead: '<S7>/Data Store ReadP'
                 */
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 18;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
                        PlaneNavigationFilter_B.b_k + PlaneNavigationFilter_B.i]
                        * PlaneNavigationFilter_DW.P[18 *
                        PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset];
                }

                PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.i + 18 *
                    PlaneNavigationFilter_B.coffset] =
                    PlaneNavigationFilter_B.t8;
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                /* Start for MATLABSystem: '<S7>/MATLAB System' */
                PlaneNavigationFilter_B.t8 = 0.0F;
                for (PlaneNavigationFilter_B.b_k = 0;
                        PlaneNavigationFilter_B.b_k < 18;
                        PlaneNavigationFilter_B.b_k++) {
                    PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.P[18 *
                        PlaneNavigationFilter_B.b_k +
                        PlaneNavigationFilter_B.coffset] * (real32_T)tmp_1[18 *
                        PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.b_k];
                }

                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i +
                    (PlaneNavigationFilter_B.coffset << 1)] =
                    PlaneNavigationFilter_B.t8;
            }
        }

        /* MATLABSystem: '<S7>/MATLAB System' */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
            PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset] =
                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset];
            PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset + 1] =
                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 1];
        }

        /* Start for MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_trisolve_dk(PlaneNavigationFilter_B.Sy_b,
            PlaneNavigationFilter_B.C_d);

        /* MATLABSystem: '<S7>/MATLAB System' */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
            PlaneNavigationFilter_B.C_g[PlaneNavigationFilter_B.coffset] =
                PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset];
            PlaneNavigationFilter_B.C_g[PlaneNavigationFilter_B.coffset + 1] =
                PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset + 1];
        }

        /* Start for MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_B.Sy_b[0] = PlaneNavigationFilter_B.R_d[0];
        PlaneNavigationFilter_B.Sy_b[1] = PlaneNavigationFilter_B.R_d[1];
        PlaneNavigationFilter_B.Sy_b[2] = PlaneNavigationFilter_B.R_d[2];
        PlaneNavigationFilter_B.Sy_b[3] = PlaneNavigationFilter_B.R_d[3];
        PlaneNavigationFilter_trisolve_dk0(PlaneNavigationFilter_B.Sy_b,
            PlaneNavigationFilter_B.C_g);

        /* MATLABSystem: '<S7>/MATLAB System' */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 18
                    * PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.C_g
                    [(PlaneNavigationFilter_B.coffset << 1) +
                    PlaneNavigationFilter_B.i];
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 36;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.i] =
                -PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i];
        }

        /* MATLABSystem: '<S7>/MATLAB System' incorporates:
         *  DataStoreRead: '<S7>/Data Store ReadP'
         */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
            A = A_2[PlaneNavigationFilter_B.coffset + 1];
            A_0 = A_2[PlaneNavigationFilter_B.coffset];
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] =
                    PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset
                    + 18] * (real32_T)A + (real32_T)A_0 *
                    PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset];
            }
        }

        for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k < 18;
                PlaneNavigationFilter_B.b_k++) {
            PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_k +
                PlaneNavigationFilter_B.b_k;
            PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
            memset(&PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.coffset
                   + 1], 0, 18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.bkj =
                    PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_k * 18 +
                    PlaneNavigationFilter_B.i];
                for (PlaneNavigationFilter_B.c_i = 0;
                        PlaneNavigationFilter_B.c_i < 18;
                        PlaneNavigationFilter_B.c_i++) {
                    PlaneNavigationFilter_B.bkj_m =
                        (PlaneNavigationFilter_B.coffset +
                         PlaneNavigationFilter_B.c_i) + 1;
                    PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.bkj_m] +=
                        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i *
                        18 + PlaneNavigationFilter_B.b_k] *
                        PlaneNavigationFilter_B.bkj;
                }
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' incorporates:
             *  Constant: '<S2>/R2'
             * */
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
            PlaneNavigationFilter_B.t8 =
                PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.coffset
                + 1];
            PlaneNavigationFilter_B.t25 =
                PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.coffset];
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.i +
                    (PlaneNavigationFilter_B.coffset << 1)] =
                    PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset
                    + 18] * PlaneNavigationFilter_B.t8 +
                    PlaneNavigationFilter_B.t25 *
                    PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset];
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S7>/MATLAB System' */
            memcpy(&PlaneNavigationFilter_B.y_cx[PlaneNavigationFilter_B.i * 20],
                   &PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.i * 18],
                   18U * sizeof(real32_T));
            PlaneNavigationFilter_B.bkj_m = PlaneNavigationFilter_B.i << 1;
            PlaneNavigationFilter_B.y_cx[20 * PlaneNavigationFilter_B.i + 18] =
                PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.bkj_m];
            PlaneNavigationFilter_B.y_cx[20 * PlaneNavigationFilter_B.i + 19] =
                PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.bkj_m + 1];
        }

        /* MATLABSystem: '<S7>/MATLAB System' */
        PlaneNavigationFilter_qr_dk0(PlaneNavigationFilter_B.y_cx,
            PlaneNavigationFilter_B.a__1_k, PlaneNavigationFilter_B.A);

        /* Start for MATLABSystem: '<S7>/MATLAB System' incorporates:
         *  DataStoreRead: '<S7>/Data Store ReadX'
         *  Inport: '<Root>/xy_pos'
         */
        PlaneNavigationFilter_B.t8 = PlaneNavigationFilter_U.xy_pos[0] -
            PlaneNavigationFilter_DW.x[16];
        PlaneNavigationFilter_B.t25 = PlaneNavigationFilter_U.xy_pos[1] -
            PlaneNavigationFilter_DW.x[17];
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* DataStoreWrite: '<S7>/Data Store WriteP' incorporates:
             *  MATLABSystem: '<S7>/MATLAB System'
             * */
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.A[18 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }

            /* End of DataStoreWrite: '<S7>/Data Store WriteP' */

            /* DataStoreWrite: '<S7>/Data Store WriteX' incorporates:
             *  DataStoreRead: '<S7>/Data Store ReadX'
             *  MATLABSystem: '<S7>/MATLAB System'
             * */
            PlaneNavigationFilter_DW.x[PlaneNavigationFilter_B.i] +=
                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i + 18] *
                PlaneNavigationFilter_B.t25 +
                PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i] *
                PlaneNavigationFilter_B.t8;
        }
    }

    /* End of Inport: '<Root>/GPS_POS_EN' */
    /* End of Outputs for SubSystem: '<S2>/Correct2' */

    /* Outputs for Enabled SubSystem: '<S2>/Correct3' incorporates:
     *  EnablePort: '<S8>/Enable'
     */
    /* Logic: '<Root>/AND1' incorporates:
     *  Inport: '<Root>/HAS_BARO_DATA'
     *  Logic: '<Root>/NOT'
     */
    if (PlaneNavigationFilter_U.HAS_BARO_DATA && (!rtb_INIT_MEAS_EN)) {
        /* MATLABSystem: '<S8>/MATLAB System' incorporates:
         *  Constant: '<S2>/R3'
         *  DataStoreRead: '<S8>/Data Store ReadP'
         *  DataStoreRead: '<S8>/Data Store ReadX'
         *  Inport: '<Root>/baro_pressure'
         */
        EKFCorrectorAdditive_getMeasurementJacob
            (PlaneNavigationFilter_P.R3_Value, PlaneNavigationFilter_DW.x,
             PlaneNavigationFilter_DW.P, &PlaneNavigationFilter_B.residue,
             PlaneNavigationFilter_B.Pxy, &PlaneNavigationFilter_B.t25,
             PlaneNavigationFilter_B.dHdx, &PlaneNavigationFilter_B.t8);
        PlaneNavigationFilter_B.residue = PlaneNavigationFilter_U.baro_pressure
            - PlaneNavigationFilter_B.residue;
        memcpy(&PlaneNavigationFilter_B.C_l[0], &PlaneNavigationFilter_B.Pxy[0],
               18U * sizeof(real32_T));

        /* Start for MATLABSystem: '<S8>/MATLAB System' */
        PlaneNavigationFilter_trisolve_dk0c(PlaneNavigationFilter_B.t25,
            PlaneNavigationFilter_B.C_l);

        /* MATLABSystem: '<S8>/MATLAB System' */
        memcpy(&PlaneNavigationFilter_B.Pxy[0], &PlaneNavigationFilter_B.C_l[0],
               18U * sizeof(real32_T));

        /* Start for MATLABSystem: '<S8>/MATLAB System' */
        PlaneNavigationFilter_trisolve_dk0c(PlaneNavigationFilter_B.t25,
            PlaneNavigationFilter_B.Pxy);
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.C_l[PlaneNavigationFilter_B.i] =
                -PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i];
        }

        /* MATLABSystem: '<S8>/MATLAB System' incorporates:
         *  DataStoreRead: '<S8>/Data Store ReadP'
         */
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] =
                    PlaneNavigationFilter_B.C_l[PlaneNavigationFilter_B.coffset]
                    * PlaneNavigationFilter_B.dHdx[PlaneNavigationFilter_B.i];
            }
        }

        for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k < 18;
                PlaneNavigationFilter_B.b_k++) {
            PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_k +
                PlaneNavigationFilter_B.b_k;
            PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
            memset(&PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.coffset
                   + 1], 0, 18U * sizeof(real32_T));
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    18; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.bkj =
                    PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_k * 18 +
                    PlaneNavigationFilter_B.i];
                for (PlaneNavigationFilter_B.c_i = 0;
                        PlaneNavigationFilter_B.c_i < 18;
                        PlaneNavigationFilter_B.c_i++) {
                    PlaneNavigationFilter_B.bkj_m =
                        (PlaneNavigationFilter_B.coffset +
                         PlaneNavigationFilter_B.c_i) + 1;
                    PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.bkj_m] +=
                        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i *
                        18 + PlaneNavigationFilter_B.b_k] *
                        PlaneNavigationFilter_B.bkj;
                }
            }
        }

        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* Start for MATLABSystem: '<S8>/MATLAB System' */
            memcpy(&PlaneNavigationFilter_B.y_p[PlaneNavigationFilter_B.i * 19],
                   &PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.i * 18],
                   18U * sizeof(real32_T));
            PlaneNavigationFilter_B.y_p[19 * PlaneNavigationFilter_B.i + 18] =
                PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i] *
                PlaneNavigationFilter_B.t8;
        }

        /* MATLABSystem: '<S8>/MATLAB System' */
        PlaneNavigationFilter_qr_dk0c(PlaneNavigationFilter_B.y_p,
            PlaneNavigationFilter_B.a__1_b, PlaneNavigationFilter_B.A);
        for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
                PlaneNavigationFilter_B.i++) {
            /* DataStoreWrite: '<S8>/Data Store WriteP' incorporates:
             *  MATLABSystem: '<S8>/MATLAB System'
             * */
            for (PlaneNavigationFilter_B.coffset = 0;
                    PlaneNavigationFilter_B.coffset < 18;
                    PlaneNavigationFilter_B.coffset++) {
                PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
                    PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.A[18 *
                    PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
            }

            /* End of DataStoreWrite: '<S8>/Data Store WriteP' */

            /* DataStoreWrite: '<S8>/Data Store WriteX' incorporates:
             *  DataStoreRead: '<S8>/Data Store ReadX'
             *  MATLABSystem: '<S8>/MATLAB System'
             * */
            PlaneNavigationFilter_DW.x[PlaneNavigationFilter_B.i] +=
                PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i] *
                PlaneNavigationFilter_B.residue;
        }
    }

    /* End of Logic: '<Root>/AND1' */
    /* End of Outputs for SubSystem: '<S2>/Correct3' */

    /* Outputs for Atomic SubSystem: '<S2>/Predict' */
    /* SignalConversion generated from: '<S10>/MATLAB System' incorporates:
     *  Inport: '<Root>/acc_b'
     *  Inport: '<Root>/omega_ib'
     */
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[0] =
        PlaneNavigationFilter_U.acc_b[0];
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[3] =
        PlaneNavigationFilter_U.omega_ib[0];
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[1] =
        PlaneNavigationFilter_U.acc_b[1];
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[4] =
        PlaneNavigationFilter_U.omega_ib[1];
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[2] =
        PlaneNavigationFilter_U.acc_b[2];
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp[5] =
        PlaneNavigationFilter_U.omega_ib[2];

    /* MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreRead: '<S10>/Data Store ReadX'
     *  Inport: '<Root>/omega_ib'
     *  SignalConversion generated from: '<S10>/MATLAB System'
     */
    /* StateTrans100HzJacSym */
    /*     [Jx,Jw] = StateTrans100HzJacSym(IN1,IN2,IN3) */
    /*     This function was generated by the Symbolic Math Toolbox version 24.2. */
    /*     10-Apr-2026 23:03:41 */
    PlaneNavigationFilter_B.bkj = cosf(PlaneNavigationFilter_DW.x[3]);
    PlaneNavigationFilter_B.t3 = cosf(PlaneNavigationFilter_DW.x[4]);
    PlaneNavigationFilter_B.t4 = cosf(PlaneNavigationFilter_DW.x[5]);
    PlaneNavigationFilter_B.t5 = sinf(PlaneNavigationFilter_DW.x[3]);
    PlaneNavigationFilter_B.t6 = sinf(PlaneNavigationFilter_DW.x[4]);
    PlaneNavigationFilter_B.t7 = sinf(PlaneNavigationFilter_DW.x[5]);
    PlaneNavigationFilter_B.t8 = tanf(PlaneNavigationFilter_DW.x[4]);
    PlaneNavigationFilter_B.t22 = PlaneNavigationFilter_U.omega_ib[0] / 100.0F;
    PlaneNavigationFilter_B.t23 = PlaneNavigationFilter_U.omega_ib[1] / 100.0F;
    PlaneNavigationFilter_B.t24 = PlaneNavigationFilter_U.omega_ib[2] / 100.0F;
    PlaneNavigationFilter_B.t25 = PlaneNavigationFilter_DW.x[0] / 100.0F;
    PlaneNavigationFilter_B.residue = PlaneNavigationFilter_DW.x[1] / 100.0F;
    PlaneNavigationFilter_B.t27 = PlaneNavigationFilter_DW.x[2] / 100.0F;
    PlaneNavigationFilter_B.t28 = PlaneNavigationFilter_DW.x[11] / 100.0F;
    PlaneNavigationFilter_B.t29 = PlaneNavigationFilter_DW.x[12] / 100.0F;
    PlaneNavigationFilter_B.t30 = PlaneNavigationFilter_DW.x[13] / 100.0F;
    PlaneNavigationFilter_B.t11 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t4;
    PlaneNavigationFilter_B.t12 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t7;
    PlaneNavigationFilter_B.t13 = PlaneNavigationFilter_B.t4 *
        PlaneNavigationFilter_B.t5;
    PlaneNavigationFilter_B.t14 = PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t7;
    PlaneNavigationFilter_B.t15 = 1.0F / PlaneNavigationFilter_B.t3;
    PlaneNavigationFilter_B.t16 = -PlaneNavigationFilter_DW.x[12] +
        PlaneNavigationFilter_U.omega_ib[1];
    PlaneNavigationFilter_B.t17 = -PlaneNavigationFilter_DW.x[13] +
        PlaneNavigationFilter_U.omega_ib[2];
    PlaneNavigationFilter_B.t31 = PlaneNavigationFilter_B.bkj / 100.0F;
    PlaneNavigationFilter_B.t32 = PlaneNavigationFilter_B.t5 / 100.0F;
    PlaneNavigationFilter_B.t33 = PlaneNavigationFilter_B.t6 / 100.0F;
    PlaneNavigationFilter_B.t18 = PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_B.t11;
    PlaneNavigationFilter_B.t19 = PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_B.t12;
    PlaneNavigationFilter_B.t20 = PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_B.t13;
    PlaneNavigationFilter_B.t21 = PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_B.t14;
    PlaneNavigationFilter_B.t40 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t17;
    PlaneNavigationFilter_B.t41 = PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t16;
    PlaneNavigationFilter_B.t43 = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t31;
    PlaneNavigationFilter_B.t50 = PlaneNavigationFilter_B.t12 +
        PlaneNavigationFilter_B.t20;
    PlaneNavigationFilter_B.t51 = PlaneNavigationFilter_B.t13 +
        PlaneNavigationFilter_B.t19;
    PlaneNavigationFilter_B.t54 = PlaneNavigationFilter_B.t40 +
        PlaneNavigationFilter_B.t41;
    PlaneNavigationFilter_B.t52 = PlaneNavigationFilter_B.t11 -
        PlaneNavigationFilter_B.t21;
    PlaneNavigationFilter_B.t53 = PlaneNavigationFilter_B.t14 -
        PlaneNavigationFilter_B.t18;
    PlaneNavigationFilter_B.t16 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t16 - PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t17;
    PlaneNavigationFilter_B.y_cv[0] = 1.0F;
    PlaneNavigationFilter_B.y_cv[1] = -PlaneNavigationFilter_B.t24 +
        PlaneNavigationFilter_B.t30;
    PlaneNavigationFilter_B.y_cv[2] = PlaneNavigationFilter_B.t23 -
        PlaneNavigationFilter_B.t29;
    PlaneNavigationFilter_B.y_cv[3] = 0.0F;
    PlaneNavigationFilter_B.y_cv[4] = 0.0F;
    PlaneNavigationFilter_B.y_cv[5] = 0.0F;
    PlaneNavigationFilter_B.y_cv[6] = 0.0F;
    PlaneNavigationFilter_B.y_cv[7] = PlaneNavigationFilter_B.t14 * -0.01F +
        PlaneNavigationFilter_B.t18 / 100.0F;
    PlaneNavigationFilter_B.y_cv[8] = 0.0F;
    PlaneNavigationFilter_B.y_cv[9] = 0.0F;
    PlaneNavigationFilter_B.y_cv[10] = 0.0F;
    PlaneNavigationFilter_B.y_cv[11] = 0.0F;
    PlaneNavigationFilter_B.y_cv[12] = 0.0F;
    PlaneNavigationFilter_B.y_cv[13] = 0.0F;
    PlaneNavigationFilter_B.y_cv[14] = 0.0F;
    PlaneNavigationFilter_B.y_cv[15] = 0.0F;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t18 = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t4;

    /* MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.y_cv[16] = PlaneNavigationFilter_B.t18 / 100.0F;
    PlaneNavigationFilter_B.y_cv[17] = PlaneNavigationFilter_B.t12 / 100.0F +
        PlaneNavigationFilter_B.t20 / 100.0F;
    PlaneNavigationFilter_B.y_cv[18] = PlaneNavigationFilter_B.t24 -
        PlaneNavigationFilter_B.t30;
    PlaneNavigationFilter_B.y_cv[19] = 1.0F;
    PlaneNavigationFilter_B.y_cv[20] = -PlaneNavigationFilter_B.t22 +
        PlaneNavigationFilter_B.t28;
    PlaneNavigationFilter_B.y_cv[21] = 0.0F;
    PlaneNavigationFilter_B.y_cv[22] = 0.0F;
    PlaneNavigationFilter_B.y_cv[23] = 0.0F;
    PlaneNavigationFilter_B.y_cv[24] = 0.0F;
    PlaneNavigationFilter_B.y_cv[25] = PlaneNavigationFilter_B.t13 * -0.01F -
        PlaneNavigationFilter_B.t19 / 100.0F;
    PlaneNavigationFilter_B.y_cv[26] = 0.0F;
    PlaneNavigationFilter_B.y_cv[27] = 0.0F;
    PlaneNavigationFilter_B.y_cv[28] = 0.0F;
    PlaneNavigationFilter_B.y_cv[29] = 0.0F;
    PlaneNavigationFilter_B.y_cv[30] = 0.0F;
    PlaneNavigationFilter_B.y_cv[31] = 0.0F;
    PlaneNavigationFilter_B.y_cv[32] = 0.0F;
    PlaneNavigationFilter_B.y_cv[33] = 0.0F;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t24 = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t7;

    /* MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.y_cv[34] = PlaneNavigationFilter_B.t24 * -0.01F;
    PlaneNavigationFilter_B.y_cv[35] = PlaneNavigationFilter_B.t11 / 100.0F -
        PlaneNavigationFilter_B.t21 / 100.0F;
    PlaneNavigationFilter_B.y_cv[36] = -PlaneNavigationFilter_B.t23 +
        PlaneNavigationFilter_B.t29;
    PlaneNavigationFilter_B.y_cv[37] = PlaneNavigationFilter_B.t22 -
        PlaneNavigationFilter_B.t28;
    PlaneNavigationFilter_B.y_cv[38] = 1.0F;
    PlaneNavigationFilter_B.y_cv[39] = 0.0F;
    PlaneNavigationFilter_B.y_cv[40] = 0.0F;
    PlaneNavigationFilter_B.y_cv[41] = 0.0F;
    PlaneNavigationFilter_B.y_cv[42] = 0.0F;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t22 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t3;

    /* MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.y_cv[43] = PlaneNavigationFilter_B.t22 * -0.01F;
    PlaneNavigationFilter_B.y_cv[44] = 0.0F;
    PlaneNavigationFilter_B.y_cv[45] = 0.0F;
    PlaneNavigationFilter_B.y_cv[46] = 0.0F;
    PlaneNavigationFilter_B.y_cv[47] = 0.0F;
    PlaneNavigationFilter_B.y_cv[48] = 0.0F;
    PlaneNavigationFilter_B.y_cv[49] = 0.0F;
    PlaneNavigationFilter_B.y_cv[50] = 0.0F;
    PlaneNavigationFilter_B.y_cv[51] = 0.0F;
    PlaneNavigationFilter_B.y_cv[52] = PlaneNavigationFilter_B.t33;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t23 = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t5;

    /* MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreRead: '<S10>/Data Store ReadX'
     */
    PlaneNavigationFilter_B.y_cv[53] = PlaneNavigationFilter_B.t23 * -0.01F;
    PlaneNavigationFilter_B.y_cv[54] = 0.0F;
    PlaneNavigationFilter_B.y_cv[55] = PlaneNavigationFilter_B.t43 *
        PlaneNavigationFilter_DW.x[6];
    PlaneNavigationFilter_B.y_cv[56] = PlaneNavigationFilter_B.t23 *
        PlaneNavigationFilter_DW.x[6] * -0.01F;
    PlaneNavigationFilter_B.y_cv[57] = PlaneNavigationFilter_B.t8 *
        PlaneNavigationFilter_B.t16 / 100.0F + 1.0F;
    PlaneNavigationFilter_B.y_cv[58] = PlaneNavigationFilter_B.t40 * -0.01F -
        PlaneNavigationFilter_B.t41 / 100.0F;
    PlaneNavigationFilter_B.y_cv[59] = PlaneNavigationFilter_B.t15 *
        PlaneNavigationFilter_B.t16 / 100.0F;
    PlaneNavigationFilter_B.y_cv[60] = 0.0F;
    PlaneNavigationFilter_B.y_cv[61] = (PlaneNavigationFilter_B.t50 *
        PlaneNavigationFilter_DW.x[0] * -0.01F - PlaneNavigationFilter_B.t52 *
        PlaneNavigationFilter_DW.x[1] / 100.0F) + PlaneNavigationFilter_B.t23 *
        PlaneNavigationFilter_B.t27;
    PlaneNavigationFilter_B.y_cv[62] = 0.0F;
    PlaneNavigationFilter_B.y_cv[63] = 0.0F;
    PlaneNavigationFilter_B.y_cv[64] = 0.0F;
    PlaneNavigationFilter_B.y_cv[65] = 0.0F;
    PlaneNavigationFilter_B.y_cv[66] = 0.0F;
    PlaneNavigationFilter_B.y_cv[67] = 0.0F;
    PlaneNavigationFilter_B.y_cv[68] = 0.0F;
    PlaneNavigationFilter_B.y_cv[69] = 0.0F;
    PlaneNavigationFilter_B.y_cv[70] = 0.0F;
    PlaneNavigationFilter_B.y_cv[71] = (PlaneNavigationFilter_B.t51 *
        PlaneNavigationFilter_DW.x[1] * -0.01F - PlaneNavigationFilter_B.t53 *
        PlaneNavigationFilter_DW.x[0] / 100.0F) - PlaneNavigationFilter_B.t22 *
        PlaneNavigationFilter_DW.x[2] / 100.0F;
    PlaneNavigationFilter_B.y_cv[72] = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_DW.x[6] * -0.01F;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t22 = PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t6;

    /* MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreRead: '<S10>/Data Store ReadX'
     */
    PlaneNavigationFilter_B.y_cv[73] = PlaneNavigationFilter_B.t22 *
        PlaneNavigationFilter_DW.x[6] * -0.01F;

    /* Start for MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_B.t23 = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t6;

    /* MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreRead: '<S10>/Data Store ReadP'
     *  DataStoreRead: '<S10>/Data Store ReadX'
     */
    PlaneNavigationFilter_B.y_cv[74] = PlaneNavigationFilter_B.t23 *
        PlaneNavigationFilter_DW.x[6] * -0.01F;
    PlaneNavigationFilter_B.y_cv[75] = (PlaneNavigationFilter_B.t8 *
        PlaneNavigationFilter_B.t8 + 1.0F) * PlaneNavigationFilter_B.t54 /
        100.0F;
    PlaneNavigationFilter_B.y_cv[76] = 1.0F;
    PlaneNavigationFilter_B.y_cv[77] = PlaneNavigationFilter_B.t15 *
        PlaneNavigationFilter_B.t15 * PlaneNavigationFilter_B.t33 *
        PlaneNavigationFilter_B.t54;
    PlaneNavigationFilter_B.y_cv[78] = 0.0F;
    PlaneNavigationFilter_B.y_cv[79] = (PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t11 * PlaneNavigationFilter_B.t25 +
        PlaneNavigationFilter_B.t23 * PlaneNavigationFilter_B.t27) -
        PlaneNavigationFilter_B.t3 * PlaneNavigationFilter_B.t12 *
        PlaneNavigationFilter_DW.x[1] / 100.0F;
    PlaneNavigationFilter_B.y_cv[80] = 0.0F;
    PlaneNavigationFilter_B.y_cv[81] = 0.0F;
    PlaneNavigationFilter_B.y_cv[82] = 0.0F;
    PlaneNavigationFilter_B.y_cv[83] = 0.0F;
    PlaneNavigationFilter_B.y_cv[84] = 0.0F;
    PlaneNavigationFilter_B.y_cv[85] = 0.0F;
    PlaneNavigationFilter_B.y_cv[86] = 0.0F;
    PlaneNavigationFilter_B.y_cv[87] = 0.0F;
    PlaneNavigationFilter_B.y_cv[88] = (PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_B.t7 * PlaneNavigationFilter_B.residue +
        PlaneNavigationFilter_B.t3 * PlaneNavigationFilter_B.t27) -
        PlaneNavigationFilter_B.t4 * PlaneNavigationFilter_B.t6 *
        PlaneNavigationFilter_DW.x[0] / 100.0F;
    PlaneNavigationFilter_B.y_cv[89] = (PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t13 * PlaneNavigationFilter_B.t25 +
        PlaneNavigationFilter_B.t22 * PlaneNavigationFilter_B.t27) -
        PlaneNavigationFilter_B.t3 * PlaneNavigationFilter_B.t14 *
        PlaneNavigationFilter_DW.x[1] / 100.0F;
    PlaneNavigationFilter_B.y_cv[90] = 0.0F;
    PlaneNavigationFilter_B.y_cv[91] = 0.0F;
    PlaneNavigationFilter_B.y_cv[92] = 0.0F;
    PlaneNavigationFilter_B.y_cv[93] = 0.0F;
    PlaneNavigationFilter_B.y_cv[94] = 0.0F;
    PlaneNavigationFilter_B.y_cv[95] = 1.0F;
    PlaneNavigationFilter_B.y_cv[96] = 0.0F;
    PlaneNavigationFilter_B.y_cv[97] = PlaneNavigationFilter_B.residue *
        PlaneNavigationFilter_B.t53 - PlaneNavigationFilter_B.t51 *
        PlaneNavigationFilter_DW.x[0] / 100.0F;
    PlaneNavigationFilter_B.y_cv[98] = 0.0F;
    PlaneNavigationFilter_B.y_cv[99] = 0.0F;
    PlaneNavigationFilter_B.y_cv[100] = 0.0F;
    PlaneNavigationFilter_B.y_cv[101] = 0.0F;
    PlaneNavigationFilter_B.y_cv[102] = 0.0F;
    PlaneNavigationFilter_B.y_cv[103] = 0.0F;
    PlaneNavigationFilter_B.y_cv[104] = 0.0F;
    PlaneNavigationFilter_B.y_cv[105] = 0.0F;
    PlaneNavigationFilter_B.y_cv[106] = PlaneNavigationFilter_B.t18 *
        PlaneNavigationFilter_DW.x[1] * -0.01F - PlaneNavigationFilter_B.t24 *
        PlaneNavigationFilter_DW.x[0] / 100.0F;
    PlaneNavigationFilter_B.y_cv[107] = PlaneNavigationFilter_B.t25 *
        PlaneNavigationFilter_B.t52 - PlaneNavigationFilter_B.t50 *
        PlaneNavigationFilter_DW.x[1] / 100.0F;
    PlaneNavigationFilter_B.y_cv[108] = -PlaneNavigationFilter_B.t33;
    PlaneNavigationFilter_B.y_cv[109] = PlaneNavigationFilter_B.t3 *
        PlaneNavigationFilter_B.t32;
    PlaneNavigationFilter_B.y_cv[110] = PlaneNavigationFilter_B.t43;
    PlaneNavigationFilter_B.y_cv[111] = 0.0F;
    PlaneNavigationFilter_B.y_cv[112] = 0.0F;
    PlaneNavigationFilter_B.y_cv[113] = 0.0F;
    PlaneNavigationFilter_B.y_cv[114] = 1.0F;
    memset(&PlaneNavigationFilter_B.y_cv[115], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.y_cv[133] = 1.0F;
    PlaneNavigationFilter_B.y_cv[134] = 0.0F;
    PlaneNavigationFilter_B.y_cv[135] = 0.0F;
    PlaneNavigationFilter_B.y_cv[136] = 0.0F;
    PlaneNavigationFilter_B.y_cv[137] = 0.0F;
    PlaneNavigationFilter_B.y_cv[138] = 0.0F;
    PlaneNavigationFilter_B.y_cv[139] = 0.0F;
    PlaneNavigationFilter_B.y_cv[140] = 0.0F;
    PlaneNavigationFilter_B.y_cv[141] = 0.0F;
    PlaneNavigationFilter_B.y_cv[142] = 0.0F;
    PlaneNavigationFilter_B.y_cv[143] = 0.0F;
    PlaneNavigationFilter_B.y_cv[144] = -0.01F;
    PlaneNavigationFilter_B.y_cv[145] = 0.0F;
    PlaneNavigationFilter_B.y_cv[146] = 0.0F;
    PlaneNavigationFilter_B.y_cv[147] = 0.0F;
    PlaneNavigationFilter_B.y_cv[148] = 0.0F;
    PlaneNavigationFilter_B.y_cv[149] = 0.0F;
    PlaneNavigationFilter_B.y_cv[150] = 0.0F;
    PlaneNavigationFilter_B.y_cv[151] = 0.0F;
    PlaneNavigationFilter_B.y_cv[152] = 1.0F;
    PlaneNavigationFilter_B.y_cv[153] = 0.0F;
    PlaneNavigationFilter_B.y_cv[154] = 0.0F;
    PlaneNavigationFilter_B.y_cv[155] = 0.0F;
    PlaneNavigationFilter_B.y_cv[156] = 0.0F;
    PlaneNavigationFilter_B.y_cv[157] = 0.0F;
    PlaneNavigationFilter_B.y_cv[158] = 0.0F;
    PlaneNavigationFilter_B.y_cv[159] = 0.0F;
    PlaneNavigationFilter_B.y_cv[160] = 0.0F;
    PlaneNavigationFilter_B.y_cv[161] = 0.0F;
    PlaneNavigationFilter_B.y_cv[162] = 0.0F;
    PlaneNavigationFilter_B.y_cv[163] = -0.01F;
    PlaneNavigationFilter_B.y_cv[164] = 0.0F;
    PlaneNavigationFilter_B.y_cv[165] = 0.0F;
    PlaneNavigationFilter_B.y_cv[166] = 0.0F;
    PlaneNavigationFilter_B.y_cv[167] = 0.0F;
    PlaneNavigationFilter_B.y_cv[168] = 0.0F;
    PlaneNavigationFilter_B.y_cv[169] = 0.0F;
    PlaneNavigationFilter_B.y_cv[170] = 0.0F;
    PlaneNavigationFilter_B.y_cv[171] = 1.0F;
    PlaneNavigationFilter_B.y_cv[172] = 0.0F;
    PlaneNavigationFilter_B.y_cv[173] = 0.0F;
    PlaneNavigationFilter_B.y_cv[174] = 0.0F;
    PlaneNavigationFilter_B.y_cv[175] = 0.0F;
    PlaneNavigationFilter_B.y_cv[176] = 0.0F;
    PlaneNavigationFilter_B.y_cv[177] = 0.0F;
    PlaneNavigationFilter_B.y_cv[178] = 0.0F;
    PlaneNavigationFilter_B.y_cv[179] = 0.0F;
    PlaneNavigationFilter_B.y_cv[180] = 0.0F;
    PlaneNavigationFilter_B.y_cv[181] = 0.0F;
    PlaneNavigationFilter_B.y_cv[182] = -0.01F;
    PlaneNavigationFilter_B.y_cv[183] = 0.0F;
    PlaneNavigationFilter_B.y_cv[184] = 0.0F;
    PlaneNavigationFilter_B.y_cv[185] = 0.0F;
    PlaneNavigationFilter_B.y_cv[186] = 0.0F;
    PlaneNavigationFilter_B.y_cv[187] = 0.0F;
    PlaneNavigationFilter_B.y_cv[188] = 0.0F;
    PlaneNavigationFilter_B.y_cv[189] = 0.0F;
    PlaneNavigationFilter_B.y_cv[190] = 1.0F;
    PlaneNavigationFilter_B.y_cv[191] = 0.0F;
    PlaneNavigationFilter_B.y_cv[192] = 0.0F;
    PlaneNavigationFilter_B.y_cv[193] = 0.0F;
    PlaneNavigationFilter_B.y_cv[194] = 0.0F;
    PlaneNavigationFilter_B.y_cv[195] = 0.0F;
    PlaneNavigationFilter_B.y_cv[196] = 0.0F;
    PlaneNavigationFilter_B.y_cv[197] = 0.0F;
    PlaneNavigationFilter_B.y_cv[198] = 0.0F;
    PlaneNavigationFilter_B.y_cv[199] = -PlaneNavigationFilter_B.t27;
    PlaneNavigationFilter_B.y_cv[200] = PlaneNavigationFilter_B.residue;
    PlaneNavigationFilter_B.y_cv[201] = -0.01F;
    PlaneNavigationFilter_B.y_cv[202] = 0.0F;
    PlaneNavigationFilter_B.y_cv[203] = 0.0F;
    PlaneNavigationFilter_B.y_cv[204] = 0.0F;
    PlaneNavigationFilter_B.y_cv[205] = 0.0F;
    PlaneNavigationFilter_B.y_cv[206] = 0.0F;
    PlaneNavigationFilter_B.y_cv[207] = 0.0F;
    PlaneNavigationFilter_B.y_cv[208] = 0.0F;
    PlaneNavigationFilter_B.y_cv[209] = 1.0F;
    PlaneNavigationFilter_B.y_cv[210] = 0.0F;
    PlaneNavigationFilter_B.y_cv[211] = 0.0F;
    PlaneNavigationFilter_B.y_cv[212] = 0.0F;
    PlaneNavigationFilter_B.y_cv[213] = 0.0F;
    PlaneNavigationFilter_B.y_cv[214] = 0.0F;
    PlaneNavigationFilter_B.y_cv[215] = 0.0F;
    PlaneNavigationFilter_B.y_cv[216] = PlaneNavigationFilter_B.t27;
    PlaneNavigationFilter_B.y_cv[217] = 0.0F;
    PlaneNavigationFilter_B.y_cv[218] = -PlaneNavigationFilter_B.t25;
    PlaneNavigationFilter_B.y_cv[219] = PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t8 * -0.01F;
    PlaneNavigationFilter_B.y_cv[220] = -PlaneNavigationFilter_B.t31;
    PlaneNavigationFilter_B.y_cv[221] = PlaneNavigationFilter_B.t5 *
        PlaneNavigationFilter_B.t15 * -0.01F;
    PlaneNavigationFilter_B.y_cv[222] = 0.0F;
    PlaneNavigationFilter_B.y_cv[223] = 0.0F;
    PlaneNavigationFilter_B.y_cv[224] = 0.0F;
    PlaneNavigationFilter_B.y_cv[225] = 0.0F;
    PlaneNavigationFilter_B.y_cv[226] = 0.0F;
    PlaneNavigationFilter_B.y_cv[227] = 0.0F;
    PlaneNavigationFilter_B.y_cv[228] = 1.0F;
    PlaneNavigationFilter_B.y_cv[229] = 0.0F;
    PlaneNavigationFilter_B.y_cv[230] = 0.0F;
    PlaneNavigationFilter_B.y_cv[231] = 0.0F;
    PlaneNavigationFilter_B.y_cv[232] = 0.0F;
    PlaneNavigationFilter_B.y_cv[233] = 0.0F;
    PlaneNavigationFilter_B.y_cv[234] = -PlaneNavigationFilter_B.residue;
    PlaneNavigationFilter_B.y_cv[235] = PlaneNavigationFilter_B.t25;
    PlaneNavigationFilter_B.y_cv[236] = 0.0F;
    PlaneNavigationFilter_B.y_cv[237] = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t8 * -0.01F;
    PlaneNavigationFilter_B.y_cv[238] = PlaneNavigationFilter_B.t32;
    PlaneNavigationFilter_B.y_cv[239] = PlaneNavigationFilter_B.bkj *
        PlaneNavigationFilter_B.t15 * -0.01F;
    PlaneNavigationFilter_B.y_cv[240] = 0.0F;
    PlaneNavigationFilter_B.y_cv[241] = 0.0F;
    PlaneNavigationFilter_B.y_cv[242] = 0.0F;
    PlaneNavigationFilter_B.y_cv[243] = 0.0F;
    PlaneNavigationFilter_B.y_cv[244] = 0.0F;
    PlaneNavigationFilter_B.y_cv[245] = 0.0F;
    PlaneNavigationFilter_B.y_cv[246] = 0.0F;
    PlaneNavigationFilter_B.y_cv[247] = 1.0F;
    memset(&PlaneNavigationFilter_B.y_cv[248], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.y_cv[266] = 1.0F;
    memset(&PlaneNavigationFilter_B.y_cv[267], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.y_cv[285] = 1.0F;
    memset(&PlaneNavigationFilter_B.y_cv[286], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.y_cv[304] = 1.0F;
    memset(&PlaneNavigationFilter_B.y_cv[305], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.y_cv[323] = 1.0F;
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
        memset(&PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.coffset + 1],
               0, 18U * sizeof(real32_T));
        for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k < 18;
                PlaneNavigationFilter_B.b_k++) {
            PlaneNavigationFilter_B.bkj =
                PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.b_k * 18 +
                PlaneNavigationFilter_B.i];
            for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i <
                    18; PlaneNavigationFilter_B.c_i++) {
                PlaneNavigationFilter_B.bkj_m = (PlaneNavigationFilter_B.coffset
                    + PlaneNavigationFilter_B.c_i) + 1;
                PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.bkj_m] +=
                    PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18
                    + PlaneNavigationFilter_B.b_k] * PlaneNavigationFilter_B.bkj;
            }
        }
    }

    /* Start for MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  Constant: '<S2>/Q'
     * */
    memcpy(&PlaneNavigationFilter_B.fv[0], &tmp_2[0], 164U * sizeof(real32_T));
    memcpy(&PlaneNavigationFilter_B.fv[164], &tmp_3[0], 160U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[324] = 0.01F;
    memset(&PlaneNavigationFilter_B.fv[325], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[343] = 0.01F;
    memset(&PlaneNavigationFilter_B.fv[344], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[362] = 0.01F;
    memset(&PlaneNavigationFilter_B.fv[363], 0, sizeof(real32_T) << 4U);
    PlaneNavigationFilter_B.fv[379] = PlaneNavigationFilter_B.t27;
    PlaneNavigationFilter_B.fv[380] = -PlaneNavigationFilter_B.residue;
    PlaneNavigationFilter_B.fv[381] = 0.01F;
    PlaneNavigationFilter_B.fv[382] = 0.0F;
    PlaneNavigationFilter_B.fv[383] = 0.0F;
    PlaneNavigationFilter_B.fv[384] = 0.0F;
    PlaneNavigationFilter_B.fv[385] = 0.0F;
    PlaneNavigationFilter_B.fv[386] = 0.0F;
    PlaneNavigationFilter_B.fv[387] = 0.0F;
    PlaneNavigationFilter_B.fv[388] = 0.0F;
    PlaneNavigationFilter_B.fv[389] = 0.0F;
    PlaneNavigationFilter_B.fv[390] = 0.0F;
    PlaneNavigationFilter_B.fv[391] = 0.0F;
    PlaneNavigationFilter_B.fv[392] = 0.0F;
    PlaneNavigationFilter_B.fv[393] = 0.0F;
    PlaneNavigationFilter_B.fv[394] = 0.0F;
    PlaneNavigationFilter_B.fv[395] = 0.0F;
    PlaneNavigationFilter_B.fv[396] = -PlaneNavigationFilter_B.t27;
    PlaneNavigationFilter_B.fv[397] = 0.0F;
    PlaneNavigationFilter_B.fv[398] = PlaneNavigationFilter_B.t25;
    PlaneNavigationFilter_B.fv[399] = PlaneNavigationFilter_B.t8 *
        PlaneNavigationFilter_B.t32;
    PlaneNavigationFilter_B.fv[400] = PlaneNavigationFilter_B.t31;
    PlaneNavigationFilter_B.fv[401] = PlaneNavigationFilter_B.t15 *
        PlaneNavigationFilter_B.t32;
    PlaneNavigationFilter_B.fv[402] = 0.0F;
    PlaneNavigationFilter_B.fv[403] = 0.0F;
    PlaneNavigationFilter_B.fv[404] = 0.0F;
    PlaneNavigationFilter_B.fv[405] = 0.0F;
    PlaneNavigationFilter_B.fv[406] = 0.0F;
    PlaneNavigationFilter_B.fv[407] = 0.0F;
    PlaneNavigationFilter_B.fv[408] = 0.0F;
    PlaneNavigationFilter_B.fv[409] = 0.0F;
    PlaneNavigationFilter_B.fv[410] = 0.0F;
    PlaneNavigationFilter_B.fv[411] = 0.0F;
    PlaneNavigationFilter_B.fv[412] = 0.0F;
    PlaneNavigationFilter_B.fv[413] = 0.0F;
    PlaneNavigationFilter_B.fv[414] = PlaneNavigationFilter_B.residue;
    PlaneNavigationFilter_B.fv[415] = -PlaneNavigationFilter_B.t25;
    PlaneNavigationFilter_B.fv[416] = 0.0F;
    PlaneNavigationFilter_B.fv[417] = PlaneNavigationFilter_B.t8 *
        PlaneNavigationFilter_B.t31;
    PlaneNavigationFilter_B.fv[418] = -PlaneNavigationFilter_B.t32;
    PlaneNavigationFilter_B.fv[419] = PlaneNavigationFilter_B.t15 *
        PlaneNavigationFilter_B.t31;
    memset(&PlaneNavigationFilter_B.fv[420], 0, 20U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[440] = 0.01F;
    memset(&PlaneNavigationFilter_B.fv[441], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[459] = 0.01F;
    memset(&PlaneNavigationFilter_B.fv[460], 0, 18U * sizeof(real32_T));
    PlaneNavigationFilter_B.fv[478] = 0.01F;
    PlaneNavigationFilter_B.fv[479] = 0.0F;
    PlaneNavigationFilter_B.fv[480] = 0.0F;
    PlaneNavigationFilter_B.fv[481] = 0.0F;
    PlaneNavigationFilter_B.fv[482] = 0.0F;
    PlaneNavigationFilter_B.fv[483] = 0.0F;
    PlaneNavigationFilter_B.fv[484] = 0.0F;
    PlaneNavigationFilter_B.fv[485] = 0.0F;
    PlaneNavigationFilter_B.fv[486] = 0.0F;
    PlaneNavigationFilter_B.fv[487] = 0.0F;
    PlaneNavigationFilter_B.fv[488] = 0.0F;
    PlaneNavigationFilter_B.fv[489] = 0.0F;
    PlaneNavigationFilter_B.fv[490] = 0.0F;
    PlaneNavigationFilter_B.fv[491] = 0.0F;
    PlaneNavigationFilter_B.fv[492] = 0.0F;
    memcpy(&PlaneNavigationFilter_B.fv[493], &tmp_4[0], 47U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 30;
            PlaneNavigationFilter_B.i++) {
        for (PlaneNavigationFilter_B.coffset = 0;
                PlaneNavigationFilter_B.coffset < 18;
                PlaneNavigationFilter_B.coffset++) {
            PlaneNavigationFilter_B.t8 = 0.0F;
            for (PlaneNavigationFilter_B.b_k = 0; PlaneNavigationFilter_B.b_k <
                    30; PlaneNavigationFilter_B.b_k++) {
                PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.fv[18 *
                    PlaneNavigationFilter_B.b_k +
                    PlaneNavigationFilter_B.coffset] *
                    PlaneNavigationFilter_P.Q_Value[30 *
                    PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.b_k];
            }

            PlaneNavigationFilter_B.fv1[PlaneNavigationFilter_B.i + 30 *
                PlaneNavigationFilter_B.coffset] = PlaneNavigationFilter_B.t8;
        }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        memcpy(&PlaneNavigationFilter_B.y[PlaneNavigationFilter_B.i * 48],
               &PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.i * 18],
               18U * sizeof(real32_T));
        memcpy(&PlaneNavigationFilter_B.y[PlaneNavigationFilter_B.i * 48 + 18],
               &PlaneNavigationFilter_B.fv1[PlaneNavigationFilter_B.i * 30], 30U
               * sizeof(real32_T));
    }

    /* MATLABSystem: '<S10>/MATLAB System' */
    PlaneNavigationFilter_qr_dk0ct(PlaneNavigationFilter_B.y,
        PlaneNavigationFilter_B.a__1, PlaneNavigationFilter_B.y_cv);

    /* DataStoreWrite: '<S10>/Data Store WriteP' incorporates:
     *  MATLABSystem: '<S10>/MATLAB System'
     * */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
            PlaneNavigationFilter_B.i++) {
        for (PlaneNavigationFilter_B.coffset = 0;
                PlaneNavigationFilter_B.coffset < 18;
                PlaneNavigationFilter_B.coffset++) {
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
                PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.y_cv[18 *
                PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
        }
    }

    /* End of DataStoreWrite: '<S10>/Data Store WriteP' */

    /* MATLABSystem: '<S10>/MATLAB System' */
    memset(&PlaneNavigationFilter_B.fv2[0], 0, 30U * sizeof(real32_T));

    /* Start for MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreRead: '<S10>/Data Store ReadX'
     */
    memcpy(&PlaneNavigationFilter_B.Pxy[0], &PlaneNavigationFilter_DW.x[0], 18U *
           sizeof(real32_T));

    /* MATLABSystem: '<S10>/MATLAB System' incorporates:
     *  DataStoreWrite: '<S10>/Data Store WriteX'
     */
    PlaneNavigation_nav_state_trans_dt_100Hz(PlaneNavigationFilter_B.Pxy,
        PlaneNavigationFilter_B.fv2,
        PlaneNavigationFilter_B.TmpSignalConversionAtMATLABSystemInp,
        PlaneNavigationFilter_DW.x);

    /* End of Outputs for SubSystem: '<S2>/Predict' */

    /* Update for Delay: '<Root>/Delay' */
    memcpy(&PlaneNavigationFilter_DW.Delay_DSTATE[0],
           &PlaneNavigationFilter_B.ekf_state_std[0], 18U * sizeof(real32_T));
}

/* Model initialize function */
void PlaneNavigationFilter_initialize(void)
{
    /* Start for DataStoreMemory: '<S2>/DataStoreMemory - P' */
    memcpy(&PlaneNavigationFilter_DW.P[0],
           &PlaneNavigationFilter_P.DataStoreMemoryP_InitialValue[0], 324U *
           sizeof(real32_T));

    /* Start for DataStoreMemory: '<S2>/DataStoreMemory - x' */
    memcpy(&PlaneNavigationFilter_DW.x[0],
           &PlaneNavigationFilter_P.DataStoreMemoryx_InitialValue[0], 18U *
           sizeof(real32_T));

    /* InitializeConditions for Delay: '<Root>/Delay' */
    memcpy(&PlaneNavigationFilter_DW.Delay_DSTATE[0],
           &PlaneNavigationFilter_P.initState[0], 18U * sizeof(real32_T));

    /* SystemInitialize for Enabled SubSystem: '<S2>/Correct1' */
    /* SystemInitialize for MATLABSystem: '<S6>/MATLAB System' incorporates:
     *  Outport: '<S6>/yBlockOrdering'
     */
    PlaneNavigationFilter_B.MATLABSystem_o3_f =
        PlaneNavigationFilter_P.yBlockOrdering_Y0;

    /* End of SystemInitialize for SubSystem: '<S2>/Correct1' */
}

/* Model terminate function */
void PlaneNavigationFilter_terminate(void)
{
    /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
