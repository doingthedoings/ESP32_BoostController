#include "definitions.h"

//================================================================================
// GLOBAL VARIABLE DEFINITIONS
//================================================================================

// -- Hardware --
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int touchPins[] = {TOUCH_PIN_1, TOUCH_PIN_2, TOUCH_PIN_3, TOUCH_PIN_4, TOUCH_PIN_5, TOUCH_PIN_6};
int touchCalibrationValues[6];

// -- RTOS --
SemaphoreHandle_t dataMutex;
TaskHandle_t pidControlTaskHandle;
TaskHandle_t displayAndInputTaskHandle;

// -- System State --
float targetkPa;
float pressurekPa;
float minSensorVoltage, maxSensorVoltage, scaledVoltageOffset;
float peakHoldkPa;
float spoolScore = 0.0;
float torqueScore = 0.0;
float spoolScoreA = 0.0, spoolScoreB = 0.0;
float torqueScoreA = 0.0, torqueScoreB = 0.0;
char activeProfile = 'A';
float controlPercent;
bool isDisplayAsleep = false;
bool userActivity = false;
bool displayNeedsUpdate = true;

// -- Score State Machines --
SpoolScoreState spoolState = SPOOL_IDLE;
TorqueScoreState torqueState = TORQUE_IDLE;
std::vector<PressureTimestamp> boostEventData;
float p_start, p_peak;
unsigned long t_start, t_peak;

// -- Preset Management --
ControllerPreset presets[2];
int activePresetIndex = -1;

// -- UI State --
ScreenState currentScreen = MAIN_SCREEN;
unsigned long editHoldStart = 0, saveHoldStart = 0, resetHoldStart = 0, cfgHoldStart = 0, cfgSaveHoldStart = 0, saveAHoldStart = 0, saveBHoldStart = 0;
int configMenuIndex = 0, pidMenuIndex = 0, mapMenuIndex = 0, filterMenuIndex = 0, menuScrollOffset = 0;
ScreenState lastMenuScreen;
int lastMenuIndex, lastScrollOffset;
void* currentEditingValuePtr = nullptr;
ParamType currentEditingType;
float tempEditValue;
const char* currentParamName = "";
const char* currentParamUnit = "";
int currentParamPrecision = 0;
const char* currentInfoText = "";
char confirmationLine1[20];
char confirmationLine2[20];
unsigned long confirmationEndTime;
ScreenState screenAfterConfirmation;

// -- Adjustable Parameters --
int DISPLAY_BRIGHTNESS = 100;
float kp = 10.0;
float ki = 0.1;
float kd = 1.0;
float defaultTargetkPa = 170.0;
float maxIntegral = 600.0;
int valveFrequencyHz = 33;
float PID_Control_Overhead = 6.0;
float pidTriggerkPa = 20.0;
float slow_ema_a = 0.03;
float fast_ema_a = 0.3;
float kpa_rate_change_threshold = 10.0;
int kpa_rate_time_interval_ms = 50;
float output_ema_a = 0.2;
int OVERSAMPLE_COUNT = 256;
float IDLE_TIMEOUT_SECONDS = 60;
//Defaults configured for BOSCH 0281002976 PST-3 sensor
//https://www.bosch-motorsport.com/content/downloads/Raceparts/Resources/pdf/Data%20Sheet_70513419_Pressure_Sensor_Combined_PST_1/PST_3.pdf
float RAW_MIN_SENSOR_VOLTAGE = 0.4;
float RAW_MAX_SENSOR_VOLTAGE = 4.65;
float RAW_VOLTAGE_OFFSET = -0.09643;
float MIN_KPA = 20.0;
float MAX_KPA = 300.0;
float PRESSURE_CORRECTION_KPA = 1.27;
unsigned long EDIT_HOLD_TIME_MS = 1000;
unsigned long SAVE_RESET_HOLD_TIME_MS = 1000;
int tsSampleRate = 10;
