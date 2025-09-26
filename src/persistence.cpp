#include "config.h"

//================================================================================
// HELPER AND PRESET FUNCTIONS
//================================================================================
void showConfirmationScreen(const char* line1, const char* line2, unsigned long duration, ScreenState nextScreen) {
    strncpy(confirmationLine1, line1, sizeof(confirmationLine1) - 1);
    confirmationLine1[sizeof(confirmationLine1) - 1] = '\0';
    strncpy(confirmationLine2, line2, sizeof(confirmationLine2) - 1);
    confirmationLine2[sizeof(confirmationLine2) - 1] = '\0';
    
    confirmationEndTime = millis() + duration;
    screenAfterConfirmation = nextScreen;
    currentScreen = CONFIRMATION_SCREEN;
    displayNeedsUpdate = true;
}

void copyGlobalsToPreset(ControllerPreset& preset) {
    preset.targetkPa = targetkPa;
    preset.kp = kp; preset.ki = ki; preset.kd = kd;
    preset.maxIntegral = maxIntegral;
    preset.valveFrequencyHz = valveFrequencyHz;
    preset.PID_Control_Overhead = PID_Control_Overhead;
    preset.pidTriggerkPa = pidTriggerkPa;
    preset.slow_ema_a = slow_ema_a;
    preset.fast_ema_a = fast_ema_a;
    preset.kpa_rate_change_threshold = kpa_rate_change_threshold;
    preset.kpa_rate_time_interval_ms = kpa_rate_time_interval_ms;
    preset.OVERSAMPLE_COUNT = OVERSAMPLE_COUNT;
    preset.IDLE_TIMEOUT_SECONDS = IDLE_TIMEOUT_SECONDS;
    preset.RAW_MIN_SENSOR_VOLTAGE = RAW_MIN_SENSOR_VOLTAGE;
    preset.RAW_MAX_SENSOR_VOLTAGE = RAW_MAX_SENSOR_VOLTAGE;
    preset.RAW_VOLTAGE_OFFSET = RAW_VOLTAGE_OFFSET;
    preset.MIN_KPA = MIN_KPA;
    preset.MAX_KPA = MAX_KPA;
    preset.PRESSURE_CORRECTION_KPA = PRESSURE_CORRECTION_KPA;
    preset.spoolScore = 0.0;
    preset.torqueScore = 0.0;
}

void copyPresetToGlobals(const ControllerPreset& preset) {
    targetkPa = preset.targetkPa;
    kp = preset.kp; ki = preset.ki; kd = preset.kd;
    maxIntegral = preset.maxIntegral;
    valveFrequencyHz = preset.valveFrequencyHz;
    PID_Control_Overhead = preset.PID_Control_Overhead;
    pidTriggerkPa = preset.pidTriggerkPa;
    slow_ema_a = preset.slow_ema_a;
    fast_ema_a = preset.fast_ema_a;
    kpa_rate_change_threshold = preset.kpa_rate_change_threshold;
    kpa_rate_time_interval_ms = preset.kpa_rate_time_interval_ms;
    OVERSAMPLE_COUNT = preset.OVERSAMPLE_COUNT;
    IDLE_TIMEOUT_SECONDS = preset.IDLE_TIMEOUT_SECONDS;
    RAW_MIN_SENSOR_VOLTAGE = preset.RAW_MIN_SENSOR_VOLTAGE;
    RAW_MAX_SENSOR_VOLTAGE = preset.RAW_MAX_SENSOR_VOLTAGE;
    RAW_VOLTAGE_OFFSET = preset.RAW_VOLTAGE_OFFSET;
    MIN_KPA = preset.MIN_KPA;
    MAX_KPA = preset.MAX_KPA;
    PRESSURE_CORRECTION_KPA = preset.PRESSURE_CORRECTION_KPA;

    // Copy the scores for both profiles
    if (activePresetIndex == 0) {
        spoolScoreA = preset.spoolScore;
        torqueScoreA = preset.torqueScore;
    } else {
        spoolScoreB = preset.spoolScore;
        torqueScoreB = preset.torqueScore;
    }
    
    calculateScaledVoltages();
}

void saveTargetPressure() {
    EEPROM.put(ADDR_TARGET_KPA, targetkPa);
    if (!EEPROM.commit()) {
        Serial.println("EEPROM commit failed");
        showConfirmationScreen("EEPROM", "SAVE FAIL", 2000, MAIN_SCREEN);
    } else {
        showConfirmationScreen("NEW TARGET", "PRESSURE SAVED", 1500, MAIN_SCREEN);
    }
}

