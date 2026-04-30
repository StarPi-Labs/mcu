/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: PlaneNavigationFilter.c
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

#include "PlaneNavigationFilter.h"
#include "rtwtypes.h"
#include <math.h>
#include <string.h>
#include "PlaneNavigationFilter_private.h"
#include "rt_nonfinite.h"

/* Named constants for Chart: '<Root>/EKF Logic' */
#define PlaneNavigationFi_IN_EKF_ACTIVE ((uint8_T)1U)
#define PlaneNavigationFilt_IN_EKF_INIT ((uint8_T)2U)
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
static RT_MODEL_PlaneNavigationFilte_T PlaneNavigationFilter_M_;
RT_MODEL_PlaneNavigationFilte_T *const PlaneNavigationFilter_M =
  &PlaneNavigationFilter_M_;

/* Forward declaration for local functions */
static boolean_T PlaneNavigationFi_check_ekf_std(const real32_T b_std[18]);
static real32_T PlaneNavigationFilter_xnrm2(int32_T n, const real32_T x[175],
  int32_T ix0);
static void PlaneNavigationFilter_qr(const real32_T A[175], real32_T Q[175],
  real32_T R[49]);
static void PlaneNavigationFilter_trisolve(const real32_T A[49], real32_T B[126]);
static void PlaneNavigationFilte_trisolve_d(const real32_T A[49], real32_T B[126]);
static real32_T PlaneNavigationFilter_xnrm2_d(int32_T n, const real32_T x[450],
  int32_T ix0);
static void PlaneNavigationFilter_qr_d(const real32_T A[450], real32_T Q[450],
  real32_T R[324]);
static real32_T PlaneNavigationFilter_xnrm2_dk(int32_T n, const real32_T x[40],
  int32_T ix0);
static void PlaneNavigationFilter_qr_dk(const real32_T A[40], real32_T Q[40],
  real32_T R[4]);
static void PlaneNavigationFilt_trisolve_dk(const real32_T A[4], real32_T B[36]);
static void PlaneNavigationFil_trisolve_dk0(const real32_T A[4], real32_T B[36]);
static real32_T PlaneNavigationFilter_xnrm2_dk0(int32_T n, const real32_T x[360],
  int32_T ix0);
static void PlaneNavigationFilter_qr_dk0(const real32_T A[360], real32_T Q[360],
  real32_T R[324]);
static real32_T PlaneNavigationFilte_xnrm2_dk0c(int32_T n, const real32_T x[19],
  int32_T ix0);
static void EKFCorrectorAdditive_getMeasure(real32_T Rs, const real32_T x[18],
  const real32_T S[324], real32_T *zEstimated, real32_T Pxy[18], real32_T *Sy,
  real32_T dHdx[18], real32_T *Rsqrt);
static void PlaneNavigationFi_trisolve_dk0c(real32_T A, real32_T B[18]);
static real32_T PlaneNavigationFilt_xnrm2_dk0ct(int32_T n, const real32_T x[342],
  int32_T ix0);
static void PlaneNavigationFilter_qr_dk0c(const real32_T A[342], real32_T Q[342],
  real32_T R[324]);
static real32_T PlaneNavigationFil_xnrm2_dk0ctr(int32_T n, const real32_T x[864],
  int32_T ix0);
static void PlaneNavigationFilter_qr_dk0ct(const real32_T A[864], real32_T Q[864],
  real32_T R[324]);
static void PlaneN_nav_state_trans_dt_100Hz(const real32_T state[18], const
  real32_T w[30], const real32_T u[6], real32_T x_next[18]);
int32_T div_nde_s32_floor(int32_T numerator, int32_T denominator)
{
  return (((numerator < 0) != (denominator < 0)) && (numerator % denominator !=
           0) ? -1 : 0) + numerator / denominator;
}

/* Function for Chart: '<Root>/EKF Logic' */
static boolean_T PlaneNavigationFi_check_ekf_std(const real32_T b_std[18])
{
  int32_T i;
  boolean_T x[18];
  boolean_T ekf_good;
  boolean_T exitg1;
  for (i = 0; i < 18; i++) {
    x[i] = (b_std[i] <= 10.0F);
  }

  ekf_good = true;
  i = 0;
  exitg1 = false;
  while ((!exitg1) && (i < 18)) {
    if (!x[i]) {
      ekf_good = false;
      exitg1 = true;
    } else {
      i++;
    }
  }

  return ekf_good;
}

static real32_T PlaneNavigationFilter_xnrm2(int32_T n, const real32_T x[175],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S5>/MATLAB System' */
  return y;
}

real32_T rt_hypotf_snf(real32_T u0, real32_T u1)
{
  real32_T a;
  real32_T b;
  real32_T y;
  a = (real32_T)fabs(u0);
  b = (real32_T)fabs(u1);
  if (a < b) {
    a /= b;
    y = (real32_T)sqrt(a * a + 1.0F) * b;
  } else if (a > b) {
    b /= a;
    y = (real32_T)sqrt(b * b + 1.0F) * a;
  } else if (rtIsNaNF(b)) {
    y = (rtNaNF);
  } else {
    y = a * 1.41421354F;
  }

  return y;
}

static void PlaneNavigationFilter_qr(const real32_T A[175], real32_T Q[175],
  real32_T R[49])
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;
  for (i = 0; i < 7; i++) {
    /* Start for MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_B.b_tau_ln[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  memcpy(&Q[0], &A[0], 175U * sizeof(real32_T));
  for (i = 0; i < 7; i++) {
    /* Start for MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_B.work_h[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  for (PlaneNavigationFilter_B.iaii_md = 0; PlaneNavigationFilter_B.iaii_md < 7;
       PlaneNavigationFilter_B.iaii_md++) {
    PlaneNavigationFilter_B.ii_m = PlaneNavigationFilter_B.iaii_md * 25 +
      PlaneNavigationFilter_B.iaii_md;
    i = PlaneNavigationFilter_B.ii_m + 2;
    PlaneNavigationFilter_B.atmp_ju = Q[PlaneNavigationFilter_B.ii_m];
    PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md] = 0.0F;
    beta1 = PlaneNavigationFilter_xnrm2(24 - PlaneNavigationFilter_B.iaii_md, Q,
      PlaneNavigationFilter_B.ii_m + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii_m];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b = PlaneNavigationFilter_B.ii_m - PlaneNavigationFilter_B.iaii_md;
          for (lastv = i; lastv <= b + 25; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp_ju *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp_ju,
                              PlaneNavigationFilter_xnrm2(24 -
          PlaneNavigationFilter_B.iaii_md, Q, PlaneNavigationFilter_B.ii_m + 2));
        if (PlaneNavigationFilter_B.atmp_ju >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md] =
          (beta1 - PlaneNavigationFilter_B.atmp_ju) / beta1;
        PlaneNavigationFilter_B.atmp_ju = 1.0F /
          (PlaneNavigationFilter_B.atmp_ju - beta1);
        for (lastv = i; lastv <= b + 25; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_ju;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp_ju = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md] =
          (beta1 - c) / beta1;
        PlaneNavigationFilter_B.atmp_ju = 1.0F / (c - beta1);
        b = PlaneNavigationFilter_B.ii_m - PlaneNavigationFilter_B.iaii_md;
        for (lastv = i; lastv <= b + 25; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_ju;
        }

        PlaneNavigationFilter_B.atmp_ju = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii_m] = PlaneNavigationFilter_B.atmp_ju;
    if (PlaneNavigationFilter_B.iaii_md + 1 < 7) {
      Q[PlaneNavigationFilter_B.ii_m] = 1.0F;
      knt = PlaneNavigationFilter_B.ii_m + 26;
      if (PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md] !=
          0.0F) {
        lastv = 25 - PlaneNavigationFilter_B.iaii_md;
        i = PlaneNavigationFilter_B.ii_m - PlaneNavigationFilter_B.iaii_md;
        while ((lastv > 0) && (Q[i + 24] == 0.0F)) {
          lastv--;
          i--;
        }

        b = 6 - PlaneNavigationFilter_B.iaii_md;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 25 + PlaneNavigationFilter_B.ii_m;
          jA = lastc + 26;
          do {
            exitg1 = 0;
            if (jA <= (lastc + lastv) + 25) {
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
            memset(&PlaneNavigationFilter_B.work_h[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (25 * lastc + PlaneNavigationFilter_B.ii_m) + 26;
          for (iac = knt; iac <= b; iac += 25) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii_m + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii_m) - 26, 25);
            PlaneNavigationFilter_B.work_h[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md]
              == 0.0F)) {
          jA = PlaneNavigationFilter_B.ii_m;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_h[iac];
            if (c != 0.0F) {
              c *=
                -PlaneNavigationFilter_B.b_tau_ln[PlaneNavigationFilter_B.iaii_md];
              b = jA + 26;
              knt = (lastv + jA) + 25;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_m + d) - jA) - 26] *
                  c;
              }
            }

            jA += 25;
          }
        }
      }

      Q[PlaneNavigationFilter_B.ii_m] = PlaneNavigationFilter_B.atmp_ju;
    }
  }

  for (i = 0; i < 7; i++) {
    for (PlaneNavigationFilter_B.iaii_md = 0; PlaneNavigationFilter_B.iaii_md <=
         i; PlaneNavigationFilter_B.iaii_md++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii_md + 7 * i] = Q[25 * i +
        PlaneNavigationFilter_B.iaii_md];
    }

    for (PlaneNavigationFilter_B.iaii_md = i + 2;
         PlaneNavigationFilter_B.iaii_md < 8; PlaneNavigationFilter_B.iaii_md++)
    {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      R[(PlaneNavigationFilter_B.iaii_md + 7 * i) - 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_B.work_h[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  for (i = 6; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii_md = (i * 25 + i) + 25;
    if (i + 1 < 7) {
      Q[PlaneNavigationFilter_B.iaii_md - 25] = 1.0F;
      knt = PlaneNavigationFilter_B.iaii_md + 1;
      if (PlaneNavigationFilter_B.b_tau_ln[i] != 0.0F) {
        lastv = 25 - i;
        PlaneNavigationFilter_B.ii_m = (PlaneNavigationFilter_B.iaii_md - i) - 1;
        while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_m] == 0.0F)) {
          lastv--;
          PlaneNavigationFilter_B.ii_m--;
        }

        b = 6 - i;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 25 + PlaneNavigationFilter_B.iaii_md;
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
            memset(&PlaneNavigationFilter_B.work_h[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (25 * lastc + PlaneNavigationFilter_B.iaii_md) + 1;
          for (iac = knt; iac <= b; iac += 25) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii_md + jA) - iac) - 25] * Q[jA
                - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii_md) - 1,
              25);
            PlaneNavigationFilter_B.work_h[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_ln[i] == 0.0F)) {
          jA = PlaneNavigationFilter_B.iaii_md;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_h[iac];
            if (c != 0.0F) {
              c *= -PlaneNavigationFilter_B.b_tau_ln[i];
              b = jA + 1;
              knt = lastv + jA;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_md + d) - jA) - 26]
                  * c;
              }
            }

            jA += 25;
          }
        }
      }
    }

    b = PlaneNavigationFilter_B.iaii_md - i;
    for (lastv = PlaneNavigationFilter_B.iaii_md - 23; lastv <= b; lastv++) {
      Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_ln[i];
    }

    Q[PlaneNavigationFilter_B.iaii_md - 25] = 1.0F -
      PlaneNavigationFilter_B.b_tau_ln[i];
    for (PlaneNavigationFilter_B.ii_m = 0; PlaneNavigationFilter_B.ii_m < i;
         PlaneNavigationFilter_B.ii_m++) {
      Q[(PlaneNavigationFilter_B.iaii_md - PlaneNavigationFilter_B.ii_m) - 26] =
        0.0F;
    }
  }
}

static void PlaneNavigationFilter_trisolve(const real32_T A[49], real32_T B[126])
{
  int32_T B_tmp;
  int32_T b_j;
  int32_T b_k;
  int32_T i;
  int32_T jBcol;
  int32_T k;
  int32_T kAcol;
  int32_T tmp;
  real32_T B_0;

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  for (b_j = 0; b_j < 18; b_j++) {
    jBcol = 7 * b_j - 1;
    for (b_k = 0; b_k < 7; b_k++) {
      k = b_k + 1;
      kAcol = b_k * 7 - 1;
      B_tmp = (b_k + jBcol) + 1;
      B_0 = B[B_tmp];
      if (B_0 != 0.0F) {
        B[B_tmp] = B_0 / A[(b_k + kAcol) + 1];
        for (i = k + 1; i < 8; i++) {
          tmp = i + jBcol;
          B[tmp] -= A[i + kAcol] * B[B_tmp];
        }
      }
    }
  }

  /* End of Start for MATLABSystem: '<S5>/MATLAB System' */
}

