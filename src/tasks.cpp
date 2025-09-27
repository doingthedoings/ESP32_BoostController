#include "config.h"

//================================================================================
// PID CONTROL TASK (Core 0)
//================================================================================
void pidControlTask(void *pvParameters) {
    float error = 0.0, lastError = 0.0, integral = 0.0, derivative = 0.0, output = 0.0;
    unsigned long lastTime = millis();
    float v_ema_s = 0, output_ema_s = 0;
    
    int local_kpa_rate_time_interval_ms;
    int local_valve_frequency;
    const int task_delay_ms = 10;
    const int HISTORY_SAMPLES_MAX = 20;
    int history_samples;
    float pressureHistory[HISTORY_SAMPLES_MAX];
    int historyIndex = 0;
    bool solenoidDisabledByIdle = false;
    unsigned long idleTimerStart = 0;

    int intervalTime;
    bool mosfetState = false;
    unsigned long controlLastTime = 0;

    local_valve_frequency = valveFrequencyHz;
    intervalTime = 1000 / local_valve_frequency;
    local_kpa_rate_time_interval_ms = kpa_rate_time_interval_ms;
    history_samples = min(local_kpa_rate_time_interval_ms / task_delay_ms, HISTORY_SAMPLES_MAX);

    float initialMeasuredVoltage = readOversampledVoltage();
    float initialVoltage = initialMeasuredVoltage - scaledVoltageOffset;
    v_ema_s = initialVoltage;
    float initialPressure = fmap(initialVoltage, minSensorVoltage, maxSensorVoltage, MIN_KPA, MAX_KPA) + PRESSURE_CORRECTION_KPA;
    for(int i = 0; i < history_samples; i++) {
        pressureHistory[i] = initialPressure;
    }

    for (;;) {
        if (millis() % 1000 < task_delay_ms) {
            local_valve_frequency = valveFrequencyHz;
            intervalTime = 1000 / local_valve_frequency;
            local_kpa_rate_time_interval_ms = kpa_rate_time_interval_ms;
            history_samples = min(local_kpa_rate_time_interval_ms / task_delay_ms, HISTORY_SAMPLES_MAX);
        }

        float measuredVoltage = readOversampledVoltage();
        float sensorVoltage = measuredVoltage - scaledVoltageOffset;
        float rawPressure = fmap(sensorVoltage, minSensorVoltage, maxSensorVoltage, MIN_KPA, MAX_KPA) + PRESSURE_CORRECTION_KPA;
        
        int lookbackIndex = (historyIndex + 1) % history_samples;
        float pressureChange = abs(rawPressure - pressureHistory[lookbackIndex]);
        float current_alpha = (pressureChange > kpa_rate_change_threshold) ? fast_ema_a : slow_ema_a;
        v_ema_s = (current_alpha * sensorVoltage) + ((1 - current_alpha) * v_ema_s);
        
        pressureHistory[historyIndex] = rawPressure;
        historyIndex = (historyIndex + 1) % history_samples;
        
        float currentPressure = fmap(v_ema_s, minSensorVoltage, maxSensorVoltage, MIN_KPA, MAX_KPA) + PRESSURE_CORRECTION_KPA;
        bool activityDetected = false;
        if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            if(userActivity) {
                activityDetected = true;
                userActivity = false;
            }
            xSemaphoreGive(dataMutex);
        }
        if (activityDetected) {
            idleTimerStart = 0;
        }

        if (currentPressure > IDLE_PRESSURE_MIN_KPA && currentPressure < IDLE_PRESSURE_MAX_KPA || currentPressure < MIN_KPA) {
            if (idleTimerStart == 0) {
                idleTimerStart = millis();
            } else if (millis() - idleTimerStart > (IDLE_TIMEOUT_SECONDS * 1000)) {
                solenoidDisabledByIdle = true;
                if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) { isDisplayAsleep = true; xSemaphoreGive(dataMutex); }
            }
        } else {
            idleTimerStart = 0;
        }

        if (solenoidDisabledByIdle && currentPressure < REACTIVATE_PRESSURE_KPA) {
            solenoidDisabledByIdle = false;
            if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) { isDisplayAsleep = false; xSemaphoreGive(dataMutex); }
        }

        float localTargetkPa;
        if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            pressurekPa = currentPressure;
            if (currentPressure > peakHoldkPa) {
                peakHoldkPa = currentPressure;
            }
            localTargetkPa = targetkPa;
            xSemaphoreGive(dataMutex);
        }

        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - lastTime;

        static int armingSampleCounter = 0;

        switch (spoolState) {
            case SPOOL_IDLE:
                if (currentPressure > ARMING_THRESHOLD_KPA) {
                    spoolState = SPOOL_ARMING;
                    armingSampleCounter = 1;
                }
                break;

            case SPOOL_ARMING:
                if (currentPressure > ARMING_THRESHOLD_KPA) {
                    armingSampleCounter++;
                    if (armingSampleCounter >= ARMING_DWELL_SAMPLES) {
                        p_start = currentPressure;
                        t_start = currentTime;
                        p_peak = p_start;
                        t_peak = t_start;
                        boostEventData.clear();
                        boostEventData.push_back({p_start, t_start});
                        spoolState = SPOOL_LOGGING;
                    }
                } else {
                    spoolState = SPOOL_IDLE; 
                }
                break;

            case SPOOL_LOGGING:
                if (boostEventData.size() < MAX_BOOST_EVENT_SAMPLES) {
                    boostEventData.push_back({currentPressure, currentTime});
                }
                if (currentPressure > p_peak) {
                    p_peak = currentPressure;
                    t_peak = currentTime;
                }
                if (currentPressure < (p_peak - TERMINATION_DROP_KPA)) {
                    spoolState = SPOOL_CALCULATE_AND_DISPLAY;
                }
                break;
            
            case SPOOL_CALCULATE_AND_DISPLAY:
                break;
        }


        if (currentPressure < localTargetkPa - pidTriggerkPa) {
            output = 255.0;
            integral = 0;
        } else {
            error = (localTargetkPa - currentPressure) + PID_Control_Overhead;
            integral += error * (elapsedTime / 1000.0);
            if (abs(integral) > maxIntegral) {
                integral = maxIntegral * (integral > 0 ? 1 : -1);
            }
            derivative = (error - lastError) / (elapsedTime / 1000.0);
            output = (kp * error) + (ki * integral) + (kd * derivative);
        }
        
        lastError = error;
        lastTime = currentTime;
        output = constrain(output, 0, 255);
        output_ema_s = (output_ema_a * output) + ((1 - output_ema_a) * output_ema_s);
        float localControlPercent = fmap(output_ema_s, 0.0, 255.0, 0.0, 100.0);

        if (solenoidDisabledByIdle) {
            localControlPercent = 0;
        }

        if (spoolState == SPOOL_CALCULATE_AND_DISPLAY) {
            float maxRate = 0.0;
            if (boostEventData.size() > 1) {
                for (size_t i = 1; i < boostEventData.size(); ++i) {
                    vTaskDelay(1); // Yield to prevent watchdog timeout
                    float p_delta = boostEventData[i].pressure - boostEventData[i-1].pressure;
                    unsigned long t_delta = boostEventData[i].timestamp - boostEventData[i-1].timestamp;
                    if (t_delta > 0) {
                        float rate = (p_delta / t_delta) * 1000.0;
                        if (rate > maxRate) {
                            maxRate = rate;
                        }
                    }
                }
            }
            if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                if (maxRate > spoolScore) { spoolScore = maxRate; }
                if (activeProfile == 'A') {
                    if (spoolScore > spoolScoreA) { spoolScoreA = spoolScore; }
                    if (spoolScore > spoolScoreA) { 
                        spoolScoreA = spoolScore;
                        saveScoresForProfile(0);
                    }
                } else {
                    if (spoolScore > spoolScoreB) { 
                        spoolScoreB = spoolScore;
                        saveScoresForProfile(1);
                    }
                }
                xSemaphoreGive(dataMutex);
            }
            spoolState = SPOOL_IDLE;
        }

        // Torque Score Calculation
        if (torqueState == TORQUE_IDLE && currentPressure > ARMING_THRESHOLD_KPA) {
            torqueState = TORQUE_LOGGING;
            boostEventData.clear();
            torqueLoggingStartTime = currentTime;
        }

        if (torqueState == TORQUE_LOGGING) {
            if (currentTime - torqueLoggingStartTime > 3000 && currentPressure < localTargetkPa) {
                torqueState = TORQUE_IDLE;
                boostEventData.clear();
            } else {
                if (boostEventData.size() < MAX_BOOST_EVENT_SAMPLES) {
                    boostEventData.push_back({currentPressure, currentTime});
                }
                if (currentPressure < (p_peak - TERMINATION_DROP_KPA)) {
                    torqueState = TORQUE_CALCULATE_AND_DISPLAY;
                }
            }
        }

        if (torqueState == TORQUE_CALCULATE_AND_DISPLAY) {
            float totalScore = 0;
            unsigned long setpointTime = 0;
            if (boostEventData.size() > 1) {
                for (size_t i = 1; i < boostEventData.size(); ++i) {
                    vTaskDelay(1); // Yield to prevent watchdog timeout
                    float p_avg = (boostEventData[i].pressure + boostEventData[i-1].pressure) / 2.0;
                    unsigned long t_delta = boostEventData[i].timestamp - boostEventData[i-1].timestamp;
                    
                    if (p_avg >= localTargetkPa && setpointTime == 0) {
                        setpointTime = boostEventData[i].timestamp;
                    }

                    if (setpointTime > 0 && boostEventData[i].timestamp - setpointTime > torqueScoreCutoffMs) {
                        break;
                    }

                    float area = (p_avg - 100.0) * t_delta; // Area above atmospheric pressure

                    if (p_avg > localTargetkPa) { // Overshoot penalty
                        area -= (p_avg - localTargetkPa) * t_delta * 2.0; // Penalize overshoot more heavily
                    }
                    totalScore += area;
                }
            }
            if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                float scaledTorqueScore = totalScore / 1000.0;
                if (scaledTorqueScore > torqueScore) { torqueScore = scaledTorqueScore; }
                if (activeProfile == 'A') {
                    if (torqueScore > torqueScoreA) {
                        torqueScoreA = torqueScore;
                        saveScoresForProfile(0);
                    }
                } else {
                    if (torqueScore > torqueScoreB) {
                        torqueScoreB = torqueScore;
                        saveScoresForProfile(1);
                    }
                }
                xSemaphoreGive(dataMutex);
            }
            torqueState = TORQUE_IDLE;
        }

        if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            controlPercent = localControlPercent;
            if (currentScreen == MAIN_SCREEN || currentScreen == TUNE_SCORING_SCREEN) {
                displayNeedsUpdate = true;
            }
            xSemaphoreGive(dataMutex);
        }
        
        if (localControlPercent <= 1.0) {
            if (mosfetState) { digitalWrite(SOLENOID_PIN, LOW); mosfetState = false; }
        } else if (localControlPercent >= 99.0) {
            if (!mosfetState) { digitalWrite(SOLENOID_PIN, HIGH); mosfetState = true; }
        } else {
            int onTime = localControlPercent / 100.0 * intervalTime;
            int offTime = intervalTime - onTime;
            unsigned long controlCurrentTime = millis();
            unsigned long controlElapsedTime = controlCurrentTime - controlLastTime;
            if (mosfetState && controlElapsedTime >= onTime) {
                digitalWrite(SOLENOID_PIN, LOW);
                mosfetState = false;
                controlLastTime = controlCurrentTime;
            } else if (!mosfetState && controlElapsedTime >= offTime) {
                digitalWrite(SOLENOID_PIN, HIGH);
                mosfetState = true;
                controlLastTime = controlCurrentTime;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(tsSampleRate));
    }
}

//================================================================================
// DISPLAY & INPUT TASK (Core 1)
//================================================================================
void displayAndInputTask(void *pvParameters) {
    for (;;) {
        if (currentScreen == CONFIRMATION_SCREEN && millis() >= confirmationEndTime) {
            currentScreen = screenAfterConfirmation;
            displayNeedsUpdate = true;
        }
        handleTouchInputs();
        if (displayNeedsUpdate) {
            updateDisplay();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