void loadPreset(int index) {
    if (index < 0 || index > 1) return;
    ControllerPreset presetToLoad;
    EEPROM.get(ADDR_PRESET_1 + (index * sizeof(ControllerPreset)), presetToLoad);

    if (!isPresetDataValid(presetToLoad)) {
        char line1[20];
        sprintf(line1, "PROFILE %c BAD", (index == 0 ? 'A' : 'B'));
        showConfirmationScreen(line1, "RESET TO DEFAULTS", 2500, MAIN_SCREEN);
        
        initializeDefaultParameters(); 
        copyGlobalsToPreset(presetToLoad); 
        EEPROM.put(ADDR_PRESET_1 + (index * sizeof(ControllerPreset)), presetToLoad); 
        if (!EEPROM.commit()) {
            Serial.println("Preset fix commit failed");
        }
        return; 
    }

    copyPresetToGlobals(presetToLoad);
    activePresetIndex = index;
    EEPROM.put(ADDR_ACTIVE_PRESET, activePresetIndex);
    if (!EEPROM.commit()) {
        Serial.println("Preset Load Commit Failed");
    }
    
    char line1[20];
    sprintf(line1, "PROFILE %c", (index == 0 ? 'A' : 'B'));
    showConfirmationScreen(line1, "LOADED", 1500, MAIN_SCREEN);
}

void saveAllParameters() {
    EEPROM.put(ADDR_TARGET_KPA, targetkPa);
    EEPROM.put(ADDR_KP, kp); EEPROM.put(ADDR_KI, ki); EEPROM.put(ADDR_KD, kd);
    EEPROM.put(ADDR_MAX_INTEGRAL, maxIntegral); EEPROM.put(ADDR_PID_TRIGGER_KPA, pidTriggerkPa);
    EEPROM.put(ADDR_PID_OVERHEAD, PID_Control_Overhead); EEPROM.put(ADDR_VALVE_FREQ, valveFrequencyHz);
    EEPROM.put(ADDR_PRESSURE_CORRECTION, PRESSURE_CORRECTION_KPA); EEPROM.put(ADDR_MIN_KPA, MIN_KPA);
    EEPROM.put(ADDR_MAX_KPA, MAX_KPA); EEPROM.put(ADDR_RAW_VOLTAGE_OFFSET, RAW_VOLTAGE_OFFSET);
    EEPROM.put(ADDR_RAW_MIN_VOLTAGE, RAW_MIN_SENSOR_VOLTAGE); EEPROM.put(ADDR_RAW_MAX_VOLTAGE, RAW_MAX_SENSOR_VOLTAGE);
    EEPROM.put(ADDR_SLOW_EMA_A, slow_ema_a); EEPROM.put(ADDR_FAST_EMA_A, fast_ema_a);
    EEPROM.put(ADDR_KPA_RATE_THRESH, kpa_rate_change_threshold); EEPROM.put(ADDR_KPA_RATE_INTERVAL, kpa_rate_time_interval_ms);
    EEPROM.put(ADDR_OVERSAMPLE_COUNT, OVERSAMPLE_COUNT); EEPROM.put(ADDR_SAVE_RESET_HOLD, SAVE_RESET_HOLD_TIME_MS);
    EEPROM.put(ADDR_EDIT_HOLD, EDIT_HOLD_TIME_MS); EEPROM.put(ADDR_IDLE_TIMEOUT, IDLE_TIMEOUT_SECONDS);
    EEPROM.put(ADDR_TS_CUTOFF, torqueScoreCutoffMs);
    if (!EEPROM.commit()) {
        Serial.println("EEPROM commit failed");
        showConfirmationScreen("EEPROM", "SAVE FAIL", 2000, MAIN_SCREEN);
    }
}

void loadAllParameters() {
    EEPROM.get(ADDR_TARGET_KPA, targetkPa); EEPROM.get(ADDR_KP, kp);
    EEPROM.get(ADDR_KI, ki);
    EEPROM.get(ADDR_KD, kd); EEPROM.get(ADDR_MAX_INTEGRAL, maxIntegral); EEPROM.get(ADDR_PID_TRIGGER_KPA, pidTriggerkPa);
    EEPROM.get(ADDR_PID_OVERHEAD, PID_Control_Overhead); EEPROM.get(ADDR_VALVE_FREQ, valveFrequencyHz);
    EEPROM.get(ADDR_PRESSURE_CORRECTION, PRESSURE_CORRECTION_KPA); EEPROM.get(ADDR_MIN_KPA, MIN_KPA);
    EEPROM.get(ADDR_MAX_KPA, MAX_KPA); EEPROM.get(ADDR_RAW_VOLTAGE_OFFSET, RAW_VOLTAGE_OFFSET);
    EEPROM.get(ADDR_RAW_MIN_VOLTAGE, RAW_MIN_SENSOR_VOLTAGE); EEPROM.get(ADDR_RAW_MAX_VOLTAGE, RAW_MAX_SENSOR_VOLTAGE);
    EEPROM.get(ADDR_SLOW_EMA_A, slow_ema_a); EEPROM.get(ADDR_FAST_EMA_A, fast_ema_a);
    EEPROM.get(ADDR_KPA_RATE_THRESH, kpa_rate_change_threshold); EEPROM.get(ADDR_KPA_RATE_INTERVAL, kpa_rate_time_interval_ms);
    EEPROM.get(ADDR_OVERSAMPLE_COUNT, OVERSAMPLE_COUNT); EEPROM.get(ADDR_SAVE_RESET_HOLD, SAVE_RESET_HOLD_TIME_MS);
    EEPROM.get(ADDR_EDIT_HOLD, EDIT_HOLD_TIME_MS); EEPROM.get(ADDR_IDLE_TIMEOUT, IDLE_TIMEOUT_SECONDS);
    EEPROM.get(ADDR_TS_CUTOFF, torqueScoreCutoffMs);
    if (torqueScoreCutoffMs < 0 || torqueScoreCutoffMs > 10000) {
        torqueScoreCutoffMs = defaultTorqueScoreCutoffMs;
    }
}