static void PlaneNavigationFilte_trisolve_d(const real32_T A[49], real32_T B[126])
{
  int32_T b_i;
  int32_T b_j;
  int32_T jBcol;
  int32_T k;
  int32_T kAcol;
  int32_T tmp_0;
  int32_T tmp_1;
  real32_T tmp;

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  for (b_j = 0; b_j < 18; b_j++) {
    jBcol = 7 * b_j;
    for (k = 6; k >= 0; k--) {
      kAcol = 7 * k;
      tmp_0 = k + jBcol;
      tmp = B[tmp_0];
      if (tmp != 0.0F) {
        B[tmp_0] = tmp / A[k + kAcol];
        for (b_i = 0; b_i < k; b_i++) {
          tmp_1 = b_i + jBcol;
          B[tmp_1] -= A[b_i + kAcol] * B[tmp_0];
        }
      }
    }
  }

  /* End of Start for MATLABSystem: '<S5>/MATLAB System' */
}

static real32_T PlaneNavigationFilter_xnrm2_d(int32_T n, const real32_T x[450],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S5>/MATLAB System' */
  return y;
}

static void PlaneNavigationFilter_qr_d(const real32_T A[450], real32_T Q[450],
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  memset(&PlaneNavigationFilter_B.b_tau_b[0], 0, 18U * sizeof(real32_T));
  memcpy(&Q[0], &A[0], 450U * sizeof(real32_T));
  memset(&PlaneNavigationFilter_B.work_n[0], 0, 18U * sizeof(real32_T));
  for (PlaneNavigationFilter_B.iaii_m = 0; PlaneNavigationFilter_B.iaii_m < 18;
       PlaneNavigationFilter_B.iaii_m++) {
    PlaneNavigationFilter_B.ii_c = PlaneNavigationFilter_B.iaii_m * 25 +
      PlaneNavigationFilter_B.iaii_m;
    i = PlaneNavigationFilter_B.ii_c + 2;
    PlaneNavigationFilter_B.atmp_a = Q[PlaneNavigationFilter_B.ii_c];
    PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m] = 0.0F;
    beta1 = PlaneNavigationFilter_xnrm2_d(24 - PlaneNavigationFilter_B.iaii_m, Q,
      PlaneNavigationFilter_B.ii_c + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii_c];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b = PlaneNavigationFilter_B.ii_c - PlaneNavigationFilter_B.iaii_m;
          for (lastv = i; lastv <= b + 25; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp_a *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp_a,
                              PlaneNavigationFilter_xnrm2_d(24 -
          PlaneNavigationFilter_B.iaii_m, Q, PlaneNavigationFilter_B.ii_c + 2));
        if (PlaneNavigationFilter_B.atmp_a >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m] = (beta1
          - PlaneNavigationFilter_B.atmp_a) / beta1;
        PlaneNavigationFilter_B.atmp_a = 1.0F / (PlaneNavigationFilter_B.atmp_a
          - beta1);
        for (lastv = i; lastv <= b + 25; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_a;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp_a = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m] = (beta1
          - c) / beta1;
        PlaneNavigationFilter_B.atmp_a = 1.0F / (c - beta1);
        b = PlaneNavigationFilter_B.ii_c - PlaneNavigationFilter_B.iaii_m;
        for (lastv = i; lastv <= b + 25; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_a;
        }

        PlaneNavigationFilter_B.atmp_a = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii_c] = PlaneNavigationFilter_B.atmp_a;
    if (PlaneNavigationFilter_B.iaii_m + 1 < 18) {
      Q[PlaneNavigationFilter_B.ii_c] = 1.0F;
      knt = PlaneNavigationFilter_B.ii_c + 26;
      if (PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m] !=
          0.0F) {
        lastv = 25 - PlaneNavigationFilter_B.iaii_m;
        i = PlaneNavigationFilter_B.ii_c - PlaneNavigationFilter_B.iaii_m;
        while ((lastv > 0) && (Q[i + 24] == 0.0F)) {
          lastv--;
          i--;
        }

        b = 17 - PlaneNavigationFilter_B.iaii_m;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 25 + PlaneNavigationFilter_B.ii_c;
          jA = lastc + 26;
          do {
            exitg1 = 0;
            if (jA <= (lastc + lastv) + 25) {
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
            memset(&PlaneNavigationFilter_B.work_n[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (25 * lastc + PlaneNavigationFilter_B.ii_c) + 26;
          for (iac = knt; iac <= b; iac += 25) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii_c + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii_c) - 26, 25);
            PlaneNavigationFilter_B.work_n[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m] ==
              0.0F)) {
          jA = PlaneNavigationFilter_B.ii_c;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_n[iac];
            if (c != 0.0F) {
              c *=
                -PlaneNavigationFilter_B.b_tau_b[PlaneNavigationFilter_B.iaii_m];
              b = jA + 26;
              knt = (lastv + jA) + 25;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_c + d) - jA) - 26] *
                  c;
              }
            }

            jA += 25;
          }
        }
      }

      Q[PlaneNavigationFilter_B.ii_c] = PlaneNavigationFilter_B.atmp_a;
    }
  }

  for (i = 0; i < 18; i++) {
    for (PlaneNavigationFilter_B.iaii_m = 0; PlaneNavigationFilter_B.iaii_m <= i;
         PlaneNavigationFilter_B.iaii_m++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii_m + 18 * i] = Q[25 * i +
        PlaneNavigationFilter_B.iaii_m];
    }

    for (PlaneNavigationFilter_B.iaii_m = i + 2; PlaneNavigationFilter_B.iaii_m <
         19; PlaneNavigationFilter_B.iaii_m++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      R[(PlaneNavigationFilter_B.iaii_m + 18 * i) - 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_B.work_n[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S5>/MATLAB System' */
  for (i = 17; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii_m = (i * 25 + i) + 25;
    if (i + 1 < 18) {
      Q[PlaneNavigationFilter_B.iaii_m - 25] = 1.0F;
      knt = PlaneNavigationFilter_B.iaii_m + 1;
      if (PlaneNavigationFilter_B.b_tau_b[i] != 0.0F) {
        lastv = 25 - i;
        PlaneNavigationFilter_B.ii_c = (PlaneNavigationFilter_B.iaii_m - i) - 1;
        while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_c] == 0.0F)) {
          lastv--;
          PlaneNavigationFilter_B.ii_c--;
        }

        b = 17 - i;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 25 + PlaneNavigationFilter_B.iaii_m;
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
            memset(&PlaneNavigationFilter_B.work_n[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (25 * lastc + PlaneNavigationFilter_B.iaii_m) + 1;
          for (iac = knt; iac <= b; iac += 25) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii_m + jA) - iac) - 25] * Q[jA
                - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii_m) - 1,
              25);
            PlaneNavigationFilter_B.work_n[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_b[i] == 0.0F)) {
          jA = PlaneNavigationFilter_B.iaii_m;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_n[iac];
            if (c != 0.0F) {
              c *= -PlaneNavigationFilter_B.b_tau_b[i];
              b = jA + 1;
              knt = lastv + jA;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_m + d) - jA) - 26] *
                  c;
              }
            }

            jA += 25;
          }
        }
      }
    }

    b = PlaneNavigationFilter_B.iaii_m - i;
    for (lastv = PlaneNavigationFilter_B.iaii_m - 23; lastv <= b; lastv++) {
      Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_b[i];
    }

    Q[PlaneNavigationFilter_B.iaii_m - 25] = 1.0F -
      PlaneNavigationFilter_B.b_tau_b[i];
    for (PlaneNavigationFilter_B.ii_c = 0; PlaneNavigationFilter_B.ii_c < i;
         PlaneNavigationFilter_B.ii_c++) {
      Q[(PlaneNavigationFilter_B.iaii_m - PlaneNavigationFilter_B.ii_c) - 26] =
        0.0F;
    }
  }
}

