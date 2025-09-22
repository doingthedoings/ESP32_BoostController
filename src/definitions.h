#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <vector>
#include <cmath>

//================================================================================
// PIN & HARDWARE DEFINITIONS
//================================================================================
#define OLED_SDA 2
#define OLED_SCK 3
#define SOLENOID_PIN 1
#define PRESSURE_SENSOR_PIN 6
#define TOUCH_PIN_1 7
#define TOUCH_PIN_2 8
#define TOUCH_PIN_3 9
#define TOUCH_PIN_4 10
#define TOUCH_PIN_5 11
#define TOUCH_PIN_6 12
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define MAX_BOOST_EVENT_SAMPLES 1000

//================================================================================
// EEPROM ADDRESSES
//================================================================================
#define EEPROM_SIZE 512
#define ADDR_TARGET_KPA 0
#define ADDR_KP 4
#define ADDR_KI 8
#define ADDR_KD 12
#define ADDR_INITIALIZED 16
#define ADDR_MAX_INTEGRAL 20
#define ADDR_PID_TRIGGER_KPA 24
#define ADDR_PID_OVERHEAD 28
#define ADDR_VALVE_FREQ 32
#define ADDR_PRESSURE_CORRECTION 36
#define ADDR_MIN_KPA 40
#define ADDR_MAX_KPA 44
#define ADDR_RAW_VOLTAGE_OFFSET 48
#define ADDR_RAW_MIN_VOLTAGE 52
#define ADDR_RAW_MAX_VOLTAGE 56
#define ADDR_SLOW_EMA_A 60
#define ADDR_FAST_EMA_A 64
#define ADDR_KPA_RATE_THRESH 68
#define ADDR_KPA_RATE_INTERVAL 72
#define ADDR_OVERSAMPLE_COUNT 76
#define ADDR_SAVE_RESET_HOLD 80
#define ADDR_EDIT_HOLD 84
#define ADDR_IDLE_TIMEOUT 88
#define ADDR_ACTIVE_PRESET 92
#define ADDR_PRESET_1 96
#define ADDR_PRESET_2 (ADDR_PRESET_1 + sizeof(ControllerPreset))
#define ADDR_PRESET_2 (ADDR_PRESET_1 + sizeof(ControllerPreset))

//================================================================================
// STRUCT & ENUM DEFINITIONS
//================================================================================

struct ControllerPreset {
    float targetkPa;
    float kp, ki, kd;
    float maxIntegral;
    int valveFrequencyHz;
    float PID_Control_Overhead;
    float pidTriggerkPa;
    float slow_ema_a;
    float fast_ema_a;
    float kpa_rate_change_threshold;
    int kpa_rate_time_interval_ms;
    int OVERSAMPLE_COUNT;
    float IDLE_TIMEOUT_SECONDS;
    float RAW_MIN_SENSOR_VOLTAGE;
    float RAW_MAX_SENSOR_VOLTAGE;
    float RAW_VOLTAGE_OFFSET;
    float MIN_KPA;
    float MAX_KPA;
    float PRESSURE_CORRECTION_KPA;
    float spoolScore;
    float torqueScore;
};

struct PressureTimestamp {
    float pressure;
    unsigned long timestamp;
};

enum ScreenState {
    MAIN_SCREEN,
    EDIT_SETPOINT,
    CONFIG_MENU,
    PID_TUNING_MENU,
    MAP_SENSOR_MENU,
    FILTERING_MISC_MENU,
    EDIT_PARAMETER,
    INFO_SCREEN,
    TUNE_SCORING_SCREEN,
    CONFIRMATION_SCREEN
};

enum SpoolScoreState {
    SPOOL_IDLE,
    SPOOL_ARMING,
    SPOOL_LOGGING,
    SPOOL_CALCULATE_AND_DISPLAY
};

enum TorqueScoreState {
    TORQUE_IDLE,
    TORQUE_LOGGING,
    TORQUE_CALCULATE_AND_DISPLAY
};

enum ParamType { P_FLOAT, P_INT, P_ULONG };

struct MenuItem {
    const char* label;
    void* valuePtr;
    ParamType type;
    int precision;
    const char* unit;
    const char* info;
};

//================================================================================
// EXTERNAL VARIABLE DECLARATIONS
//================================================================================

// -- Hardware --
extern Adafruit_SSD1306 display;
extern const int touchPins[];
extern int touchCalibrationValues[6];