void initializeDefaultParameters(){
    EEPROM.write(ADDR_INITIALIZED, 'V');
    
    targetkPa = 170.0;
    kp = 10.0; ki = 0.1; kd = 1.0;
    maxIntegral = 600.0;
    valveFrequencyHz = 33;
    PID_Control_Overhead = 6.0;
    pidTriggerkPa = 20.0;
    slow_ema_a = 0.01;
    fast_ema_a = 0.3;
    kpa_rate_change_threshold = 10.0;
    kpa_rate_time_interval_ms = 50;
    OVERSAMPLE_COUNT = 256;
    IDLE_TIMEOUT_SECONDS = 60;
    RAW_MIN_SENSOR_VOLTAGE = 0.4;
    RAW_MAX_SENSOR_VOLTAGE = 4.65;
    RAW_VOLTAGE_OFFSET = -0.09643;
    MIN_KPA = 20.0;
    MAX_KPA = 300.0;
    PRESSURE_CORRECTION_KPA = 1.27;
    EDIT_HOLD_TIME_MS = 1000;
    SAVE_RESET_HOLD_TIME_MS = 1000;
    
    saveAllParameters();
    
    ControllerPreset defaultPreset;
    copyGlobalsToPreset(defaultPreset);
    defaultPreset.spoolScore = 0.0;
    defaultPreset.torqueScore = 0.0;
    for (int i = 0; i < 2; i++) {
        presets[i] = defaultPreset;
        EEPROM.put(ADDR_PRESET_1 + (i * sizeof(ControllerPreset)), presets[i]);
    }
    
    activePresetIndex = 0;
    EEPROM.put(ADDR_ACTIVE_PRESET, activePresetIndex);
    
    if (!EEPROM.commit()) {
        Serial.println("Initial EEPROM Commit Failed");
    }
    Serial.println("EEPROM Initialized with default values and presets.");
}

void invalidatePresetScores() {
    ControllerPreset preset;
    // Invalidate Preset A
    EEPROM.get(ADDR_PRESET_1, preset);
    preset.spoolScore = 0.0;
    preset.torqueScore = 0.0;
    EEPROM.put(ADDR_PRESET_1, preset);

    // Invalidate Preset B
    EEPROM.get(ADDR_PRESET_2, preset);
    preset.spoolScore = 0.0;
    preset.torqueScore = 0.0;
    EEPROM.put(ADDR_PRESET_2, preset);

    if (!EEPROM.commit()) {
        Serial.println("Score invalidation commit failed");
    }

    // Also reset the live score variables
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        spoolScoreA = 0.0;
        torqueScoreA = 0.0;
        spoolScoreB = 0.0;
        torqueScoreB = 0.0;
        xSemaphoreGive(dataMutex);
    }
}

void saveCurrentConfigToProfile(int index) {
    if (index < 0 || index > 1) return;

    ControllerPreset preset;
    copyGlobalsToPreset(preset); // Copy current settings into the preset

    // Use the live score values for the active profile
    if (activePresetIndex == 0) { // Profile A
        preset.spoolScore = spoolScoreA;
        preset.torqueScore = torqueScoreA;
    } else { // Profile B
        preset.spoolScore = spoolScoreB;
        preset.torqueScore = torqueScoreB;
    }

    EEPROM.put(ADDR_PRESET_1 + (index * sizeof(ControllerPreset)), preset);

    if (!EEPROM.commit()) {
        Serial.println("Config save commit failed");
        showConfirmationScreen("EEPROM", "SAVE FAIL", 2000, TUNE_SCORING_SCREEN);
    } else {
        char line1[20];
        sprintf(line1, "PROFILE %c CONFIG", (index == 0 ? 'A' : 'B'));
        showConfirmationScreen(line1, "SAVED", 1500, TUNE_SCORING_SCREEN);
    }
}