static real32_T PlaneNavigationFilter_xnrm2_dk(int32_T n, const real32_T x[40],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  PlaneNavigationFilter_B.b_tau_e[0] = 0.0F;
  PlaneNavigationFilter_B.b_tau_e[1] = 0.0F;
  memcpy(&Q[0], &A[0], 40U * sizeof(real32_T));
  PlaneNavigationFilter_B.work_b[0] = 0.0F;
  PlaneNavigationFilter_B.work_b[1] = 0.0F;
  for (PlaneNavigationFilter_B.iaii_j = 0; PlaneNavigationFilter_B.iaii_j < 2;
       PlaneNavigationFilter_B.iaii_j++) {
    PlaneNavigationFilter_B.ii_h = PlaneNavigationFilter_B.iaii_j * 20 +
      PlaneNavigationFilter_B.iaii_j;
    i = PlaneNavigationFilter_B.ii_h + 2;
    PlaneNavigationFilter_B.atmp_jz = Q[PlaneNavigationFilter_B.ii_h];
    PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j] = 0.0F;
    beta1 = PlaneNavigationFilter_xnrm2_dk(19 - PlaneNavigationFilter_B.iaii_j,
      Q, PlaneNavigationFilter_B.ii_h + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii_h];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b_tmp = PlaneNavigationFilter_B.ii_h - PlaneNavigationFilter_B.iaii_j;
          for (lastv = i; lastv <= b_tmp + 20; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp_jz *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp_jz,
                              PlaneNavigationFilter_xnrm2_dk(19 -
          PlaneNavigationFilter_B.iaii_j, Q, PlaneNavigationFilter_B.ii_h + 2));
        if (PlaneNavigationFilter_B.atmp_jz >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j] = (beta1
          - PlaneNavigationFilter_B.atmp_jz) / beta1;
        PlaneNavigationFilter_B.atmp_jz = 1.0F /
          (PlaneNavigationFilter_B.atmp_jz - beta1);
        for (lastv = i; lastv <= b_tmp + 20; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_jz;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp_jz = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j] = (beta1
          - c) / beta1;
        PlaneNavigationFilter_B.atmp_jz = 1.0F / (c - beta1);
        knt = PlaneNavigationFilter_B.ii_h - PlaneNavigationFilter_B.iaii_j;
        for (lastv = i; lastv <= knt + 20; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_jz;
        }

        PlaneNavigationFilter_B.atmp_jz = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii_h] = PlaneNavigationFilter_B.atmp_jz;
    if (PlaneNavigationFilter_B.iaii_j + 1 < 2) {
      Q[PlaneNavigationFilter_B.ii_h] = 1.0F;
      b_tmp = PlaneNavigationFilter_B.ii_h + 21;
      if (PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j] !=
          0.0F) {
        lastv = 20 - PlaneNavigationFilter_B.iaii_j;
        i = PlaneNavigationFilter_B.ii_h - PlaneNavigationFilter_B.iaii_j;
        while ((lastv > 0) && (Q[i + 19] == 0.0F)) {
          lastv--;
          i--;
        }

        knt = 1 - PlaneNavigationFilter_B.iaii_j;
        exitg2 = false;
        while ((!exitg2) && (knt > 0)) {
          jA = PlaneNavigationFilter_B.ii_h + 21;
          do {
            exitg1 = 0;
            if (jA <= (PlaneNavigationFilter_B.ii_h + lastv) + 20) {
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

          knt = (20 * lastc + PlaneNavigationFilter_B.ii_h) + 21;
          for (iac = b_tmp; iac <= knt; iac += 20) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii_h + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii_h) - 21, 20);
            PlaneNavigationFilter_B.work_b[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j] ==
              0.0F)) {
          jA = PlaneNavigationFilter_B.ii_h;
          for (iac = 0; iac <= lastc; iac++) {
            if (PlaneNavigationFilter_B.work_b[0] != 0.0F) {
              c = PlaneNavigationFilter_B.work_b[0] *
                -PlaneNavigationFilter_B.b_tau_e[PlaneNavigationFilter_B.iaii_j];
              knt = jA + 21;
              b_tmp = (lastv + jA) + 20;
              for (d = knt; d <= b_tmp; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_h + d) - jA) - 21] *
                  c;
              }
            }

            jA += 20;
          }
        }
      }

      Q[PlaneNavigationFilter_B.ii_h] = PlaneNavigationFilter_B.atmp_jz;
    }
  }

  for (PlaneNavigationFilter_B.ii_h = 0; PlaneNavigationFilter_B.ii_h < 2;
       PlaneNavigationFilter_B.ii_h++) {
    for (PlaneNavigationFilter_B.iaii_j = 0; PlaneNavigationFilter_B.iaii_j <=
         PlaneNavigationFilter_B.ii_h; PlaneNavigationFilter_B.iaii_j++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii_j + (PlaneNavigationFilter_B.ii_h << 1)] =
        Q[20 * PlaneNavigationFilter_B.ii_h + PlaneNavigationFilter_B.iaii_j];
    }

    if (PlaneNavigationFilter_B.ii_h + 2 <= 2) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      R[(PlaneNavigationFilter_B.ii_h << 1) + 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilter_B.work_b[PlaneNavigationFilter_B.ii_h] = 0.0F;
  }

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  for (i = 1; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii_j = (i * 20 + i) + 20;
    if (i + 1 < 2) {
      Q[PlaneNavigationFilter_B.iaii_j - 20] = 1.0F;
      b_tmp = PlaneNavigationFilter_B.iaii_j + 1;
      if (PlaneNavigationFilter_B.b_tau_e[i] != 0.0F) {
        lastv = 20 - i;
        PlaneNavigationFilter_B.ii_h = (PlaneNavigationFilter_B.iaii_j - i) - 1;
        while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_h] == 0.0F)) {
          lastv--;
          PlaneNavigationFilter_B.ii_h--;
        }

        knt = 1 - i;
        exitg2 = false;
        while ((!exitg2) && (knt > 0)) {
          jA = PlaneNavigationFilter_B.iaii_j + 1;
          do {
            exitg1 = 0;
            if (jA <= PlaneNavigationFilter_B.iaii_j + lastv) {
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

          knt = (20 * lastc + PlaneNavigationFilter_B.iaii_j) + 1;
          for (iac = b_tmp; iac <= knt; iac += 20) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii_j + jA) - iac) - 20] * Q[jA
                - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii_j) - 1,
              20);
            PlaneNavigationFilter_B.work_b[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_e[i] == 0.0F)) {
          jA = PlaneNavigationFilter_B.iaii_j;
          for (iac = 0; iac <= lastc; iac++) {
            if (PlaneNavigationFilter_B.work_b[0] != 0.0F) {
              c = PlaneNavigationFilter_B.work_b[0] *
                -PlaneNavigationFilter_B.b_tau_e[i];
              knt = jA + 1;
              b_tmp = lastv + jA;
              for (d = knt; d <= b_tmp; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_j + d) - jA) - 21] *
                  c;
              }
            }

            jA += 20;
          }
        }
      }
    }

    knt = PlaneNavigationFilter_B.iaii_j - i;
    for (lastv = PlaneNavigationFilter_B.iaii_j - 18; lastv <= knt; lastv++) {
      Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_e[i];
    }

    Q[PlaneNavigationFilter_B.iaii_j - 20] = 1.0F -
      PlaneNavigationFilter_B.b_tau_e[i];
    if (i - 1 >= 0) {
      Q[PlaneNavigationFilter_B.iaii_j - 21] = 0.0F;
    }
  }
}

static void PlaneNavigationFilt_trisolve_dk(const real32_T A[4], real32_T B[36])
{
  int32_T B_tmp;
  int32_T b_j;
  int32_T b_k;
  int32_T i;
  int32_T jBcol;
  int32_T k;
  int32_T kAcol;
  real32_T B_0;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  for (b_j = 0; b_j < 18; b_j++) {
    jBcol = (b_j << 1) - 1;
    for (b_k = 0; b_k < 2; b_k++) {
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

  /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
}

static void PlaneNavigationFil_trisolve_dk0(const real32_T A[4], real32_T B[36])
{
  int32_T b_i;
  int32_T b_j;
  int32_T jBcol;
  int32_T k;
  int32_T kAcol;
  int32_T tmp_0;
  real32_T tmp;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  for (b_j = 0; b_j < 18; b_j++) {
    jBcol = b_j << 1;
    for (k = 1; k >= 0; k--) {
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

  /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
}

static real32_T PlaneNavigationFilter_xnrm2_dk0(int32_T n, const real32_T x[360],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S6>/MATLAB System' */
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  memset(&PlaneNavigationFilter_B.b_tau_l[0], 0, 18U * sizeof(real32_T));
  memcpy(&Q[0], &A[0], 360U * sizeof(real32_T));
  memset(&PlaneNavigationFilter_B.work_o[0], 0, 18U * sizeof(real32_T));
  for (PlaneNavigationFilter_B.iaii_o = 0; PlaneNavigationFilter_B.iaii_o < 18;
       PlaneNavigationFilter_B.iaii_o++) {
    PlaneNavigationFilter_B.ii_n = PlaneNavigationFilter_B.iaii_o * 20 +
      PlaneNavigationFilter_B.iaii_o;
    i = PlaneNavigationFilter_B.ii_n + 2;
    PlaneNavigationFilter_B.atmp_f = Q[PlaneNavigationFilter_B.ii_n];
    PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o] = 0.0F;
    beta1 = PlaneNavigationFilter_xnrm2_dk0(19 - PlaneNavigationFilter_B.iaii_o,
      Q, PlaneNavigationFilter_B.ii_n + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii_n];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b = PlaneNavigationFilter_B.ii_n - PlaneNavigationFilter_B.iaii_o;
          for (lastv = i; lastv <= b + 20; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp_f *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp_f,
                              PlaneNavigationFilter_xnrm2_dk0(19 -
          PlaneNavigationFilter_B.iaii_o, Q, PlaneNavigationFilter_B.ii_n + 2));
        if (PlaneNavigationFilter_B.atmp_f >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o] = (beta1
          - PlaneNavigationFilter_B.atmp_f) / beta1;
        PlaneNavigationFilter_B.atmp_f = 1.0F / (PlaneNavigationFilter_B.atmp_f
          - beta1);
        for (lastv = i; lastv <= b + 20; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_f;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp_f = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o] = (beta1
          - c) / beta1;
        PlaneNavigationFilter_B.atmp_f = 1.0F / (c - beta1);
        b = PlaneNavigationFilter_B.ii_n - PlaneNavigationFilter_B.iaii_o;
        for (lastv = i; lastv <= b + 20; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_f;
        }

        PlaneNavigationFilter_B.atmp_f = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii_n] = PlaneNavigationFilter_B.atmp_f;
    if (PlaneNavigationFilter_B.iaii_o + 1 < 18) {
      Q[PlaneNavigationFilter_B.ii_n] = 1.0F;
      knt = PlaneNavigationFilter_B.ii_n + 21;
      if (PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o] !=
          0.0F) {
        lastv = 20 - PlaneNavigationFilter_B.iaii_o;
        i = PlaneNavigationFilter_B.ii_n - PlaneNavigationFilter_B.iaii_o;
        while ((lastv > 0) && (Q[i + 19] == 0.0F)) {
          lastv--;
          i--;
        }

        b = 17 - PlaneNavigationFilter_B.iaii_o;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 20 + PlaneNavigationFilter_B.ii_n;
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
            memset(&PlaneNavigationFilter_B.work_o[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (20 * lastc + PlaneNavigationFilter_B.ii_n) + 21;
          for (iac = knt; iac <= b; iac += 20) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii_n + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii_n) - 21, 20);
            PlaneNavigationFilter_B.work_o[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o] ==
              0.0F)) {
          jA = PlaneNavigationFilter_B.ii_n;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_o[iac];
            if (c != 0.0F) {
              c *=
                -PlaneNavigationFilter_B.b_tau_l[PlaneNavigationFilter_B.iaii_o];
              b = jA + 21;
              knt = (lastv + jA) + 20;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_n + d) - jA) - 21] *
                  c;
              }
            }

            jA += 20;
          }
        }
      }

      Q[PlaneNavigationFilter_B.ii_n] = PlaneNavigationFilter_B.atmp_f;
    }
  }

  for (i = 0; i < 18; i++) {
    for (PlaneNavigationFilter_B.iaii_o = 0; PlaneNavigationFilter_B.iaii_o <= i;
         PlaneNavigationFilter_B.iaii_o++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii_o + 18 * i] = Q[20 * i +
        PlaneNavigationFilter_B.iaii_o];
    }

    for (PlaneNavigationFilter_B.iaii_o = i + 2; PlaneNavigationFilter_B.iaii_o <
         19; PlaneNavigationFilter_B.iaii_o++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      R[(PlaneNavigationFilter_B.iaii_o + 18 * i) - 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilter_B.work_o[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S6>/MATLAB System' */
  for (i = 17; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii_o = (i * 20 + i) + 20;
    if (i + 1 < 18) {
      Q[PlaneNavigationFilter_B.iaii_o - 20] = 1.0F;
      knt = PlaneNavigationFilter_B.iaii_o + 1;
      if (PlaneNavigationFilter_B.b_tau_l[i] != 0.0F) {
        lastv = 20 - i;
        PlaneNavigationFilter_B.ii_n = (PlaneNavigationFilter_B.iaii_o - i) - 1;
        while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_n] == 0.0F)) {
          lastv--;
          PlaneNavigationFilter_B.ii_n--;
        }

        b = 17 - i;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 20 + PlaneNavigationFilter_B.iaii_o;
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
            memset(&PlaneNavigationFilter_B.work_o[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (20 * lastc + PlaneNavigationFilter_B.iaii_o) + 1;
          for (iac = knt; iac <= b; iac += 20) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii_o + jA) - iac) - 20] * Q[jA
                - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii_o) - 1,
              20);
            PlaneNavigationFilter_B.work_o[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_l[i] == 0.0F)) {
          jA = PlaneNavigationFilter_B.iaii_o;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_o[iac];
            if (c != 0.0F) {
              c *= -PlaneNavigationFilter_B.b_tau_l[i];
              b = jA + 1;
              knt = lastv + jA;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_o + d) - jA) - 21] *
                  c;
              }
            }

            jA += 20;
          }
        }
      }
    }

    b = PlaneNavigationFilter_B.iaii_o - i;
    for (lastv = PlaneNavigationFilter_B.iaii_o - 18; lastv <= b; lastv++) {
      Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_l[i];
    }

    Q[PlaneNavigationFilter_B.iaii_o - 20] = 1.0F -
      PlaneNavigationFilter_B.b_tau_l[i];
    for (PlaneNavigationFilter_B.ii_n = 0; PlaneNavigationFilter_B.ii_n < i;
         PlaneNavigationFilter_B.ii_n++) {
      Q[(PlaneNavigationFilter_B.iaii_o - PlaneNavigationFilter_B.ii_n) - 21] =
        0.0F;
    }
  }
}

