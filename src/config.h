#ifndef CONFIG_H
#define CONFIG_H

#include "definitions.h"

//================================================================================
// CONSTANTS
//================================================================================

// -- Sensor Signal Processing --
const float IDLE_PRESSURE_MIN_KPA = 95.0;
const float IDLE_PRESSURE_MAX_KPA = 105.0;
const float REACTIVATE_PRESSURE_KPA = 75.0;

// -- Sensor Calibration --
const float R1_OHMS = 9980.0;
const float R2_OHMS = 15000.0;

// -- Touch Input --
const uint32_t TOUCH_SENSITIVITY_OFFSET = 10000;
const unsigned long DEBOUNCE_DELAY = 200;

// -- Spool Score Parameters --
const float ARMING_THRESHOLD_KPA = 105.0;
const int ARMING_DWELL_SAMPLES = 5;
const float TERMINATION_DROP_KPA = 4.0;

//================================================================================
// PARAMETER INFO TEXT
//================================================================================

extern const char* INFO_KP;
extern const char* INFO_KI;
extern const char* INFO_KD;
extern const char* INFO_MAX_I;
extern const char* INFO_TRIG_THRESH;
extern const char* INFO_PRESSURE_LIMIT;
extern const char* INFO_SOLENOID_FREQ;
extern const char* INFO_PRESSURE_OFFSET;
extern const char* INFO_MAP_SENSOR;
extern const char* INFO_SLOW_EMA;
extern const char* INFO_FAST_EMA;
extern const char* INFO_PRATE_THRESH;
extern const char* INFO_RATE_PERIOD;
extern const char* INFO_OVERSAMPLING;
extern const char* INFO_SAVE_DELAY;
extern const char* INFO_EDIT_DELAY;
extern const char* INFO_SLEEP_DELAY;
extern const char* INFO_TS_RATE;

//================================================================================
// MENU DEFINITIONS
//================================================================================

extern const MenuItem pidMenuItems[];
extern const int pidMenuCount;

extern const MenuItem mapMenuItems[];
extern const int mapMenuCount;

extern const MenuItem filterMenuItems[];
extern const int filterMenuCount;


#endif // CONFIG_H
