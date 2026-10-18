#include <Arduino.h>

#include "imu.h"
#include "board.h"
#include "logger.h"


extern SPIClass SPI2;
LSM6DSO32Sensor IMU(&SPI2, IMU_CS);
static volatile bool interrupt_flag = false;


/*
// Interrupt handler for IMU FIFO interrupt
static void IRAM_ATTR imu_fifo_interrupt()
{
	interrupt_flag = true;
}
 */


// FIXME: check errors
// returns true on success, false on failure
bool imu_setup()
{
	if (IMU.begin() != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Failed to initialize IMU");
		return false;
	}

	// Should be 0x6C
	uint8_t id;
	if (IMU.ReadID(&id) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get ID");
		return false;
	}
	if (id != 0x6C) {
		log(S_IMU, T_SYSLOG, "[ERR] ID mismatch");
	}

	if (IMU.Enable_X() !=0 || IMU.Enable_G() != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Enable gyro or accel");
		return false;
	}

	if (IMU.Set_X_FS(IMU_ACCEL_FS) != 0 || IMU.Set_G_FS(IMU_GYRO_FS) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change FS");
		return false;
	}

#if IMU_FIFO_ENABLE
	// FIFO Configuration
	// TODO: to enable timestamps we need to fork the library and expose an
	//       API to change the correct bits in FIFO_CTRL4
	if (IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Enable FIFO");
		return false;
	}
	if (IMU.Set_FIFO_X_BDR(IMU_FIFO_X_BDR_HZ) != 0 || IMU.Set_FIFO_G_BDR(IMU_FIFO_G_BDR_HZ) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change buffer data rate");
		return false;
	}
	if (IMU.Set_X_ODR(IMU_FIFO_X_BDR_HZ) != 0 || IMU.Set_G_ODR(IMU_FIFO_G_BDR_HZ) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change output data rate");
		return false;
	}
	
	// FIFO Interrupt
//	IMU.Set_FIFO_Watermark_Level(IMU_FIFO_WATERMARK);
//	pinMode(IMU_INT1, INPUT_PULLDOWN);
//	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
#endif

	return true;
}

// return true on success, false on failure
// Assumes the board is stationary
bool imu_calibrate(float *g_cal, unsigned int cal_time_ms)
{
	if (g_cal == NULL) return false;
	float g = 0.0f; 
	int samples = 0;
	int64_t start_time = millis();
	while (millis() - start_time < cal_time_ms) {
		FIFO_Sample sample;
		if (imu_get_sample(&sample) == 0) {
			float ax = (float)sample.accelerometer[0]/1000.0f;
			float ay = (float)sample.accelerometer[1]/1000.0f;
			float az = (float)sample.accelerometer[2]/1000.0f;

			g += sqrt(ax*ax + ay*ay + az*az);
			samples++;
			delay(5);
		}
	}
	if (samples != 0) {
		g /= samples;
		*g_cal = g;
		return true;
	}
	return false;
}

// TODO: make this consistent with imu_init
int imu_get_sample(FIFO_Sample *sample)
{
	if (sample == NULL) return -1;

#if IMU_FIFO_ENABLE
	uint16_t n_samples;
	if (IMU.Get_FIFO_Num_Samples(&n_samples) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get FIFO status");
		return -1;
	}
	if (n_samples == 0) return -1;

	uint16_t x_count = 0, g_count = 0;
	int32_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
	FIFO_Sample tmp_sample = {0};

	for (int i = 0; i < n_samples; i++) {
		uint8_t tag = 0;
		if (IMU.Get_FIFO_Tag(&tag) != 0) {
			log(S_IMU, T_SYSLOG, "[ERR] Get FIFO tag");
			return -1;
		}

		switch (tag) {
		case LSM6DSO32_XL_NC_TAG:
			if (IMU.Get_FIFO_X_Axes(tmp_sample.accelerometer) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get X axes");
				return -1;
			}
			ax += tmp_sample.accelerometer[0];
			ay += tmp_sample.accelerometer[1];
			az += tmp_sample.accelerometer[2];
			x_count++;
			break;
		case LSM6DSO32_GYRO_NC_TAG:
			if (IMU.Get_FIFO_G_Axes(tmp_sample.gyroscope) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get G axes");
				return -1;
			}
			gx += tmp_sample.gyroscope[0];
			gy += tmp_sample.gyroscope[1];
			gz += tmp_sample.gyroscope[2];
			g_count++;
			break;
		case LSM6DSO32_TIMESTAMP_TAG:
			// TODO: convert timestamp to unix time (local time in microseconds)
			if (IMU.Get_FIFO_Data(tmp_sample.timestamp) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get timestamp");
				return -1;
			}
			break;
		default:
			break;
		}
	}

	if (x_count == 0 || g_count == 0) {
		// Serial.println("No accelerometer or gyroscope samples in FIFO");
		return -1;
	}

	sample->accelerometer[0] = ax / x_count;
	sample->accelerometer[1] = ay / x_count;
	sample->accelerometer[2] = az / x_count;
	sample->gyroscope[0]     = gx / g_count;
	sample->gyroscope[1]     = gy / g_count;
	sample->gyroscope[2]     = gz / g_count;
	// Serial.printf("FIFO Sample: %d accel samples, %d gyro samples\n", x_count, g_count);

	#else
	if (IMU.Get_X_Axes(sample->accelerometer) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get X axes");
		return -1;
	}
	if (IMU.Get_G_Axes(sample->gyroscope) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get G axes");
		return -1;

	}
#endif

	return 0;
}