real32_T rt_powf_snf(real32_T u0, real32_T u1)
{
  real32_T tmp;
  real32_T tmp_0;
  real32_T y;
  if (rtIsNaNF(u0) || rtIsNaNF(u1)) {
    y = (rtNaNF);
  } else {
    tmp = (real32_T)fabs(u0);
    tmp_0 = (real32_T)fabs(u1);
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
      y = (real32_T)sqrt(u0);
    } else if ((u0 < 0.0F) && (u1 > (real32_T)floor(u1))) {
      y = (rtNaNF);
    } else {
      y = (real32_T)pow(u0, u1);
    }
  }

  return y;
}

static real32_T PlaneNavigationFilte_xnrm2_dk0c(int32_T n, const real32_T x[19],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
  return y;
}

static void EKFCorrectorAdditive_getMeasure(real32_T Rs, const real32_T x[18],
  const real32_T S[324], real32_T *zEstimated, real32_T Pxy[18], real32_T *Sy,
  real32_T dHdx[18], real32_T *Rsqrt)
{
  int32_T c_i;
  int32_T i;
  int32_T knt;
  real32_T beta1;
  real32_T t2;
  real32_T t6;

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
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

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  PlaneNavigationFilter_B.M[18] = Rs;
  for (i = 0; i < 1; i++) {
    t2 = PlaneNavigationFilter_B.M[0];
    beta1 = PlaneNavigationFilte_xnrm2_dk0c(18, PlaneNavigationFilter_B.M, 2);
    if (beta1 != 0.0F) {
      beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.M[0], beta1);
      if (PlaneNavigationFilter_B.M[0] >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          for (c_i = 2; c_i < 20; c_i++) {
            PlaneNavigationFilter_B.M[c_i - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          t2 *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(t2, PlaneNavigationFilte_xnrm2_dk0c(18,
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

static void PlaneNavigationFi_trisolve_dk0c(real32_T A, real32_T B[18])
{
  int32_T b_j;
  real32_T B_0;

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  for (b_j = 0; b_j < 18; b_j++) {
    B_0 = B[b_j];
    if (B_0 != 0.0F) {
      B[b_j] = B_0 / A;
    }
  }

  /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
}

static real32_T PlaneNavigationFilt_xnrm2_dk0ct(int32_T n, const real32_T x[342],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S7>/MATLAB System' */
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  memset(&PlaneNavigationFilter_B.b_tau_d[0], 0, 18U * sizeof(real32_T));
  memcpy(&Q[0], &A[0], 342U * sizeof(real32_T));
  memset(&PlaneNavigationFilter_B.work_d[0], 0, 18U * sizeof(real32_T));
  for (PlaneNavigationFilter_B.iaii_n = 0; PlaneNavigationFilter_B.iaii_n < 18;
       PlaneNavigationFilter_B.iaii_n++) {
    PlaneNavigationFilter_B.ii_i = PlaneNavigationFilter_B.iaii_n * 19 +
      PlaneNavigationFilter_B.iaii_n;
    i = PlaneNavigationFilter_B.ii_i + 2;
    PlaneNavigationFilter_B.atmp_j = Q[PlaneNavigationFilter_B.ii_i];
    PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n] = 0.0F;
    beta1 = PlaneNavigationFilt_xnrm2_dk0ct(18 - PlaneNavigationFilter_B.iaii_n,
      Q, PlaneNavigationFilter_B.ii_i + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii_i];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b = PlaneNavigationFilter_B.ii_i - PlaneNavigationFilter_B.iaii_n;
          for (lastv = i; lastv <= b + 19; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp_j *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp_j,
                              PlaneNavigationFilt_xnrm2_dk0ct(18 -
          PlaneNavigationFilter_B.iaii_n, Q, PlaneNavigationFilter_B.ii_i + 2));
        if (PlaneNavigationFilter_B.atmp_j >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n] = (beta1
          - PlaneNavigationFilter_B.atmp_j) / beta1;
        PlaneNavigationFilter_B.atmp_j = 1.0F / (PlaneNavigationFilter_B.atmp_j
          - beta1);
        for (lastv = i; lastv <= b + 19; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_j;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp_j = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n] = (beta1
          - c) / beta1;
        PlaneNavigationFilter_B.atmp_j = 1.0F / (c - beta1);
        b = PlaneNavigationFilter_B.ii_i - PlaneNavigationFilter_B.iaii_n;
        for (lastv = i; lastv <= b + 19; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp_j;
        }

        PlaneNavigationFilter_B.atmp_j = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii_i] = PlaneNavigationFilter_B.atmp_j;
    if (PlaneNavigationFilter_B.iaii_n + 1 < 18) {
      Q[PlaneNavigationFilter_B.ii_i] = 1.0F;
      knt = PlaneNavigationFilter_B.ii_i + 20;
      if (PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n] !=
          0.0F) {
        lastv = 19 - PlaneNavigationFilter_B.iaii_n;
        i = PlaneNavigationFilter_B.ii_i - PlaneNavigationFilter_B.iaii_n;
        while ((lastv > 0) && (Q[i + 18] == 0.0F)) {
          lastv--;
          i--;
        }

        b = 17 - PlaneNavigationFilter_B.iaii_n;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 19 + PlaneNavigationFilter_B.ii_i;
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
            memset(&PlaneNavigationFilter_B.work_d[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (19 * lastc + PlaneNavigationFilter_B.ii_i) + 20;
          for (iac = knt; iac <= b; iac += 19) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii_i + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii_i) - 20, 19);
            PlaneNavigationFilter_B.work_d[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n] ==
              0.0F)) {
          jA = PlaneNavigationFilter_B.ii_i;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_d[iac];
            if (c != 0.0F) {
              c *=
                -PlaneNavigationFilter_B.b_tau_d[PlaneNavigationFilter_B.iaii_n];
              b = jA + 20;
              knt = (lastv + jA) + 19;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii_i + d) - jA) - 20] *
                  c;
              }
            }

            jA += 19;
          }
        }
      }

      Q[PlaneNavigationFilter_B.ii_i] = PlaneNavigationFilter_B.atmp_j;
    }
  }

  for (i = 0; i < 18; i++) {
    for (PlaneNavigationFilter_B.iaii_n = 0; PlaneNavigationFilter_B.iaii_n <= i;
         PlaneNavigationFilter_B.iaii_n++) {
      /* Start for MATLABSystem: '<S7>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii_n + 18 * i] = Q[19 * i +
        PlaneNavigationFilter_B.iaii_n];
    }

    for (PlaneNavigationFilter_B.iaii_n = i + 2; PlaneNavigationFilter_B.iaii_n <
         19; PlaneNavigationFilter_B.iaii_n++) {
      /* Start for MATLABSystem: '<S7>/MATLAB System' */
      R[(PlaneNavigationFilter_B.iaii_n + 18 * i) - 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    PlaneNavigationFilter_B.work_d[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S7>/MATLAB System' */
  for (i = 17; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii_n = (i * 19 + i) + 19;
    if (i + 1 < 18) {
      Q[PlaneNavigationFilter_B.iaii_n - 19] = 1.0F;
      knt = PlaneNavigationFilter_B.iaii_n + 1;
      if (PlaneNavigationFilter_B.b_tau_d[i] != 0.0F) {
        lastv = 19 - i;
        PlaneNavigationFilter_B.ii_i = (PlaneNavigationFilter_B.iaii_n - i) - 1;
        while ((lastv > 0) && (Q[PlaneNavigationFilter_B.ii_i] == 0.0F)) {
          lastv--;
          PlaneNavigationFilter_B.ii_i--;
        }

        b = 17 - i;
        exitg2 = false;
        while ((!exitg2) && (b > 0)) {
          lastc = (b - 1) * 19 + PlaneNavigationFilter_B.iaii_n;
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
            memset(&PlaneNavigationFilter_B.work_d[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (19 * lastc + PlaneNavigationFilter_B.iaii_n) + 1;
          for (iac = knt; iac <= b; iac += 19) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii_n + jA) - iac) - 19] * Q[jA
                - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii_n) - 1,
              19);
            PlaneNavigationFilter_B.work_d[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau_d[i] == 0.0F)) {
          jA = PlaneNavigationFilter_B.iaii_n;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work_d[iac];
            if (c != 0.0F) {
              c *= -PlaneNavigationFilter_B.b_tau_d[i];
              b = jA + 1;
              knt = lastv + jA;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii_n + d) - jA) - 20] *
                  c;
              }
            }

            jA += 19;
          }
        }
      }
    }

    b = PlaneNavigationFilter_B.iaii_n - i;
    for (lastv = PlaneNavigationFilter_B.iaii_n - 17; lastv <= b; lastv++) {
      Q[lastv - 1] *= -PlaneNavigationFilter_B.b_tau_d[i];
    }

    Q[PlaneNavigationFilter_B.iaii_n - 19] = 1.0F -
      PlaneNavigationFilter_B.b_tau_d[i];
    for (PlaneNavigationFilter_B.ii_i = 0; PlaneNavigationFilter_B.ii_i < i;
         PlaneNavigationFilter_B.ii_i++) {
      Q[(PlaneNavigationFilter_B.iaii_n - PlaneNavigationFilter_B.ii_i) - 20] =
        0.0F;
    }
  }
}

