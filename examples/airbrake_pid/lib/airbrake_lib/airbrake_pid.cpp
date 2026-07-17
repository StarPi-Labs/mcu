
#include "airbrake_pid.h"
#include <cmath>
#include "rtwtypes.h"
#include "airbrake_pid_private.h"

bool AirbrakePID::verificaCondizioni(float vel_z, float tilt, bool trigger_fine_boost) {
    if (std::abs(tilt) > MAX_TILT_RAD || vel_z < 10.0f || !trigger_fine_boost) {
        return false;
    } else return true;
}

real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
                    uint32_T maxIndex)
{
  real_T frac;
  real_T yL_0d0;
  uint32_T iLeft;

  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

    // Binary Search
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

real_T look1_binlcapw(real_T u0, const real_T bp0[], const real_T table[],
                    uint32_T maxIndex)
{
  real_T frac;
  real_T y;
  uint32_T iLeft;

  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0;
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;

    // Binary Search
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex;
    frac = 0.0;
  }

  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    real_T yL_0d0;
    yL_0d0 = table[iLeft];
    y = (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[],
                    const real_T table[], const uint32_T maxIndex[], uint32_T
                    stride)
{
  real_T fractions[2];
  real_T frac;
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex[0U]]) {
    // Binary Search
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex[0U] - 1U;
    frac = (u0 - bp0[maxIndex[0U] - 1U]) / (bp0[maxIndex[0U]] - bp0[maxIndex[0U]
      - 1U]);
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = (u1 - bp1[0U]) / (bp1[1U] - bp1[0U]);
  } else if (u1 < bp1[maxIndex[1U]]) {
    // Binary Search
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u1 - bp1[iLeft]) / (bp1[iLeft + 1U] - bp1[iLeft]);
  } else {
    iLeft = maxIndex[1U] - 1U;
    frac = (u1 - bp1[maxIndex[1U] - 1U]) / (bp1[maxIndex[1U]] - bp1[maxIndex[1U]
      - 1U]);
  }

  bpIdx = iLeft * stride + bpIndices[0U];
  yL_0d0 = table[bpIdx];
  yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
  bpIdx += stride;
  yL_0d1 = table[bpIdx];
  return (((table[bpIdx + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) *
    frac + yL_0d0;
}

void AirbrakePID::step(float altitudine, float vel_z, float tilt, bool trigger_fine_boost) {

    real_T Coppia_Attiva_Nm;
    real_T target_angle_rad;
    real_T last_commanded_angle_deg;
    real_T P;
    real_T I;
    real_T D;
    real_T rtb_Momento_Torcente;
    real_T drag_force;
    real_T air_density;
    real_T mach_number;
    real_T P_scaled;
    real_T raw_pid_output;
    real_T flap_extension;
    real_T fattore_inclinazione;
    real_T local_gravity;
    real_T demanded_angle_deg;
    real_T demanded_angle_rad;
    real_T saturation_difference;

    // Decide se far agire il PID oppure uscire dalla funzione step() 
    if (!verificaCondizioni(vel_z, tilt, trigger_fine_boost)) {
        airbrake_pid.last_commanded_angle_rad = MIN_ANGLE_RAD;
        airbrake_pid.estimated_apogee = apogeo;
        return;
    }

    // Calcolo del fattore di inclinazione
    fattore_inclinazione = 1.0f / std::cos(tilt);

    last_commanded_angle_deg = airbrake_pid.last_commanded_angle_rad * RAD_TO_DEG;

    flap_extension = look1_binlcapw(last_commanded_angle_deg,
        airbrake_pid_ConstP.x_geom_bp01Data, airbrake_pid_ConstP.x_geom_tableData, 48U);

    air_density = look1_binlxpw(altitudine, airbrake_pid_ConstP.pooled6, airbrake_pid_ConstP.pooled5, 199U);

    mach_number = 1.0f / std::sqrt(401.94f * look1_binlxpw
        (altitudine, airbrake_pid_ConstP.pooled6,
        airbrake_pid_ConstP.pooled7, 199U)) * vel_z;

    Coppia_Attiva_Nm = air_density / 1.225f;

    drag_force = look2_binlxpw(mach_number, flap_extension, airbrake_pid_ConstP.pooled9,
        airbrake_pid_ConstP.pooled10, airbrake_pid_ConstP.Q_h0_tableData,
        airbrake_pid_ConstP.pooled14, 4U) * Coppia_Attiva_Nm;

    // Calcola l'azione Proporzionale
    P = (airbrake_pid.estimated_apogee - apogeo) * Kp;

    // Adatta l'errore proporzionale rispetto alla frequenza di esecuzione (50 Hz) 
    // per permettere il calcolo della variazione dell'errore
    P_scaled = P * SampleFreq;

    // Calcola l'azione Integrale
    I = Ki * airbrake_pid.pid_integrator_state;

    // Calcola l'azione Derivativa
    D = (P_scaled - airbrake_pid.prev_P_scaled) * Kd;

    raw_pid_output = (P + I + D) * DEG_TO_RAD;

    demanded_angle_deg = raw_pid_output + airbrake_pid.aw_correction_deg;
    demanded_angle_rad = demanded_angle_deg * DEG_TO_RAD;

    // Impedisce al PID di richiedere angoli fisicamente impossibili per i leveraggi (0.0 - 2.512rad)
    if (demanded_angle_rad < MIN_ANGLE_RAD) {
        target_angle_rad = MIN_ANGLE_RAD;
    } else if (demanded_angle_rad > MAX_ANGLE_RAD) {
        target_angle_rad = MAX_ANGLE_RAD;
    } else {
        target_angle_rad = demanded_angle_rad;
    }

    // Calcola lo sforzo meccanico sui flap basandosi sulla pressione aerodinamica teorica
    // !!! In questo momento non è utile ma potrebbe essere tenuta per condizionare l'attivazione del PID

    airbrake_pid.UnitDelay3_DSTATE = drag_force * flap_extension * 0.02388065f;


    // Fornisce il valore locale esatto dell'accelerazione di gravità in base all'altezza
    local_gravity = look1_binlxpw(altitudine, airbrake_pid_ConstP.g_bp01Data, airbrake_pid_ConstP.g_tableData, 78U);

    // Nucleo predittivo del sistema
    air_density *= 0.5f;

    rtb_Momento_Torcente = (0.263548f * flap_extension + 0.292048f) * air_density *
        fattore_inclinazione / ROCKET_DRY_MASS_KG;

    if (vel_z > 0.0f && rtb_Momento_Torcente > 1e-6f) {    
        airbrake_pid.estimated_apogee = std::log
        (vel_z * vel_z * rtb_Momento_Torcente / local_gravity + 1.0f) * 
        (1.0f / (2.0f * rtb_Momento_Torcente)) + altitudine;
    } else {
        airbrake_pid.estimated_apogee = apogeo;
    }

    // Anti-Windup
    saturation_difference = demanded_angle_deg - (target_angle_rad * RAD_TO_DEG);

    if (std::abs(saturation_difference) < 0.1) {      // !!! ci sta mettere 0.1? avere saturation_difference==0 sembra molto improbabile.
        airbrake_pid.aw_integrator_state = 0.0f;
        airbrake_pid.aw_correction_deg = 0.0f;
    } else {
        airbrake_pid.aw_integrator_state += saturation_difference * SampleTime;
        airbrake_pid.aw_correction_deg = -4.0f * airbrake_pid.aw_integrator_state;
    }

    // Aggiorna la memoria della posizione dei flap
    airbrake_pid.last_commanded_angle_rad = target_angle_rad;

    // Accumula l'errore corrente nel tempo per consentire il funzionamento dell'azione Integrale 
    airbrake_pid.pid_integrator_state += SampleTime * P;

    // Salva l'errore corrente in RAM per calcolare la differenza con l'errore futuro (derivata)
    airbrake_pid.prev_P_scaled = P_scaled;

}

// Model initialize function
void AirbrakePID::initialize() {
    airbrake_pid.pid_integrator_state = 0.0f;
    airbrake_pid.prev_P_scaled = 0.0f;
    airbrake_pid.last_commanded_angle_rad = MIN_ANGLE_RAD;
    airbrake_pid.aw_correction_deg = 0.0f;
    airbrake_pid.aw_integrator_state = 0.0f;

    // Inizializza la stima dell'apogeo al valore target per evitare sbalzi strani al primo ciclo
    airbrake_pid.estimated_apogee = apogeo; 
}

void AirbrakePID::terminate(){

}

AirbrakePID::AirbrakePID() :
    airbrake_pid(),
    airbrake_pid_M()
{
}

AirbrakePID::~AirbrakePID() = default;