bool is_imu_healthy_ground(float ax, float ay, float az, float gx, float gy, float gz) {

    static int count = 0;
    static constexpr int max_count = 10; 

    //controllo threshold acc e gyro

    constexpr float gyro_threshold = 5.0f; 
    bool gyro_thr_ok = std::abs(gx) < gyro_threshold && std::abs(gy) < gyro_threshold && std::abs(gz) < gyro_threshold;



    //controllo acc = 1g
    float gravity = std::sqrt(ax*ax + ay*ay + az*az);
    float gravity_threshold = 0.1f;
    bool gravity_ok = std::abs(gravity - 1.0f) < gravity_threshold;
    


    //controllo varianza 
    static constexpr int samples = 10;
    static constexpr float acc_variance_threshold = (10 * 5.4f * 0.001f) * (10 *5.4f * 0.001f);// 5.4 mg rumore minimo
    static constexpr float gyro_variance_threshold = (10 * 75.0f * 0.001f) * (10 * 75.0f * 0.001f); //75 mdps rumore minimo

    static float ax_samples[samples] = {};
    static float ay_samples[samples] = {};
    static float az_samples[samples] = {};
    
    static float gx_samples[samples] = {};
    static float gy_samples[samples] = {};
    static float gz_samples[samples] = {};

    static int sample_index = 0;


    ax_samples[sample_index] = ax;
    ay_samples[sample_index] = ay;
    az_samples[sample_index] = az;

    gx_samples[sample_index] = gx;
    gy_samples[sample_index] = gy;
    gz_samples[sample_index] = gz;

    sample_index = (sample_index + 1) % samples;

    static bool variance_ok = true;

    if (sample_index == 0) {
        float ax_mean = 0, ay_mean = 0, az_mean = 0;
        float gx_mean = 0, gy_mean = 0, gz_mean = 0;

        // Calcolo di tutte le medie in un solo ciclo (ottimizzato!)
        for (int i = 0; i < samples; i++) {
            ax_mean += ax_samples[i];
            ay_mean += ay_samples[i];
            az_mean += az_samples[i];
            gx_mean += gx_samples[i];
            gy_mean += gy_samples[i];
            gz_mean += gz_samples[i];
        }
        ax_mean /= samples; ay_mean /= samples; az_mean /= samples;
        gx_mean /= samples; gy_mean /= samples; gz_mean /= samples;

        float ax_variance = 0, ay_variance = 0, az_variance = 0;
        float gx_variance = 0, gy_variance = 0, gz_variance = 0;

        // Calcolo di tutte le varianze in un solo ciclo
        for (int i = 0; i < samples; i++) {
            ax_variance += (ax_samples[i] - ax_mean) * (ax_samples[i] - ax_mean);
            ay_variance += (ay_samples[i] - ay_mean) * (ay_samples[i] - ay_mean);
            az_variance += (az_samples[i] - az_mean) * (az_samples[i] - az_mean);
            gx_variance += (gx_samples[i] - gx_mean) * (gx_samples[i] - gx_mean);
            gy_variance += (gy_samples[i] - gy_mean) * (gy_samples[i] - gy_mean);
            gz_variance += (gz_samples[i] - gz_mean) * (gz_samples[i] - gz_mean);
        }
        ax_variance /= samples; ay_variance /= samples; az_variance /= samples;
        gx_variance /= samples; gy_variance /= samples; gz_variance /= samples;

        // Controllo che NESSUN asse sia congelato (varianza troppo piccola) 
        // e che NESSUN asse sia troppo rumoroso (varianza sopra il threshold)
        variance_ok = 
            (ax_variance < acc_variance_threshold && ax_variance > std::numeric_limits<float>::epsilon()) &&
            (ay_variance < acc_variance_threshold && ay_variance > std::numeric_limits<float>::epsilon()) &&
            (az_variance < acc_variance_threshold && az_variance > std::numeric_limits<float>::epsilon()) &&
            (gx_variance < gyro_variance_threshold && gx_variance > std::numeric_limits<float>::epsilon()) &&
            (gy_variance < gyro_variance_threshold && gy_variance > std::numeric_limits<float>::epsilon()) &&
            (gz_variance < gyro_variance_threshold && gz_variance > std::numeric_limits<float>::epsilon());
    }


    if (!gyro_thr_ok || !gravity_ok || !variance_ok) {
        count++;
    }
    else count = 0;

    return count < max_count;
}

