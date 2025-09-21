#include "config.h"
#include "definitions.h"

//================================================================================
// PARAMETER INFO TEXT DEFINITIONS
//================================================================================

const char* INFO_KP = "P-Gain: Main driving force. Higher is faster but can oscillate.";
const char* INFO_KI = "I-Gain: Corrects steady error over time. Fights boost droop.";
const char* INFO_KD = "D-Gain: Dampens overshoot by reacting to the rate of change.";
const char* INFO_MAX_I = "Max I-Term: Prevents integral 'windup' causing large overshoots.";
const char* INFO_TRIG_THRESH = "PID Trigger (kPa): How close to target for PID loop to activate.";
const char* INFO_PRESSURE_LIMIT = "Limiter (kPa): Forces wastegate open if pressure > target + value.";
const char* INFO_SOLENOID_FREQ = "Solenoid Freq (Hz): Match to your specific solenoid's spec sheet.";
const char* INFO_PRESSURE_OFFSET = "Offset (kPa): Manual correction to match final reading to a known gauge.";
const char* INFO_MAP_SENSOR = "These values must match your pressure sensor's datasheet exactly.";
const char* INFO_SLOW_EMA = "Slow EMA: Heavy smoothing for stable boost (0-1). Low val=more smooth.";
const char* INFO_FAST_EMA = "Fast EMA: Light smoothing for rapid boost change (0-1). High val=less smooth.";
const char* INFO_PRATE_THRESH = "P-Rate Thresh (kPa): Change needed to switch from slow to fast EMA.";
const char* INFO_RATE_PERIOD = "Rate Period (ms): Time window for calculating pressure rate of change.";
const char* INFO_OVERSAMPLING = "Oversampling: ADC reads to avg for 1 measurement. Reduces noise at cost of CPU time.";
const char* INFO_SAVE_DELAY = "Save/Reset Hold (ms): Time to hold SAVE or RESET button to activate.";
const char* INFO_EDIT_DELAY = "Edit/CFG Hold (ms): Time to hold EDIT or CFG button to enter menu.";
const char* INFO_SLEEP_DELAY = "Sleep Delay (s): Idle time at atmos before screen/solenoid turns off.";
const char* INFO_TS_RATE = "TS Rate (ms): Sample rate for Torque Score calculation.";

//================================================================================
// MENU DEFINITIONS
//================================================================================

const MenuItem pidMenuItems[] = {
    {"Kp", &kp, P_FLOAT, 2, "", INFO_KP},
    {"Ki", &ki, P_FLOAT, 3, "", INFO_KI},
    {"Kd", &kd, P_FLOAT, 2, "", INFO_KD},
    {"Max I term", &maxIntegral, P_FLOAT, 0, "", INFO_MAX_I},
    {"Trig. Thres.", &pidTriggerkPa, P_FLOAT, 1, "kPa", INFO_TRIG_THRESH}, 
    {"Press. Limiter", &PID_Control_Overhead, P_FLOAT, 1, "kPa", INFO_PRESSURE_LIMIT},
    {"Solenoid Freq.", &valveFrequencyHz, P_INT, 0, "Hz", INFO_SOLENOID_FREQ}
};
const int pidMenuCount = sizeof(pidMenuItems) / sizeof(MenuItem);

const MenuItem mapMenuItems[] = {
    {"Pressure Offset", &PRESSURE_CORRECTION_KPA, P_FLOAT, 2, "kPa", INFO_PRESSURE_OFFSET},
    {"Min kPa", &MIN_KPA, P_FLOAT, 1, "kPa", INFO_MAP_SENSOR},
    {"Max kPa", &MAX_KPA, P_FLOAT, 1, "kPa", INFO_MAP_SENSOR},
    {"V Offset", &RAW_VOLTAGE_OFFSET, P_FLOAT, 4, "V", INFO_MAP_SENSOR},
    {"Min Volts", &RAW_MIN_SENSOR_VOLTAGE, P_FLOAT, 2, "V", INFO_MAP_SENSOR},
    {"Max Volts", &RAW_MAX_SENSOR_VOLTAGE, P_FLOAT, 2, "V", INFO_MAP_SENSOR}
};
const int mapMenuCount = sizeof(mapMenuItems) / sizeof(MenuItem);

const MenuItem filterMenuItems[] = {
    {"Slow EMA-A", &slow_ema_a, P_FLOAT, 3, "", INFO_SLOW_EMA},
    {"Fast EMA-A", &fast_ema_a, P_FLOAT, 2, "", INFO_FAST_EMA},
    {"P-Rate Thres.", &kpa_rate_change_threshold, P_FLOAT, 1, "kPa", INFO_PRATE_THRESH},
    {"Rate period", &kpa_rate_time_interval_ms, P_INT, 0, "ms", INFO_RATE_PERIOD},
    {"Oversampling", &OVERSAMPLE_COUNT, P_INT, 0, "", INFO_OVERSAMPLING},
    {"Save/Reset Delay", &SAVE_RESET_HOLD_TIME_MS, P_ULONG, 0, "ms", INFO_SAVE_DELAY},
    {"Edit/CFG Delay", &EDIT_HOLD_TIME_MS, P_ULONG, 0, "ms", INFO_EDIT_DELAY},
    {"Sleep Delay", &IDLE_TIMEOUT_SECONDS, P_FLOAT, 0, "s", INFO_SLEEP_DELAY},
    {"TS Rate", &tsSampleRate, P_INT, 0, "ms", INFO_TS_RATE}
};
const int filterMenuCount = sizeof(filterMenuItems) / sizeof(MenuItem);