static real32_T PlaneNavigationFil_xnrm2_dk0ctr(int32_T n, const real32_T x[864],
  int32_T ix0)
{
  int32_T k;
  int32_T kend;
  real32_T absxk;
  real32_T scale;
  real32_T t;
  real32_T y;

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  y = 0.0F;
  if (n >= 1) {
    if (n == 1) {
      y = (real32_T)fabs(x[ix0 - 1]);
    } else {
      scale = 1.29246971E-26F;
      kend = ix0 + n;
      for (k = ix0; k < kend; k++) {
        absxk = (real32_T)fabs(x[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = y * t * t + 1.0F;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * (real32_T)sqrt(y);
    }
  }

  /* End of Start for MATLABSystem: '<S9>/MATLAB System' */
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
  real32_T beta1;
  real32_T c;
  boolean_T exitg2;

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
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
    beta1 = PlaneNavigationFil_xnrm2_dk0ctr(47 - PlaneNavigationFilter_B.iaii, Q,
      PlaneNavigationFilter_B.ii + 2);
    if (beta1 != 0.0F) {
      c = Q[PlaneNavigationFilter_B.ii];
      beta1 = rt_hypotf_snf(c, beta1);
      if (c >= 0.0F) {
        beta1 = -beta1;
      }

      if ((real32_T)fabs(beta1) < 9.86076132E-32F) {
        knt = -1;
        do {
          knt++;
          b = PlaneNavigationFilter_B.ii - PlaneNavigationFilter_B.iaii;
          for (lastv = i; lastv <= b + 48; lastv++) {
            Q[lastv - 1] *= 1.01412048E+31F;
          }

          beta1 *= 1.01412048E+31F;
          PlaneNavigationFilter_B.atmp *= 1.01412048E+31F;
        } while (((real32_T)fabs(beta1) < 9.86076132E-32F) && (knt + 1 < 20));

        beta1 = rt_hypotf_snf(PlaneNavigationFilter_B.atmp,
                              PlaneNavigationFil_xnrm2_dk0ctr(47 -
          PlaneNavigationFilter_B.iaii, Q, PlaneNavigationFilter_B.ii + 2));
        if (PlaneNavigationFilter_B.atmp >= 0.0F) {
          beta1 = -beta1;
        }

        PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] = (beta1 -
          PlaneNavigationFilter_B.atmp) / beta1;
        PlaneNavigationFilter_B.atmp = 1.0F / (PlaneNavigationFilter_B.atmp -
          beta1);
        for (lastv = i; lastv <= b + 48; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp;
        }

        for (lastv = 0; lastv <= knt; lastv++) {
          beta1 *= 9.86076132E-32F;
        }

        PlaneNavigationFilter_B.atmp = beta1;
      } else {
        PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] = (beta1 - c)
          / beta1;
        PlaneNavigationFilter_B.atmp = 1.0F / (c - beta1);
        b = PlaneNavigationFilter_B.ii - PlaneNavigationFilter_B.iaii;
        for (lastv = i; lastv <= b + 48; lastv++) {
          Q[lastv - 1] *= PlaneNavigationFilter_B.atmp;
        }

        PlaneNavigationFilter_B.atmp = beta1;
      }
    }

    Q[PlaneNavigationFilter_B.ii] = PlaneNavigationFilter_B.atmp;
    if (PlaneNavigationFilter_B.iaii + 1 < 18) {
      Q[PlaneNavigationFilter_B.ii] = 1.0F;
      knt = PlaneNavigationFilter_B.ii + 49;
      if (PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] != 0.0F) {
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
            memset(&PlaneNavigationFilter_B.work[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (48 * lastc + PlaneNavigationFilter_B.ii) + 49;
          for (iac = knt; iac <= b; iac += 48) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[(PlaneNavigationFilter_B.ii + jA) - iac] * Q[jA - 1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.ii) - 49, 48);
            PlaneNavigationFilter_B.work[jA] += c;
          }
        }

        if (!(-PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii] ==
              0.0F)) {
          jA = PlaneNavigationFilter_B.ii;
          for (iac = 0; iac <= lastc; iac++) {
            c = PlaneNavigationFilter_B.work[iac];
            if (c != 0.0F) {
              c *= -PlaneNavigationFilter_B.b_tau[PlaneNavigationFilter_B.iaii];
              b = jA + 49;
              knt = (lastv + jA) + 48;
              for (d = b; d <= knt; d++) {
                Q[d - 1] += Q[((PlaneNavigationFilter_B.ii + d) - jA) - 49] * c;
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
      /* Start for MATLABSystem: '<S9>/MATLAB System' */
      R[PlaneNavigationFilter_B.iaii + 18 * i] = Q[48 * i +
        PlaneNavigationFilter_B.iaii];
    }

    for (PlaneNavigationFilter_B.iaii = i + 2; PlaneNavigationFilter_B.iaii < 19;
         PlaneNavigationFilter_B.iaii++) {
      /* Start for MATLABSystem: '<S9>/MATLAB System' */
      R[(PlaneNavigationFilter_B.iaii + 18 * i) - 1] = 0.0F;
    }

    /* Start for MATLABSystem: '<S9>/MATLAB System' */
    PlaneNavigationFilter_B.work[i] = 0.0F;
  }

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  for (i = 17; i >= 0; i--) {
    PlaneNavigationFilter_B.iaii = (i * 48 + i) + 48;
    if (i + 1 < 18) {
      Q[PlaneNavigationFilter_B.iaii - 48] = 1.0F;
      knt = PlaneNavigationFilter_B.iaii + 1;
      if (PlaneNavigationFilter_B.b_tau[i] != 0.0F) {
        lastv = 48 - i;
        PlaneNavigationFilter_B.ii = (PlaneNavigationFilter_B.iaii - i) - 1;
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
            memset(&PlaneNavigationFilter_B.work[0], 0, (uint32_T)(lastc + 1) *
                   sizeof(real32_T));
          }

          b = (48 * lastc + PlaneNavigationFilter_B.iaii) + 1;
          for (iac = knt; iac <= b; iac += 48) {
            c = 0.0F;
            d = iac + lastv;
            for (jA = iac; jA < d; jA++) {
              c += Q[((PlaneNavigationFilter_B.iaii + jA) - iac) - 48] * Q[jA -
                1];
            }

            jA = div_nde_s32_floor((iac - PlaneNavigationFilter_B.iaii) - 1, 48);
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
                Q[d - 1] += Q[((PlaneNavigationFilter_B.iaii + d) - jA) - 49] *
                  c;
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
      Q[(PlaneNavigationFilter_B.iaii - PlaneNavigationFilter_B.ii) - 49] = 0.0F;
    }
  }
}

static void PlaneN_nav_state_trans_dt_100Hz(const real32_T state[18], const
  real32_T w[30], const real32_T u[6], real32_T x_next[18])
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  /*  h = state(8); */
  /*  p0 = state(15); */
  /*  T0 = state(16); */
  /*  xy_local = state(17:18); */
  /*  Rotation matrix for intrinsic XYZ Euler angles: R = Rx(roll)*Ry(pitch)*Rz(yaw) */
  cr_tmp = (real32_T)cos(state[3]);
  sr_tmp = (real32_T)sin(state[3]);
  cp_tmp = (real32_T)cos(state[4]);
  sp_tmp = (real32_T)sin(state[4]);
  cy = (real32_T)cos(state[5]);
  sy = (real32_T)sin(state[5]);
  PlaneNavigationFilter_B.R_ib[0] = cp_tmp * cy;
  PlaneNavigationFilter_B.R_ib[3] = -cp_tmp * sy;
  PlaneNavigationFilter_B.R_ib[6] = sp_tmp;
  PlaneNavigationFilter_B.R_ib[1] = sr_tmp * sp_tmp * cy + cr_tmp * sy;
  PlaneNavigationFilter_B.R_ib[4] = -sr_tmp * sp_tmp * sy + cr_tmp * cy;
  PlaneNavigationFilter_B.R_ib[7] = -sr_tmp * cp_tmp;
  PlaneNavigationFilter_B.R_ib[2] = -cr_tmp * sp_tmp * cy + sr_tmp * sy;
  PlaneNavigationFilter_B.R_ib[5] = cr_tmp * sp_tmp * sy + sr_tmp * cy;

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  cy = cr_tmp * cp_tmp;
  PlaneNavigationFilter_B.R_ib[8] = cy;

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
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
    PlaneNavigationFilter_B.fv5[i] = (PlaneNavigationFilter_B.fv3[i + 3] *
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
  x_next[3] = ((state_0 * (real32_T)tan(state[4]) + sy) + w[3]) * 0.01F + state
    [3];
  x_next[4] = ((omega_corr_idx_1 * cr_tmp - omega_corr_idx_2 * sr_tmp) + w[4]) *
    0.01F + state[4];
  x_next[5] = (1.0F / cp_tmp * state_0 + w[5]) * 0.01F + state[5];
  x_next[6] = w[6] * 0.01F + state[6];
  x_next[7] = (-((state[0] * PlaneNavigationFilter_B.R_ib[2] + state[1] *
                  PlaneNavigationFilter_B.R_ib[5]) + state[2] * cy) + w[7]) *
    0.01F + state[7];
  x_next[0] = (((((state[6] * -sp_tmp + PlaneNavigationFilter_B.fv5[0]) + u[0])
                 + w[18]) - state[8]) + w[0]) * 0.01F + state[0];
  x_next[8] = (w[8] + w[24]) * 0.01F + state[8];
  x_next[11] = (w[11] + w[27]) * 0.01F + state[11];
  x_next[1] = (((((sr_tmp * cp_tmp * state[6] + PlaneNavigationFilter_B.fv5[1])
                  + u[1]) + w[19]) - state[9]) + w[1]) * 0.01F + state[1];
  x_next[9] = (w[9] + w[25]) * 0.01F + state[9];
  x_next[12] = (w[12] + w[28]) * 0.01F + state[12];
  x_next[2] = (((((cy * state[6] + PlaneNavigationFilter_B.fv5[2]) + u[2]) + w
                 [20]) - state[10]) + w[2]) * 0.01F + state[2];
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
  boolean_T rtb_INIT_MEAS_EN;
  static const int8_T A_1[126] = { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0 };

  static const int8_T A_2[36] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1 };

  static const int8_T tmp[126] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0 };

  static const int8_T tmp_0[36] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

  static const real32_T tmp_1[164] = { 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F };

  static const real32_T tmp_2[160] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.01F };

  static const real32_T tmp_3[47] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.01F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.01F, 0.0F,
    0.0F, 0.0F, 0.0F };

  /* Outputs for Enabled SubSystem: '<S2>/Correct1' incorporates:
   *  EnablePort: '<S5>/Enable'
   */
  /* Outputs for Atomic SubSystem: '<S2>/Output' */
  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
       PlaneNavigationFilter_B.i++) {
    for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
         18; PlaneNavigationFilter_B.coffset++) {
      /* MATLAB Function: '<S8>/MATLAB Function' incorporates:
       *  DataStoreRead: '<S8>/Data Store Read1'
       *  MATLABSystem: '<S5>/MATLAB System'
       * */
      PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
        PlaneNavigationFilter_B.i] = PlaneNavigationFilter_DW.P[18 *
        PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
    }
  }

  /* End of Outputs for SubSystem: '<S2>/Correct1' */

  /* MATLAB Function: '<S8>/MATLAB Function' incorporates:
   *  DataStoreRead: '<S8>/Data Store Read1'
   */
  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
       PlaneNavigationFilter_B.i++) {
    for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
         18; PlaneNavigationFilter_B.coffset++) {
      PlaneNavigationFilter_B.t8 = 0.0F;
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
          PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
          PlaneNavigationFilter_B.A[18 * PlaneNavigationFilter_B.i +
          PlaneNavigationFilter_B.b_i];
      }

      PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.coffset + 18 *
        PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
    }
  }

  /* End of Outputs for SubSystem: '<S2>/Output' */

  /* BusCreator generated from: '<Root>/EkfState' incorporates:
   *  DataStoreRead: '<S8>/Data Store Read'
   *  Outport: '<Root>/EkfState'
   */
  PlaneNavigationFilter_Y.EkfState.g = PlaneNavigationFilter_DW.x[6];
  PlaneNavigationFilter_Y.EkfState.h = PlaneNavigationFilter_DW.x[7];
  PlaneNavigationFilter_Y.EkfState.vel_body[0] = PlaneNavigationFilter_DW.x[0];
  PlaneNavigationFilter_Y.EkfState.rpy[0] = PlaneNavigationFilter_DW.x[3];
  PlaneNavigationFilter_Y.EkfState.acc_bias[0] = PlaneNavigationFilter_DW.x[8];
  PlaneNavigationFilter_Y.EkfState.gyro_bias[0] = PlaneNavigationFilter_DW.x[11];
  PlaneNavigationFilter_Y.EkfState.vel_body[1] = PlaneNavigationFilter_DW.x[1];
  PlaneNavigationFilter_Y.EkfState.rpy[1] = PlaneNavigationFilter_DW.x[4];
  PlaneNavigationFilter_Y.EkfState.acc_bias[1] = PlaneNavigationFilter_DW.x[9];
  PlaneNavigationFilter_Y.EkfState.gyro_bias[1] = PlaneNavigationFilter_DW.x[12];
  PlaneNavigationFilter_Y.EkfState.vel_body[2] = PlaneNavigationFilter_DW.x[2];
  PlaneNavigationFilter_Y.EkfState.rpy[2] = PlaneNavigationFilter_DW.x[5];
  PlaneNavigationFilter_Y.EkfState.acc_bias[2] = PlaneNavigationFilter_DW.x[10];
  PlaneNavigationFilter_Y.EkfState.gyro_bias[2] = PlaneNavigationFilter_DW.x[13];
  PlaneNavigationFilter_Y.EkfState.P0 = PlaneNavigationFilter_DW.x[14];
  PlaneNavigationFilter_Y.EkfState.T0 = PlaneNavigationFilter_DW.x[15];
  PlaneNavigationFilter_Y.EkfState.xy[0] = PlaneNavigationFilter_DW.x[16];
  PlaneNavigationFilter_Y.EkfState.xy[1] = PlaneNavigationFilter_DW.x[17];
  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
       PlaneNavigationFilter_B.i++) {
    /* Sqrt: '<Root>/Sqrt' incorporates:
     *  S-Function (sdspdiag2): '<Root>/Extract Diagonal'
     */
    PlaneNavigationFilter_B.ekf_state_std[PlaneNavigationFilter_B.i] = (real32_T)
      sqrt(PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.i * 19]);
  }

  /* BusCreator generated from: '<Root>/EkfStateStd' incorporates:
   *  Outport: '<Root>/EkfStateStd'
   */
  PlaneNavigationFilter_Y.EkfStateStd.g = PlaneNavigationFilter_B.ekf_state_std
    [6];
  PlaneNavigationFilter_Y.EkfStateStd.h = PlaneNavigationFilter_B.ekf_state_std
    [7];
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
   *  Delay: '<Root>/Delay'
   *  Inport: '<Root>/ARM_CMD'
   */
  if (PlaneNavigationFilter_DW.is_active_c3_PlaneNavigationFil == 0) {
    PlaneNavigationFilter_DW.is_active_c3_PlaneNavigationFil = 1U;
    PlaneNavigationFilter_DW.is_c3_PlaneNavigationFilter =
      PlaneNavigationFilter_IN_IDLE;
    rtb_INIT_MEAS_EN = false;
  } else {
    switch (PlaneNavigationFilter_DW.is_c3_PlaneNavigationFilter) {
     case PlaneNavigationFi_IN_EKF_ACTIVE:
      rtb_INIT_MEAS_EN = false;
      if (!PlaneNavigationFi_check_ekf_std(PlaneNavigationFilter_DW.Delay_DSTATE))
      {
        PlaneNavigationFilter_DW.durationCounter_2 = 0U;
      }
      break;

     case PlaneNavigationFilt_IN_EKF_INIT:
      if (!PlaneNavigationFi_check_ekf_std(PlaneNavigationFilter_DW.Delay_DSTATE))
      {
        PlaneNavigationFilter_DW.durationCounter_1 = 0U;
      }

      if (PlaneNavigationFilter_DW.durationCounter_1 > 100U) {
        PlaneNavigationFilter_DW.durationCounter_2 = 0U;
        PlaneNavigationFilter_DW.is_c3_PlaneNavigationFilter =
          PlaneNavigationFi_IN_EKF_ACTIVE;
        rtb_INIT_MEAS_EN = false;
      } else {
        rtb_INIT_MEAS_EN = true;
      }
      break;

     default:
      /* case IN_IDLE: */
      if (PlaneNavigationFilter_U.ARM_CMD != 0.0) {
        PlaneNavigationFilter_DW.durationCounter_1 = 0U;
        PlaneNavigationFilter_DW.is_c3_PlaneNavigationFilter =
          PlaneNavigationFilt_IN_EKF_INIT;
        rtb_INIT_MEAS_EN = true;
      } else {
        rtb_INIT_MEAS_EN = false;
      }
      break;
    }
  }

  if (PlaneNavigationFi_check_ekf_std(PlaneNavigationFilter_DW.Delay_DSTATE)) {
    PlaneNavigationFilter_DW.durationCounter_2++;
    PlaneNavigationFilter_DW.durationCounter_1++;
  } else {
    PlaneNavigationFilter_DW.durationCounter_2 = 0U;
    PlaneNavigationFilter_DW.durationCounter_1 = 0U;
  }

  /* End of Chart: '<Root>/EKF Logic' */

  /* Outputs for Enabled SubSystem: '<S2>/Correct1' incorporates:
   *  EnablePort: '<S5>/Enable'
   */
  if (rtb_INIT_MEAS_EN) {
    /* MATLABSystem: '<S5>/MATLAB System' incorporates:
     *  Constant: '<S2>/BlockOrdering'
     */
    PlaneNavigationFilter_B.MATLABSystem_o3_f =
      PlaneNavigationFilter_P.BlockOrdering_Value;

    /* MATLABSystem: '<S5>/MATLAB System' incorporates:
     *  DataStoreRead: '<S5>/Data Store ReadP'
     */
    /*  vx, vy, vz */
    /*  rpy */
    /*  h */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
      memset(&PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.coffset + 1],
             0, 18U * sizeof(real32_T));
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.bkj_o = A_1[PlaneNavigationFilter_B.b_i * 7 +
          PlaneNavigationFilter_B.i];
        for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
             PlaneNavigationFilter_B.c_i++) {
          PlaneNavigationFilter_B.K_tmp = (PlaneNavigationFilter_B.coffset +
            PlaneNavigationFilter_B.c_i) + 1;
          PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.K_tmp] +=
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
            PlaneNavigationFilter_B.b_i] * (real32_T)
            PlaneNavigationFilter_B.bkj_o;
        }
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' incorporates:
       *  Constant: '<S2>/R1'
       * */
      memcpy(&PlaneNavigationFilter_B.K[PlaneNavigationFilter_B.i * 25],
             &PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i * 18], 18U *
             sizeof(real32_T));
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           7; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.K[(PlaneNavigationFilter_B.coffset + 25 *
          PlaneNavigationFilter_B.i) + 18] = PlaneNavigationFilter_P.R1_Value[7 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }
    }

    /* MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_qr(PlaneNavigationFilter_B.K,
      PlaneNavigationFilter_B.a__1_me, PlaneNavigationFilter_B.R);
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           7; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.Sy[PlaneNavigationFilter_B.coffset + 7 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.R[7 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        /* Start for MATLABSystem: '<S5>/MATLAB System' incorporates:
         *  DataStoreRead: '<S5>/Data Store ReadP'
         */
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
            PlaneNavigationFilter_B.A[18 * PlaneNavigationFilter_B.i +
            PlaneNavigationFilter_B.b_i];
        }

        PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        /* Start for MATLABSystem: '<S5>/MATLAB System' */
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.P[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
            (real32_T)tmp[18 * PlaneNavigationFilter_B.i +
            PlaneNavigationFilter_B.b_i];
        }

        PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i + 7 *
          PlaneNavigationFilter_B.coffset] = PlaneNavigationFilter_B.t8;
      }
    }

    /* MATLABSystem: '<S5>/MATLAB System' */
    memcpy(&PlaneNavigationFilter_B.C[0], &PlaneNavigationFilter_B.K_n[0], 126U *
           sizeof(real32_T));

    /* Start for MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_trisolve(PlaneNavigationFilter_B.Sy,
      PlaneNavigationFilter_B.C);
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           7; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.R[PlaneNavigationFilter_B.coffset + 7 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.Sy[7 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }
    }

    PlaneNavigationFilte_trisolve_d(PlaneNavigationFilter_B.R,
      PlaneNavigationFilter_B.C);

    /* MATLABSystem: '<S5>/MATLAB System' */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.C[7 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 126;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      PlaneNavigationFilter_B.C[PlaneNavigationFilter_B.i] =
        -PlaneNavigationFilter_B.K_n[PlaneNavigationFilter_B.i];
    }

    /* MATLABSystem: '<S5>/MATLAB System' incorporates:
     *  DataStoreRead: '<S5>/Data Store ReadP'
     */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 7;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.C[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
            (real32_T)A_1[7 * PlaneNavigationFilter_B.i +
            PlaneNavigationFilter_B.b_i];
        }

        PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.t8;
      }
    }

    for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
         PlaneNavigationFilter_B.b_i++) {
      PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_i +
        PlaneNavigationFilter_B.b_i;
      PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
      memset(&PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.coffset + 1],
             0, 18U * sizeof(real32_T));
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.bkj =
          PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_i * 18 +
          PlaneNavigationFilter_B.i];
        for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
             PlaneNavigationFilter_B.c_i++) {
          PlaneNavigationFilter_B.bkj_o = (PlaneNavigationFilter_B.coffset +
            PlaneNavigationFilter_B.c_i) + 1;
          PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.bkj_o] +=
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
            PlaneNavigationFilter_B.b_i] * PlaneNavigationFilter_B.bkj;
        }
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 7;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        /* Start for MATLABSystem: '<S5>/MATLAB System' incorporates:
         *  Constant: '<S2>/R1'
         * */
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 7;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.K_n[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
            PlaneNavigationFilter_P.R1_Value[7 * PlaneNavigationFilter_B.i +
            PlaneNavigationFilter_B.b_i];
        }

        PlaneNavigationFilter_B.C[PlaneNavigationFilter_B.i + 7 *
          PlaneNavigationFilter_B.coffset] = PlaneNavigationFilter_B.t8;
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S5>/MATLAB System' */
      memcpy(&PlaneNavigationFilter_B.y_c[PlaneNavigationFilter_B.i * 25],
             &PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.i * 18], 18U *
             sizeof(real32_T));
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           7; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.y_c[(PlaneNavigationFilter_B.coffset + 25 *
          PlaneNavigationFilter_B.i) + 18] = PlaneNavigationFilter_B.C[7 *
          PlaneNavigationFilter_B.i + PlaneNavigationFilter_B.coffset];
      }
    }

    /* MATLABSystem: '<S5>/MATLAB System' */
    PlaneNavigationFilter_qr_d(PlaneNavigationFilter_B.y_c,
      PlaneNavigationFilter_B.a__1_m, PlaneNavigationFilter_B.A);

    /* Start for MATLABSystem: '<S5>/MATLAB System' incorporates:
     *  Constant: '<Root>/Constant1'
     *  DataStoreRead: '<S5>/Data Store ReadX'
     * */
    PlaneNavigationFilter_B.fv4[0] = PlaneNavigationFilter_P.Constant1_Value[0]
      - PlaneNavigationFilter_DW.x[0];
    PlaneNavigationFilter_B.fv4[3] = PlaneNavigationFilter_P.Constant1_Value[3]
      - PlaneNavigationFilter_DW.x[3];
    PlaneNavigationFilter_B.fv4[1] = PlaneNavigationFilter_P.Constant1_Value[1]
      - PlaneNavigationFilter_DW.x[1];
    PlaneNavigationFilter_B.fv4[4] = PlaneNavigationFilter_P.Constant1_Value[4]
      - PlaneNavigationFilter_DW.x[4];
    PlaneNavigationFilter_B.fv4[2] = PlaneNavigationFilter_P.Constant1_Value[2]
      - PlaneNavigationFilter_DW.x[2];
    PlaneNavigationFilter_B.fv4[5] = PlaneNavigationFilter_P.Constant1_Value[5]
      - PlaneNavigationFilter_DW.x[5];
    PlaneNavigationFilter_B.fv4[6] = PlaneNavigationFilter_P.Constant1_Value[6]
      - PlaneNavigationFilter_DW.x[7];
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* DataStoreWrite: '<S5>/Data Store WriteP' incorporates:
       *  MATLABSystem: '<S5>/MATLAB System'
       * */
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.A[18 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }

      /* End of DataStoreWrite: '<S5>/Data Store WriteP' */

      /* DataStoreWrite: '<S5>/Data Store WriteX' incorporates:
       *  DataStoreRead: '<S5>/Data Store ReadX'
       *  MATLABSystem: '<S5>/MATLAB System'
       * */
      PlaneNavigationFilter_B.t8 = 0.0F;
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           7; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.K_n[18 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i] *
          PlaneNavigationFilter_B.fv4[PlaneNavigationFilter_B.coffset];
      }

      PlaneNavigationFilter_DW.x[PlaneNavigationFilter_B.i] +=
        PlaneNavigationFilter_B.t8;

      /* End of DataStoreWrite: '<S5>/Data Store WriteX' */
    }
  }

  /* End of Outputs for SubSystem: '<S2>/Correct1' */

  /* Outputs for Enabled SubSystem: '<S2>/Correct2' incorporates:
   *  EnablePort: '<S6>/Enable'
   */
  /* Inport: '<Root>/GPS_POS_EN' */
  if (PlaneNavigationFilter_U.GPS_POS_EN) {
    /* MATLABSystem: '<S6>/MATLAB System' incorporates:
     *  DataStoreRead: '<S6>/Data Store ReadP'
     */
    /*  GPS measurement function: returns [north_m; east_m] in the local NED frame. */
    /*  */
    /*    state(17:18) is already the North/East displacement in metres relative */
    /*    to the reference origin.  Conversion from raw lat/lon to metres happens */
    /*    outside the EKF (preprocessing block) using latlon2xy. */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
      memset(&PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 1],
             0, 18U * sizeof(real32_T));
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.bkj_o = A_2[(PlaneNavigationFilter_B.b_i << 1) +
          PlaneNavigationFilter_B.i];
        for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
             PlaneNavigationFilter_B.c_i++) {
          PlaneNavigationFilter_B.K_tmp = (PlaneNavigationFilter_B.coffset +
            PlaneNavigationFilter_B.c_i) + 1;
          PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.K_tmp] +=
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
            PlaneNavigationFilter_B.b_i] * (real32_T)
            PlaneNavigationFilter_B.bkj_o;
        }
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
       *  Constant: '<S2>/R2'
       * */
      memcpy(&PlaneNavigationFilter_B.K_l[PlaneNavigationFilter_B.i * 20],
             &PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i * 18], 18U *
             sizeof(real32_T));
      PlaneNavigationFilter_B.K_l[20 * PlaneNavigationFilter_B.i + 18] =
        PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.i];
      PlaneNavigationFilter_B.K_l[20 * PlaneNavigationFilter_B.i + 19] =
        PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.i + 2];
    }

    /* MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilter_qr_dk(PlaneNavigationFilter_B.K_l,
      PlaneNavigationFilter_B.a__1_p, PlaneNavigationFilter_B.R_d);
    PlaneNavigationFilter_B.Sy_b[0] = PlaneNavigationFilter_B.R_d[0];
    PlaneNavigationFilter_B.Sy_b[1] = PlaneNavigationFilter_B.R_d[2];
    PlaneNavigationFilter_B.Sy_b[2] = PlaneNavigationFilter_B.R_d[1];
    PlaneNavigationFilter_B.Sy_b[3] = PlaneNavigationFilter_B.R_d[3];
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
         *  DataStoreRead: '<S6>/Data Store ReadP'
         */
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_DW.P[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.i] *
            PlaneNavigationFilter_DW.P[18 * PlaneNavigationFilter_B.b_i +
            PlaneNavigationFilter_B.coffset];
        }

        PlaneNavigationFilter_B.P[PlaneNavigationFilter_B.i + 18 *
          PlaneNavigationFilter_B.coffset] = PlaneNavigationFilter_B.t8;
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        /* Start for MATLABSystem: '<S6>/MATLAB System' */
        PlaneNavigationFilter_B.t8 = 0.0F;
        for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
             PlaneNavigationFilter_B.b_i++) {
          PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.P[18 *
            PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
            (real32_T)tmp_0[18 * PlaneNavigationFilter_B.i +
            PlaneNavigationFilter_B.b_i];
        }

        PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i +
          (PlaneNavigationFilter_B.coffset << 1)] = PlaneNavigationFilter_B.t8;
      }
    }

    /* MATLABSystem: '<S6>/MATLAB System' */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
      PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset] =
        PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset];
      PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset + 1] =
        PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 1];
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilt_trisolve_dk(PlaneNavigationFilter_B.Sy_b,
      PlaneNavigationFilter_B.C_d);

    /* MATLABSystem: '<S6>/MATLAB System' */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
      PlaneNavigationFilter_B.C_g[PlaneNavigationFilter_B.coffset] =
        PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset];
      PlaneNavigationFilter_B.C_g[PlaneNavigationFilter_B.coffset + 1] =
        PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset + 1];
    }

    /* Start for MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilter_B.Sy_b[0] = PlaneNavigationFilter_B.R_d[0];
    PlaneNavigationFilter_B.Sy_b[1] = PlaneNavigationFilter_B.R_d[1];
    PlaneNavigationFilter_B.Sy_b[2] = PlaneNavigationFilter_B.R_d[2];
    PlaneNavigationFilter_B.Sy_b[3] = PlaneNavigationFilter_B.R_d[3];
    PlaneNavigationFil_trisolve_dk0(PlaneNavigationFilter_B.Sy_b,
      PlaneNavigationFilter_B.C_g);

    /* MATLABSystem: '<S6>/MATLAB System' */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.C_g
          [(PlaneNavigationFilter_B.coffset << 1) + PlaneNavigationFilter_B.i];
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 36;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.i] =
        -PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.i];
    }

    /* MATLABSystem: '<S6>/MATLAB System' incorporates:
     *  DataStoreRead: '<S6>/Data Store ReadP'
     */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
      A = A_2[PlaneNavigationFilter_B.coffset + 1];
      A_0 = A_2[PlaneNavigationFilter_B.coffset];
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] =
          PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset + 18] *
          (real32_T)A + (real32_T)A_0 *
          PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.coffset];
      }
    }

    for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
         PlaneNavigationFilter_B.b_i++) {
      PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_i +
        PlaneNavigationFilter_B.b_i;
      PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
      memset(&PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.coffset + 1],
             0, 18U * sizeof(real32_T));
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.bkj =
          PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_i * 18 +
          PlaneNavigationFilter_B.i];
        for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
             PlaneNavigationFilter_B.c_i++) {
          PlaneNavigationFilter_B.bkj_o = (PlaneNavigationFilter_B.coffset +
            PlaneNavigationFilter_B.c_i) + 1;
          PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.bkj_o] +=
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
            PlaneNavigationFilter_B.b_i] * PlaneNavigationFilter_B.bkj;
        }
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 2;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
       *  Constant: '<S2>/R2'
       * */
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i << 1;
      PlaneNavigationFilter_B.t8 =
        PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.coffset + 1];
      PlaneNavigationFilter_B.t25 =
        PlaneNavigationFilter_P.R2_Value[PlaneNavigationFilter_B.coffset];
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.i +
          (PlaneNavigationFilter_B.coffset << 1)] =
          PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset + 18] *
          PlaneNavigationFilter_B.t8 + PlaneNavigationFilter_B.t25 *
          PlaneNavigationFilter_B.K_j[PlaneNavigationFilter_B.coffset];
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S6>/MATLAB System' */
      memcpy(&PlaneNavigationFilter_B.y_cx[PlaneNavigationFilter_B.i * 20],
             &PlaneNavigationFilter_B.y_f[PlaneNavigationFilter_B.i * 18], 18U *
             sizeof(real32_T));
      PlaneNavigationFilter_B.bkj_o = PlaneNavigationFilter_B.i << 1;
      PlaneNavigationFilter_B.y_cx[20 * PlaneNavigationFilter_B.i + 18] =
        PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.bkj_o];
      PlaneNavigationFilter_B.y_cx[20 * PlaneNavigationFilter_B.i + 19] =
        PlaneNavigationFilter_B.C_d[PlaneNavigationFilter_B.bkj_o + 1];
    }

    /* MATLABSystem: '<S6>/MATLAB System' */
    PlaneNavigationFilter_qr_dk0(PlaneNavigationFilter_B.y_cx,
      PlaneNavigationFilter_B.a__1_k, PlaneNavigationFilter_B.A);

    /* Start for MATLABSystem: '<S6>/MATLAB System' incorporates:
     *  DataStoreRead: '<S6>/Data Store ReadX'
     *  Inport: '<Root>/xy_pos'
     */
    PlaneNavigationFilter_B.t8 = PlaneNavigationFilter_U.xy_pos[0] -
      PlaneNavigationFilter_DW.x[16];
    PlaneNavigationFilter_B.t25 = PlaneNavigationFilter_U.xy_pos[1] -
      PlaneNavigationFilter_DW.x[17];
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* DataStoreWrite: '<S6>/Data Store WriteP' incorporates:
       *  MATLABSystem: '<S6>/MATLAB System'
       * */
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.A[18 *
          PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
      }

      /* End of DataStoreWrite: '<S6>/Data Store WriteP' */

      /* DataStoreWrite: '<S6>/Data Store WriteX' incorporates:
       *  DataStoreRead: '<S6>/Data Store ReadX'
       *  MATLABSystem: '<S6>/MATLAB System'
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
   *  EnablePort: '<S7>/Enable'
   */
  /* Inport: '<Root>/BARO_EN' */
  if (PlaneNavigationFilter_U.BARO_EN) {
    /* MATLABSystem: '<S7>/MATLAB System' incorporates:
     *  Constant: '<S2>/R3'
     *  DataStoreRead: '<S7>/Data Store ReadP'
     *  DataStoreRead: '<S7>/Data Store ReadX'
     *  Inport: '<Root>/baro_pressure'
     */
    EKFCorrectorAdditive_getMeasure(PlaneNavigationFilter_P.R3_Value,
      PlaneNavigationFilter_DW.x, PlaneNavigationFilter_DW.P,
      &PlaneNavigationFilter_B.residue, PlaneNavigationFilter_B.Pxy,
      &PlaneNavigationFilter_B.t25, PlaneNavigationFilter_B.dHdx,
      &PlaneNavigationFilter_B.t8);
    PlaneNavigationFilter_B.residue = PlaneNavigationFilter_U.baro_pressure -
      PlaneNavigationFilter_B.residue;
    memcpy(&PlaneNavigationFilter_B.C_l[0], &PlaneNavigationFilter_B.Pxy[0], 18U
           * sizeof(real32_T));

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    PlaneNavigationFi_trisolve_dk0c(PlaneNavigationFilter_B.t25,
      PlaneNavigationFilter_B.C_l);

    /* MATLABSystem: '<S7>/MATLAB System' */
    memcpy(&PlaneNavigationFilter_B.Pxy[0], &PlaneNavigationFilter_B.C_l[0], 18U
           * sizeof(real32_T));

    /* Start for MATLABSystem: '<S7>/MATLAB System' */
    PlaneNavigationFi_trisolve_dk0c(PlaneNavigationFilter_B.t25,
      PlaneNavigationFilter_B.Pxy);
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.C_l[PlaneNavigationFilter_B.i] =
        -PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i];
    }

    /* MATLABSystem: '<S7>/MATLAB System' incorporates:
     *  DataStoreRead: '<S7>/Data Store ReadP'
     */
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
        PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset + 18 *
          PlaneNavigationFilter_B.i] =
          PlaneNavigationFilter_B.C_l[PlaneNavigationFilter_B.coffset] *
          PlaneNavigationFilter_B.dHdx[PlaneNavigationFilter_B.i];
      }
    }

    for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
         PlaneNavigationFilter_B.b_i++) {
      PlaneNavigationFilter_B.coffset = 18 * PlaneNavigationFilter_B.b_i +
        PlaneNavigationFilter_B.b_i;
      PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.coffset]++;
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      PlaneNavigationFilter_B.coffset = PlaneNavigationFilter_B.i * 18 - 1;
      memset(&PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.coffset + 1],
             0, 18U * sizeof(real32_T));
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.bkj =
          PlaneNavigationFilter_B.A[PlaneNavigationFilter_B.b_i * 18 +
          PlaneNavigationFilter_B.i];
        for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
             PlaneNavigationFilter_B.c_i++) {
          PlaneNavigationFilter_B.bkj_o = (PlaneNavigationFilter_B.coffset +
            PlaneNavigationFilter_B.c_i) + 1;
          PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.bkj_o] +=
            PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
            PlaneNavigationFilter_B.b_i] * PlaneNavigationFilter_B.bkj;
        }
      }
    }

    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* Start for MATLABSystem: '<S7>/MATLAB System' */
      memcpy(&PlaneNavigationFilter_B.y_p[PlaneNavigationFilter_B.i * 19],
             &PlaneNavigationFilter_B.y_g[PlaneNavigationFilter_B.i * 18], 18U *
             sizeof(real32_T));
      PlaneNavigationFilter_B.y_p[19 * PlaneNavigationFilter_B.i + 18] =
        PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i] *
        PlaneNavigationFilter_B.t8;
    }

    /* MATLABSystem: '<S7>/MATLAB System' */
    PlaneNavigationFilter_qr_dk0c(PlaneNavigationFilter_B.y_p,
      PlaneNavigationFilter_B.a__1_b, PlaneNavigationFilter_B.A);
    for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
         PlaneNavigationFilter_B.i++) {
      /* DataStoreWrite: '<S7>/Data Store WriteP' incorporates:
       *  MATLABSystem: '<S7>/MATLAB System'
       * */
      for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
           18; PlaneNavigationFilter_B.coffset++) {
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
        PlaneNavigationFilter_B.Pxy[PlaneNavigationFilter_B.i] *
        PlaneNavigationFilter_B.residue;
    }
  }

  /* End of Inport: '<Root>/BARO_EN' */
  /* End of Outputs for SubSystem: '<S2>/Correct3' */

  /* Outputs for Atomic SubSystem: '<S2>/Predict' */
  /* SignalConversion generated from: '<S9>/MATLAB System' incorporates:
   *  Inport: '<Root>/acc_b'
   *  Inport: '<Root>/omega_ib'
   */
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[0] =
    PlaneNavigationFilter_U.acc_b[0];
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[3] =
    PlaneNavigationFilter_U.omega_ib[0];
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[1] =
    PlaneNavigationFilter_U.acc_b[1];
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[4] =
    PlaneNavigationFilter_U.omega_ib[1];
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[2] =
    PlaneNavigationFilter_U.acc_b[2];
  PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB[5] =
    PlaneNavigationFilter_U.omega_ib[2];

  /* MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreRead: '<S9>/Data Store ReadX'
   *  Inport: '<Root>/omega_ib'
   *  SignalConversion generated from: '<S9>/MATLAB System'
   */
  /* StateTrans100HzJacSym */
  /*     [Jx,Jw] = StateTrans100HzJacSym(IN1,IN2,IN3) */
  /*     This function was generated by the Symbolic Math Toolbox version 24.2. */
  /*     10-Apr-2026 23:03:41 */
  PlaneNavigationFilter_B.bkj = (real32_T)cos(PlaneNavigationFilter_DW.x[3]);
  PlaneNavigationFilter_B.t3 = (real32_T)cos(PlaneNavigationFilter_DW.x[4]);
  PlaneNavigationFilter_B.t4 = (real32_T)cos(PlaneNavigationFilter_DW.x[5]);
  PlaneNavigationFilter_B.t5 = (real32_T)sin(PlaneNavigationFilter_DW.x[3]);
  PlaneNavigationFilter_B.t6 = (real32_T)sin(PlaneNavigationFilter_DW.x[4]);
  PlaneNavigationFilter_B.t7 = (real32_T)sin(PlaneNavigationFilter_DW.x[5]);
  PlaneNavigationFilter_B.t8 = (real32_T)tan(PlaneNavigationFilter_DW.x[4]);
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t18 = PlaneNavigationFilter_B.t3 *
    PlaneNavigationFilter_B.t4;

  /* MATLABSystem: '<S9>/MATLAB System' */
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t24 = PlaneNavigationFilter_B.t3 *
    PlaneNavigationFilter_B.t7;

  /* MATLABSystem: '<S9>/MATLAB System' */
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t22 = PlaneNavigationFilter_B.bkj *
    PlaneNavigationFilter_B.t3;

  /* MATLABSystem: '<S9>/MATLAB System' */
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t23 = PlaneNavigationFilter_B.t3 *
    PlaneNavigationFilter_B.t5;

  /* MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreRead: '<S9>/Data Store ReadX'
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

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t22 = PlaneNavigationFilter_B.t5 *
    PlaneNavigationFilter_B.t6;

  /* MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreRead: '<S9>/Data Store ReadX'
   */
  PlaneNavigationFilter_B.y_cv[73] = PlaneNavigationFilter_B.t22 *
    PlaneNavigationFilter_DW.x[6] * -0.01F;

  /* Start for MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_B.t23 = PlaneNavigationFilter_B.bkj *
    PlaneNavigationFilter_B.t6;

  /* MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreRead: '<S9>/Data Store ReadP'
   *  DataStoreRead: '<S9>/Data Store ReadX'
   */
  PlaneNavigationFilter_B.y_cv[74] = PlaneNavigationFilter_B.t23 *
    PlaneNavigationFilter_DW.x[6] * -0.01F;
  PlaneNavigationFilter_B.y_cv[75] = (PlaneNavigationFilter_B.t8 *
    PlaneNavigationFilter_B.t8 + 1.0F) * PlaneNavigationFilter_B.t54 / 100.0F;
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
    memset(&PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.coffset + 1], 0,
           18U * sizeof(real32_T));
    for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 18;
         PlaneNavigationFilter_B.b_i++) {
      PlaneNavigationFilter_B.bkj =
        PlaneNavigationFilter_B.y_cv[PlaneNavigationFilter_B.b_i * 18 +
        PlaneNavigationFilter_B.i];
      for (PlaneNavigationFilter_B.c_i = 0; PlaneNavigationFilter_B.c_i < 18;
           PlaneNavigationFilter_B.c_i++) {
        PlaneNavigationFilter_B.bkj_o = (PlaneNavigationFilter_B.coffset +
          PlaneNavigationFilter_B.c_i) + 1;
        PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.bkj_o] +=
          PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.c_i * 18 +
          PlaneNavigationFilter_B.b_i] * PlaneNavigationFilter_B.bkj;
      }
    }
  }

  /* Start for MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  Constant: '<S2>/Q'
   * */
  memcpy(&PlaneNavigationFilter_B.fv[0], &tmp_1[0], 164U * sizeof(real32_T));
  memcpy(&PlaneNavigationFilter_B.fv[164], &tmp_2[0], 160U * sizeof(real32_T));
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
  memcpy(&PlaneNavigationFilter_B.fv[493], &tmp_3[0], 47U * sizeof(real32_T));
  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 30;
       PlaneNavigationFilter_B.i++) {
    for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
         18; PlaneNavigationFilter_B.coffset++) {
      PlaneNavigationFilter_B.t8 = 0.0F;
      for (PlaneNavigationFilter_B.b_i = 0; PlaneNavigationFilter_B.b_i < 30;
           PlaneNavigationFilter_B.b_i++) {
        PlaneNavigationFilter_B.t8 += PlaneNavigationFilter_B.fv[18 *
          PlaneNavigationFilter_B.b_i + PlaneNavigationFilter_B.coffset] *
          PlaneNavigationFilter_P.Q_Value[30 * PlaneNavigationFilter_B.i +
          PlaneNavigationFilter_B.b_i];
      }

      PlaneNavigationFilter_B.fv1[PlaneNavigationFilter_B.i + 30 *
        PlaneNavigationFilter_B.coffset] = PlaneNavigationFilter_B.t8;
    }
  }

  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
       PlaneNavigationFilter_B.i++) {
    memcpy(&PlaneNavigationFilter_B.y[PlaneNavigationFilter_B.i * 48],
           &PlaneNavigationFilter_B.y_g1[PlaneNavigationFilter_B.i * 18], 18U *
           sizeof(real32_T));
    memcpy(&PlaneNavigationFilter_B.y[PlaneNavigationFilter_B.i * 48 + 18],
           &PlaneNavigationFilter_B.fv1[PlaneNavigationFilter_B.i * 30], 30U *
           sizeof(real32_T));
  }

  /* MATLABSystem: '<S9>/MATLAB System' */
  PlaneNavigationFilter_qr_dk0ct(PlaneNavigationFilter_B.y,
    PlaneNavigationFilter_B.a__1, PlaneNavigationFilter_B.y_cv);

  /* DataStoreWrite: '<S9>/Data Store WriteP' incorporates:
   *  MATLABSystem: '<S9>/MATLAB System'
   * */
  for (PlaneNavigationFilter_B.i = 0; PlaneNavigationFilter_B.i < 18;
       PlaneNavigationFilter_B.i++) {
    for (PlaneNavigationFilter_B.coffset = 0; PlaneNavigationFilter_B.coffset <
         18; PlaneNavigationFilter_B.coffset++) {
      PlaneNavigationFilter_DW.P[PlaneNavigationFilter_B.coffset + 18 *
        PlaneNavigationFilter_B.i] = PlaneNavigationFilter_B.y_cv[18 *
        PlaneNavigationFilter_B.coffset + PlaneNavigationFilter_B.i];
    }
  }

  /* End of DataStoreWrite: '<S9>/Data Store WriteP' */

  /* MATLABSystem: '<S9>/MATLAB System' */
  memset(&PlaneNavigationFilter_B.fv2[0], 0, 30U * sizeof(real32_T));

  /* Start for MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreRead: '<S9>/Data Store ReadX'
   */
  memcpy(&PlaneNavigationFilter_B.Pxy[0], &PlaneNavigationFilter_DW.x[0], 18U *
         sizeof(real32_T));

  /* MATLABSystem: '<S9>/MATLAB System' incorporates:
   *  DataStoreWrite: '<S9>/Data Store WriteX'
   */
  PlaneN_nav_state_trans_dt_100Hz(PlaneNavigationFilter_B.Pxy,
    PlaneNavigationFilter_B.fv2,
    PlaneNavigationFilter_B.TmpSignalConversionAtMATLAB,
    PlaneNavigationFilter_DW.x);

  /* End of Outputs for SubSystem: '<S2>/Predict' */

  /* Update for Delay: '<Root>/Delay' */
  memcpy(&PlaneNavigationFilter_DW.Delay_DSTATE[0],
         &PlaneNavigationFilter_B.ekf_state_std[0], 18U * sizeof(real32_T));
}