// -- RTOS --
extern SemaphoreHandle_t dataMutex;
extern TaskHandle_t pidControlTaskHandle;
extern TaskHandle_t displayAndInputTaskHandle;

// -- System State --
extern float targetkPa;
extern float pressurekPa;
extern float peakHoldkPa;
extern float spoolScore;
extern float torqueScore;
extern float spoolScoreA, spoolScoreB;
extern float torqueScoreA, torqueScoreB;
extern char activeProfile;
extern float controlPercent;
extern bool isDisplayAsleep;
extern bool userActivity;
extern bool displayNeedsUpdate;

// -- Score State Machines --
extern SpoolScoreState spoolState;
extern TorqueScoreState torqueState;
extern std::vector<PressureTimestamp> boostEventData;
extern float p_start, p_peak;
extern unsigned long t_start, t_peak;

// -- Preset Management --
extern ControllerPreset presets[2];
extern int activePresetIndex;

// -- UI State --
extern ScreenState currentScreen;
extern unsigned long editHoldStart, saveHoldStart, resetHoldStart, cfgHoldStart, cfgSaveHoldStart, saveAHoldStart, saveBHoldStart;
extern int configMenuIndex, pidMenuIndex, mapMenuIndex, filterMenuIndex, menuScrollOffset;
extern ScreenState lastMenuScreen;
extern int lastMenuIndex, lastScrollOffset;
extern void* currentEditingValuePtr;
extern ParamType currentEditingType;
extern float tempEditValue;
extern const char* currentParamName;
extern const char* currentParamUnit;
extern int currentParamPrecision;
extern const char* currentInfoText;
extern char confirmationLine1[20], confirmationLine2[20];
extern unsigned long confirmationEndTime;
extern ScreenState screenAfterConfirmation;

// -- Adjustable Parameters (declared in config.h, defined in globals.cpp) --
extern int DISPLAY_BRIGHTNESS;
extern float kp, ki, kd;
extern float defaultTargetkPa;
extern float maxIntegral;
extern int valveFrequencyHz;
extern float PID_Control_Overhead;
extern float pidTriggerkPa;
extern float slow_ema_a, fast_ema_a;
extern float kpa_rate_change_threshold;
extern int kpa_rate_time_interval_ms;
extern float output_ema_a;
extern int OVERSAMPLE_COUNT;
extern float IDLE_TIMEOUT_SECONDS;
extern float RAW_MIN_SENSOR_VOLTAGE, RAW_MAX_SENSOR_VOLTAGE, RAW_VOLTAGE_OFFSET;
extern float minSensorVoltage, maxSensorVoltage, scaledVoltageOffset;
extern float MIN_KPA, MAX_KPA;
extern float PRESSURE_CORRECTION_KPA;
extern unsigned long EDIT_HOLD_TIME_MS, SAVE_RESET_HOLD_TIME_MS;
extern int tsSampleRate;

//================================================================================
// FUNCTION PROTOTYPES
//================================================================================

// -- Main Tasks --
void pidControlTask(void *pvParameters);
void displayAndInputTask(void *pvParameters);

// -- Display --
void updateDisplay();
void drawMenuList(const char* title, const MenuItem menuItems[], int count, int selectedIndex);
void drawActionLabels();
void drawHoldIndicator();
void drawTuneScoringHoldIndicator();
void drawCenteredString(const String &text, int y);
void drawRightAlignedString(const String &text, int y, int maxX = SCREEN_WIDTH);
void wrapAndDrawText(const String& text, int x, int y, int maxWidth);

// -- Input --
void handleTouchInputs();
void calibrateTouchSensors();

// -- Persistence --

void loadPreset(int index);
void saveScoresForPreset(int index);
void invalidatePresetScores();
void saveAllParameters();
void loadAllParameters();
void initializeDefaultParameters();
void copyGlobalsToPreset(ControllerPreset& preset);
void copyPresetToGlobals(const ControllerPreset& preset);
void showConfirmationScreen(const char* line1, const char* line2, unsigned long duration, ScreenState nextScreen);

// -- Helpers --
void calculateScaledVoltages();
float readOversampledVoltage();
float fmap(float x, float in_min, float in_max, float out_min, float out_max);
bool isPresetDataValid(const ControllerPreset& preset);

#endif // DEFINITIONS_H
