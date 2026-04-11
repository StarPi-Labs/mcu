#include <board.h>
#include <logging.h>
#include <scheduler.h>
#include <messages.h>
#include <tasks.h>
#include <SerialUSB.h>
#include <stdatomic.h>
#include <pico/platform.h>

#include "helpers.h"
#include "config.h"
#include "serial_commands.h"
#include "sdcard.h"


extern void esc_arm(void); // in esc.cpp
extern bool set_motor_rpm(uint16_t rpm); // in esc.cpp
extern void esc_disarm(void); // in esc.cpp


static bool init_done = false;
volatile bool system_armed = true; // TODO: FIX


// to be run periodically to reset the watchdog
static bool reset_watchdog_task(void* args)
{
    (void)args;
    watchdog_update();
    return true;
}


static bool check_system_armed(void* args)
{
    (void)args;
    system_armed = system_armed && digitalRead(PIN_SYSTEM_ARM);
    return true;
}


/* periodically check for over current protection (ocp) and undervoltage lockout
 * (uvlo) conditions, if met then disarm the system to prevent damage */
static bool check_ocp_uvlo(void *args)
{
    (void)args;
    static absolute_time_t last_check = get_absolute_time();
    absolute_time_t now = get_absolute_time();

    // Calculate time elapsed since last check in microseconds
    int64_t dt_us = absolute_time_diff_us(last_check, now);

    // Static accumulators to track how long we've been in a fault state
    static int64_t ocp_time_us = 0;
    static int64_t uvlo_time_us = 0;

    // Fetch the latest readings without triggering a blocking ADC read
    float current = board_last_current_reading();
    float voltage = board_last_voltage_reading();

    // 1. Over-Current Protection (OCP) Check
    if (current > MAX_BATTERY_CURRENT) {
        ocp_time_us += dt_us;
        if (ocp_time_us >= (int64_t)MAX_BATTERY_CURRENT_SECONDS * 1000000LL) {
            system_armed = false;
            Serial.printf("OCP TRIGGERED! Current: %.2f A\n", current);
        }
    } else if (current < (MAX_BATTERY_CURRENT - OCP_HYSTERESIS)) {
        // Only reset the timer if current drops into the safe recovery zone
        ocp_time_us = 0;
    }

    // 2. Under-Voltage Lockout (UVLO) Check
    if (voltage < MIN_BATTERY_VOLTAGE) {
        uvlo_time_us += dt_us;
        if (uvlo_time_us >= (int64_t)MIN_BATTERY_VOLTAGE_SECONDS * 1000000LL) {
            system_armed = false;
            Serial.printf("UVLO TRIGGERED! Voltage: %.2f V\n", voltage);
        }
    } else if (voltage > (MIN_BATTERY_VOLTAGE + UVLO_HYSTERESIS)) {
        // Only reset the timer if voltage recovers into the safe recovery zone
        uvlo_time_us = 0;
    }

    last_check = now;
    return true;
}


static void system_armed_irq(void)
{
    // wait a bit, this event could be triggered by vibrations
    //delayMicroseconds(10);
    system_armed = false; //digitalRead(PIN_SYSTEM_ARM);
}


// CORE 0 ------------------------------------------------------------------- //

void setup() {
    init_done = false; // just to be sure

    if (watchdog_caused_reboot() || watchdog_enable_caused_reboot()) {
        ERROR("ERROR: BOARD SHUT OFF BECAUSE OF THE WATCHDOG\n");
        // TODO: blink a LED or do something
    }

    board_init();
    message_queues_init();

    // wait for system arm
    pinMode(PIN_SYSTEM_ARM, INPUT);
    while (!check_system_armed(NULL)) delay(1);

    init_done = true;

    // enable the watchdog to make sure the board turns off in case of error
    watchdog_enable(WATCHDOG_TIMEOUT_MS, true);

    // enable the system arm interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_SYSTEM_ARM), system_armed_irq, LOW);
}


void loop() {
    if (!system_armed) {
        Serial.println("DISARMED");
    }
    char buffer[300];
    MessageQueue* queue = poll_ready_queue(0);
    if (queue) {
        void* msg = message_queue_front(queue, 0);
        message_to_json(msg, queue->desc, buffer, sizeof(buffer));
        Serial.println(buffer);
            card_log(msg, queue->desc);

        message_queue_consume(queue, 0);
    }

    serial_command_t cmd;
    int32_t cmd_data;
    if (parse_serial_command(&cmd, &cmd_data)) {
        switch (cmd) {
        case CMD_RPM:
        {
            MessageQueue* rpm_ref_queue = rpm_ref_message_queue();
            rpm_t rpm_msg = {
                .timestamp = board_us_since_boot(),
                .val = (int16_t)cmd_data,
            };
            message_queue_push(rpm_ref_queue, &rpm_msg);
        }
            break;
        case CMD_ARM:
            Serial.println("ARMED");
            esc_arm();
            break;
        case CMD_DISARM:
            Serial.println("DISARMED");
            esc_disarm();
            break;
        case CMD_NEW_SESSION:
            if (sdcard_start_session()) {
                Serial.println("SESSION STARTED");
            } else {
                Serial.println("SESSION START FAILED");
            }
            break;
        case CMD_STOP_SESSION:
            if (sdcard_end_session()) {
                Serial.println("SESSION STOPPED");
            } else {
                Serial.println("SESSION STOP FAILED");
            }
            break;
        case CMD_TARE:
            Serial.println("taring load cell, oneshot");
            board_tare_lc();
        default:
            break;
        }
    }
}


// CORE 1 ------------------------------------------------------------------- //

/* Setup: all data gathering and controlling tasks are assigned here */
static callback_id_t stoppable_tasks[8];
static int num_stoppable_tasks = 0;
void setup1() {
    // wait for board initialization
    // TODO: this should be atomic
    while (!init_done) delay(1);

    scheduler_init();

    // stoppable tasks, all sensors
    stoppable_tasks[num_stoppable_tasks++] = scheduler_add_callback(CORE_ID_1, 1, read_battery);
    stoppable_tasks[num_stoppable_tasks++] = scheduler_add_callback(CORE_ID_1, 50, read_rpm);
    stoppable_tasks[num_stoppable_tasks++] = scheduler_add_callback(CORE_ID_1, 1000/HX711_DATA_RATE_HZ, read_load_cell);
    //scheduler_add_callback(CORE_ID_1, 200, read_temperatures);
    stoppable_tasks[num_stoppable_tasks++] = scheduler_add_callback(CORE_ID_1, 100, command_motor);
    stoppable_tasks[num_stoppable_tasks++] = scheduler_add_callback(CORE_ID_1, 500, check_system_armed);

    // watchdog reset shouldn't be stopped
    scheduler_add_callback(CORE_ID_1, WATCHDOG_RESET_EVERY_MS, reset_watchdog_task);
    scheduler_add_callback(CORE_ID_1, 500, check_ocp_uvlo);

    sleep_ms(500);

    scheduler_start();
}


/* Periodically checks if the system is armed, if not stops the scheduler and
 * stops the motor */
void loop1() {
    static bool was_armed = true;
    // TODO: this check should be atomic
    if (was_armed && !system_armed) {
        for (int i = 0; i < num_stoppable_tasks; i++)
            scheduler_pause_task(CORE_ID_1, stoppable_tasks[i]);
        was_armed = false;
    }
    if (!system_armed) { \
        set_motor_rpm(0);
        esc_disarm();
        board_toggle_led();
        sleep_ms(20);
        board_toggle_led();
        sleep_ms(800);
    }
}