/* Model initialize function */
void PlaneNavigationFilter_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  {
    int32_T i;

    /* Start for DataStoreMemory: '<S2>/DataStoreMemory - P' */
    memcpy(&PlaneNavigationFilter_DW.P[0],
           &PlaneNavigationFilter_P.DataStoreMemoryP_InitialValue[0], 324U *
           sizeof(real32_T));

    /* Start for DataStoreMemory: '<S2>/DataStoreMemory - x' */
    memcpy(&PlaneNavigationFilter_DW.x[0],
           &PlaneNavigationFilter_P.DataStoreMemoryx_InitialValue[0], 18U *
           sizeof(real32_T));

    /* InitializeConditions for Delay: '<Root>/Delay' */
    for (i = 0; i < 18; i++) {
      PlaneNavigationFilter_DW.Delay_DSTATE[i] =
        PlaneNavigationFilter_P.Delay_InitialCondition;
    }

    /* End of InitializeConditions for Delay: '<Root>/Delay' */

    /* SystemInitialize for Enabled SubSystem: '<S2>/Correct1' */
    /* SystemInitialize for MATLABSystem: '<S5>/MATLAB System' incorporates:
     *  Outport: '<S5>/yBlockOrdering'
     */
    PlaneNavigationFilter_B.MATLABSystem_o3_f =
      PlaneNavigationFilter_P.yBlockOrdering_Y0;

    /* End of SystemInitialize for SubSystem: '<S2>/Correct1' */
  }
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
