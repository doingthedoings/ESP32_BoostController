#include "definitions.h"
#include "config.h"

void calculateScaledVoltages() {
    float dividerRatio = R2_OHMS / (R1_OHMS + R2_OHMS);
    minSensorVoltage = RAW_MIN_SENSOR_VOLTAGE * dividerRatio;
    maxSensorVoltage = RAW_MAX_SENSOR_VOLTAGE * dividerRatio;
    scaledVoltageOffset = RAW_VOLTAGE_OFFSET * dividerRatio;
}

void calibrateTouchSensors() {
    const int DEVIATION_THRESHOLD = 10000; bool needsWarning = false;
    while (true) {
        int currentReadings[6]; 
        int minReading = 100000; int maxReading = 0;
        for (int i = 0; i < 6; i++) {
            currentReadings[i] = touchRead(touchPins[i]);
            if (currentReadings[i] < minReading) minReading = currentReadings[i];
            if (currentReadings[i] > maxReading) maxReading = currentReadings[i];
        }
        if ((maxReading - minReading) > DEVIATION_THRESHOLD) {
            if (!needsWarning) {
                 display.clearDisplay();
                 display.setTextSize(1);
                 drawCenteredString("DO NOT TOUCH BUTTONS", (SCREEN_HEIGHT / 2) - 5);
                 display.display(); needsWarning = true;
            }
            delay(100); continue;
        } else { break;
        }
    }
    display.clearDisplay(); display.setTextSize(1); drawCenteredString("Calibrating Input...", SCREEN_HEIGHT / 2); display.display();
    delay(500);
    long touch_sum[6] = {0}; 
    const int samples = 10;
    for (int s = 0; s < samples; s++) {
        for (int i = 0; i < 6; i++) { touch_sum[i] += touchRead(touchPins[i]);
        }
        delay(10);
    }
    for (int i = 0; i < 6; i++) { touchCalibrationValues[i] = touch_sum[i] / samples;
    }
    display.clearDisplay(); drawCenteredString("Calibration Complete!", (SCREEN_HEIGHT / 2) - 4); display.display();
    delay(1000);
}

float readOversampledVoltage() {
    uint32_t total_adc_value = 0;
    int oversample_count_local = OVERSAMPLE_COUNT;
    for (int i = 0; i < oversample_count_local; i++) {
        total_adc_value += analogRead(PRESSURE_SENSOR_PIN);
    }
    float avg_adc_value = (float)total_adc_value / oversample_count_local;
    return (avg_adc_value / 4095.0) * 3.3;
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool isPresetDataValid(const ControllerPreset& preset) {
    if (isnan(preset.targetkPa) || isinf(preset.targetkPa) || preset.targetkPa < 100.0) return false;
    if (isnan(preset.kp) || isinf(preset.kp)) return false;
    if (isnan(preset.ki) || isinf(preset.ki)) return false;
    if (isnan(preset.kd) || isinf(preset.kd)) return false;
    if (isnan(preset.maxIntegral) || isinf(preset.maxIntegral) || preset.maxIntegral < 0) return false;
    if (preset.valveFrequencyHz <= 0 || preset.valveFrequencyHz > 100) return false;
    if (isnan(preset.PID_Control_Overhead) || isinf(preset.PID_Control_Overhead)) return false;
    if (isnan(preset.pidTriggerkPa) || isinf(preset.pidTriggerkPa) || preset.pidTriggerkPa < 0) return false;
    if (isnan(preset.slow_ema_a) || isinf(preset.slow_ema_a) || preset.slow_ema_a <= 0.0 || preset.slow_ema_a >= 1.0) return false;
    if (isnan(preset.fast_ema_a) || isinf(preset.fast_ema_a) || preset.fast_ema_a <= 0.0 || preset.fast_ema_a >= 1.0) return false;
    if (isnan(preset.kpa_rate_change_threshold) || isinf(preset.kpa_rate_change_threshold) || preset.kpa_rate_change_threshold < 0) return false;
    if (preset.kpa_rate_time_interval_ms <= 0) return false;
    if (preset.OVERSAMPLE_COUNT <= 0) return false;
    if (isnan(preset.IDLE_TIMEOUT_SECONDS) || isinf(preset.IDLE_TIMEOUT_SECONDS) || preset.IDLE_TIMEOUT_SECONDS < 0) return false;
    if (isnan(preset.RAW_MIN_SENSOR_VOLTAGE) || isinf(preset.RAW_MIN_SENSOR_VOLTAGE)) return false;
    if (isnan(preset.RAW_MAX_SENSOR_VOLTAGE) || isinf(preset.RAW_MAX_SENSOR_VOLTAGE)) return false;
    if (isnan(preset.RAW_VOLTAGE_OFFSET) || isinf(preset.RAW_VOLTAGE_OFFSET)) return false;
    if (isnan(preset.MIN_KPA) || isinf(preset.MIN_KPA)) return false;
    if (isnan(preset.MAX_KPA) || isinf(preset.MAX_KPA) || preset.MAX_KPA <= preset.MIN_KPA) return false;
    if (isnan(preset.PRESSURE_CORRECTION_KPA) || isinf(preset.PRESSURE_CORRECTION_KPA)) return false;
    if (isnan(preset.spoolScore) || isinf(preset.spoolScore) || preset.spoolScore < 0) return false;
    if (isnan(preset.torqueScore) || isinf(preset.torqueScore) || preset.torqueScore < 0) return false;
    
    return true;
}
