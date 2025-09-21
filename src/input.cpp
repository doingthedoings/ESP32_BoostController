#include "config.h"

//================================================================================
// TOUCH INPUT HANDLING
//================================================================================
void handleTouchInputs() {
    static unsigned long lastInstantActionTime = 0;
    static bool waitForRelease = false;

    int rawReadings[6];
    bool isAnyButtonPressed = false;
    for (int i=0; i < 6; i++) {
        rawReadings[i] = touchRead(touchPins[i]);
        if(rawReadings[i] > (touchCalibrationValues[i] + TOUCH_SENSITIVITY_OFFSET)) {
            isAnyButtonPressed = true;
        }
    }

    if (waitForRelease) {
        if (!isAnyButtonPressed) {
            waitForRelease = false;
        }
        return;
    }

    bool displayWasAsleep = false;
    if(xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
        displayWasAsleep = isDisplayAsleep;
        if(isDisplayAsleep && isAnyButtonPressed) { isDisplayAsleep = false; userActivity = true;
        displayNeedsUpdate = true;}
        xSemaphoreGive(dataMutex);
    }
    if(displayWasAsleep && isAnyButtonPressed) return;
    
    bool infoButtonPressed = rawReadings[4] > (touchCalibrationValues[4] + TOUCH_SENSITIVITY_OFFSET);
    static bool wasInfoButtonPressed = false;
    if (currentScreen >= PID_TUNING_MENU && currentScreen <= FILTERING_MISC_MENU) {
        if (infoButtonPressed && !wasInfoButtonPressed) {
            lastMenuScreen = currentScreen;
            const MenuItem* menuItems;
            int* currentIndex;
            if(currentScreen == PID_TUNING_MENU) { menuItems = pidMenuItems; currentIndex = &pidMenuIndex;
            }
            else if(currentScreen == MAP_SENSOR_MENU) { menuItems = mapMenuItems;
            currentIndex = &mapMenuIndex; }
            else { menuItems = filterMenuItems;
            currentIndex = &filterMenuIndex; }
            currentInfoText = menuItems[*currentIndex].info;
            currentScreen = INFO_SCREEN;
            displayNeedsUpdate = true;
        }
        if (infoButtonPressed) {
            wasInfoButtonPressed = true;
            return;
        }
    }
    
    if(currentScreen == INFO_SCREEN && !infoButtonPressed && wasInfoButtonPressed) {
        currentScreen = lastMenuScreen;
        displayNeedsUpdate = true;
    }
    wasInfoButtonPressed = infoButtonPressed;

    switch(currentScreen) {
        case MAIN_SCREEN:
            if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) {
                if (editHoldStart == 0) editHoldStart = millis();
                else if (millis() - editHoldStart > EDIT_HOLD_TIME_MS) { 
                    currentScreen = EDIT_SETPOINT;
                    editHoldStart = 0; 
                    displayNeedsUpdate = true; 
                    waitForRelease = true;
                }
            } else { 
                if (editHoldStart != 0) displayNeedsUpdate = true;
                editHoldStart = 0;
            }
            if (rawReadings[4] > (touchCalibrationValues[4] + TOUCH_SENSITIVITY_OFFSET)) {
                if (cfgHoldStart == 0) cfgHoldStart = millis();
                else if (millis() - cfgHoldStart > EDIT_HOLD_TIME_MS) { 
                    currentScreen = CONFIG_MENU;
                    cfgHoldStart = 0; 
                    displayNeedsUpdate = true; 
                    waitForRelease = true;
                }
            } else { 
                if (cfgHoldStart != 0) displayNeedsUpdate = true;
                cfgHoldStart = 0;
            }
            if (rawReadings[5] > (touchCalibrationValues[5] + TOUCH_SENSITIVITY_OFFSET)) {
                if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                    if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                        peakHoldkPa = pressurekPa; 
                        spoolScore = 0.0;
                        torqueScore = 0.0;
                        xSemaphoreGive(dataMutex);
                    }
                    lastInstantActionTime = millis();
                    displayNeedsUpdate = true;
                }
            }
            // A button
            if (rawReadings[1] > (touchCalibrationValues[1] + TOUCH_SENSITIVITY_OFFSET)) {
                if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                    loadPreset(0);
                    activeProfile = 'A';
                    lastInstantActionTime = millis();
                }
            }
            // B button
            if (rawReadings[2] > (touchCalibrationValues[2] + TOUCH_SENSITIVITY_OFFSET)) {
                if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                    loadPreset(1);
                    activeProfile = 'B';
                    lastInstantActionTime = millis();
                }
            }
            // TS button
            if (rawReadings[3] > (touchCalibrationValues[3] + TOUCH_SENSITIVITY_OFFSET)) {
                if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                    currentScreen = TUNE_SCORING_SCREEN;
                    lastInstantActionTime = millis();
                    displayNeedsUpdate = true;
                }
            }
            break;
        case EDIT_SETPOINT:
            if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                if (rawReadings[3] > (touchCalibrationValues[3] + TOUCH_SENSITIVITY_OFFSET)) {
                    if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) { targetkPa += 1.0;
                    xSemaphoreGive(dataMutex); }
                    lastInstantActionTime = millis();
                    displayNeedsUpdate = true;
                } else if (rawReadings[2] > (touchCalibrationValues[2] + TOUCH_SENSITIVITY_OFFSET)) {
                    if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
                        targetkPa = max(100.0f, targetkPa - 1.0f);
                        xSemaphoreGive(dataMutex);
                    }
                    lastInstantActionTime = millis();
                    displayNeedsUpdate = true;
                } else if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) {
                    currentScreen = MAIN_SCREEN;
                    lastInstantActionTime = millis(); displayNeedsUpdate = true;
                }
            }
            if (rawReadings[4] > (touchCalibrationValues[4] + TOUCH_SENSITIVITY_OFFSET)) {
                if (saveHoldStart == 0) saveHoldStart = millis();
                else if (millis() - saveHoldStart > SAVE_RESET_HOLD_TIME_MS) {
                    saveAllParameters();
                    activePresetIndex = -1; // These are now custom settings
                    EEPROM.put(ADDR_ACTIVE_PRESET, activePresetIndex);
                    EEPROM.commit();
                    showConfirmationScreen("SETTINGS", "SAVED!", 1500, MAIN_SCREEN);
                    saveHoldStart = 0;
                    waitForRelease = true;
                }
            } else { 
                if (saveHoldStart != 0) displayNeedsUpdate = true;
                saveHoldStart = 0; 
            }
            if (rawReadings[1] > (touchCalibrationValues[1] + TOUCH_SENSITIVITY_OFFSET)) {
                if (resetHoldStart == 0) resetHoldStart = millis();
                else if (millis() - resetHoldStart > SAVE_RESET_HOLD_TIME_MS) {
                    if(xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) { targetkPa = defaultTargetkPa;
                    xSemaphoreGive(dataMutex); }
                    saveAllParameters();
                    showConfirmationScreen("SETPOINT", "RESET!", 1500, MAIN_SCREEN);
                    resetHoldStart = 0;
                    waitForRelease = true;
                }
            } else { 
                if (resetHoldStart != 0) displayNeedsUpdate = true;
                resetHoldStart = 0; 
            }
            break;
        case CONFIG_MENU:
            if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                if (rawReadings[2] > (touchCalibrationValues[2] + TOUCH_SENSITIVITY_OFFSET)) { configMenuIndex = max(0, configMenuIndex - 1);
                lastInstantActionTime = millis(); displayNeedsUpdate = true; } 
                else if (rawReadings[3] > (touchCalibrationValues[3] + TOUCH_SENSITIVITY_OFFSET)) { configMenuIndex = min(2, configMenuIndex + 1);
                lastInstantActionTime = millis(); displayNeedsUpdate = true; } 
                else if (rawReadings[5] > (touchCalibrationValues[5] + TOUCH_SENSITIVITY_OFFSET)) {
                    menuScrollOffset = 0;
                    if (configMenuIndex == 0) currentScreen = PID_TUNING_MENU;
                    else if (configMenuIndex == 1) currentScreen = MAP_SENSOR_MENU;
                    else if (configMenuIndex == 2) currentScreen = FILTERING_MISC_MENU;
                    lastInstantActionTime = millis(); displayNeedsUpdate = true;
                }
                else if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) {
                     currentScreen = MAIN_SCREEN;
                     lastInstantActionTime = millis(); displayNeedsUpdate = true;
                }
            }
            break;
        case PID_TUNING_MENU:
        case MAP_SENSOR_MENU:
        case FILTERING_MISC_MENU:
            {
                int* currentIndex;
                int maxIndex; const MenuItem* menuItems;
                if(currentScreen == PID_TUNING_MENU) { currentIndex = &pidMenuIndex; maxIndex = pidMenuCount - 1; menuItems = pidMenuItems;
                }
                else if(currentScreen == MAP_SENSOR_MENU) { currentIndex = &mapMenuIndex;
                maxIndex = mapMenuCount - 1; menuItems = mapMenuItems; }
                else { currentIndex = &filterMenuIndex;
                maxIndex = filterMenuCount - 1; menuItems = filterMenuItems; }

                if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                    if (rawReadings[2] > (touchCalibrationValues[2] + TOUCH_SENSITIVITY_OFFSET)) { *currentIndex = max(0, *currentIndex - 1);
                    lastInstantActionTime = millis(); displayNeedsUpdate = true; } 
                    else if (rawReadings[3] > (touchCalibrationValues[3] + TOUCH_SENSITIVITY_OFFSET)) { *currentIndex = min(maxIndex, *currentIndex + 1);
                    lastInstantActionTime = millis(); displayNeedsUpdate = true; } 
                    else if (rawReadings[5] > (touchCalibrationValues[5] + TOUCH_SENSITIVITY_OFFSET)) {
                        lastMenuScreen = currentScreen;
                        lastMenuIndex = *currentIndex; lastScrollOffset = menuScrollOffset;
                        const MenuItem* item = &menuItems[*currentIndex];
                        currentParamName = item->label; currentParamUnit = item->unit; currentParamPrecision = item->precision;
                        currentEditingValuePtr = item->valuePtr; currentEditingType = item->type;
                        
                        if(currentEditingType == P_FLOAT) tempEditValue = *(float*)currentEditingValuePtr;
                        else if(currentEditingType == P_INT) tempEditValue = *(int*)currentEditingValuePtr;
                        else tempEditValue = *(unsigned long*)currentEditingValuePtr;
                        
                        currentScreen = EDIT_PARAMETER;
                        lastInstantActionTime = millis(); displayNeedsUpdate = true;
                    }
                    else if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) { 
                        currentScreen = CONFIG_MENU;
                        lastInstantActionTime = millis(); displayNeedsUpdate = true;
                    }
                }
                if (rawReadings[1] > (touchCalibrationValues[1] + TOUCH_SENSITIVITY_OFFSET)) {
                    if (cfgSaveHoldStart == 0) cfgSaveHoldStart = millis();
                    else if (millis() - cfgSaveHoldStart > SAVE_RESET_HOLD_TIME_MS) {
                        invalidatePresetScores();
                        saveAllParameters();
                        calculateScaledVoltages();
                        
                        activePresetIndex = -1;
                        EEPROM.put(ADDR_ACTIVE_PRESET, activePresetIndex);
                        EEPROM.commit();

                        showConfirmationScreen("SETTINGS", "SAVED!", 1500, MAIN_SCREEN);
                        cfgSaveHoldStart = 0;
                        waitForRelease = true;
                    }
                } else { 
                    if (cfgSaveHoldStart != 0) displayNeedsUpdate = true;
                    cfgSaveHoldStart = 0; 
                }
            }
            break;
        case EDIT_PARAMETER:
            {
                static int holdCount = 0;
                static unsigned long lastRepeatTime = 0; const unsigned long REPEAT_DELAY = 100;
                float step = 0.01;
                if (currentEditingType == P_FLOAT) {
                    if (currentParamPrecision == 0) step = 1.0;
                    else if (currentParamPrecision == 1) step = 0.1;
                    else if (currentParamPrecision == 2) step = 0.01;
                    else if (currentParamPrecision == 3) step = 0.001;
                    else step = 0.0001;
                } else if(currentEditingType == P_INT) { step = 1.0; } else { step = 50;
                }
                if (holdCount > 10) step *= 2;
                if (holdCount > 20) step *= 5; if (holdCount > 30) step *= 10;
                
                if (rawReadings[3] > (touchCalibrationValues[3] + TOUCH_SENSITIVITY_OFFSET)) {
                    if (holdCount == 0 || millis() - lastRepeatTime > REPEAT_DELAY) { tempEditValue += step;
                    holdCount++; lastRepeatTime = millis(); displayNeedsUpdate = true; }
                } else if (rawReadings[2] > (touchCalibrationValues[2] + TOUCH_SENSITIVITY_OFFSET)) {
                    if (holdCount == 0 || millis() - lastRepeatTime > REPEAT_DELAY) { tempEditValue -= step;
                    holdCount++; lastRepeatTime = millis(); displayNeedsUpdate = true; }
                } else { holdCount = 0;
                }
                
                if (tempEditValue < 0 && currentEditingValuePtr != &RAW_VOLTAGE_OFFSET && currentEditingValuePtr != &PRESSURE_CORRECTION_KPA) { tempEditValue = 0;
                }
                
                if (rawReadings[5] > (touchCalibrationValues[5] + TOUCH_SENSITIVITY_OFFSET)) {
                    if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                        if(currentEditingType == P_FLOAT) *(float*)currentEditingValuePtr = tempEditValue;
                        else if(currentEditingType == P_INT) *(int*)currentEditingValuePtr = (int)tempEditValue;
                        else *(unsigned long*)currentEditingValuePtr = (unsigned long)tempEditValue;
                        currentScreen = lastMenuScreen;
                        if(currentScreen == PID_TUNING_MENU) pidMenuIndex = lastMenuIndex;
                        else if(currentScreen == MAP_SENSOR_MENU) mapMenuIndex = lastMenuIndex;
                        else filterMenuIndex = lastMenuIndex;
                        menuScrollOffset = lastScrollOffset;
                        lastInstantActionTime = millis(); displayNeedsUpdate = true;
                    }
                }
                 if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) {
                    if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                        currentScreen = lastMenuScreen;
                        if(currentScreen == PID_TUNING_MENU) pidMenuIndex = lastMenuIndex;
                        else if(currentScreen == MAP_SENSOR_MENU) mapMenuIndex = lastMenuIndex;
                        else filterMenuIndex = lastMenuIndex;
                        menuScrollOffset = lastScrollOffset;
                        lastInstantActionTime = millis(); displayNeedsUpdate = true;
                    }
                }
            }
            break;
        case TUNE_SCORING_SCREEN:
            if (millis() - lastInstantActionTime > DEBOUNCE_DELAY) {
                if (rawReadings[0] > (touchCalibrationValues[0] + TOUCH_SENSITIVITY_OFFSET)) {
                    currentScreen = MAIN_SCREEN;
                    lastInstantActionTime = millis();
                    displayNeedsUpdate = true;
                }
            }

            // Save A
            if (rawReadings[1] > (touchCalibrationValues[1] + TOUCH_SENSITIVITY_OFFSET)) {
                if (saveAHoldStart == 0) saveAHoldStart = millis();
                else if (millis() - saveAHoldStart > SAVE_RESET_HOLD_TIME_MS) {
                    saveScoresForPreset(0);
                    saveAHoldStart = 0;
                    waitForRelease = true;
                }
            } else {
                if (saveAHoldStart != 0) displayNeedsUpdate = true;
                saveAHoldStart = 0;
            }

            // Save B
            if (rawReadings[4] > (touchCalibrationValues[4] + TOUCH_SENSITIVITY_OFFSET)) {
                if (saveBHoldStart == 0) saveBHoldStart = millis();
                else if (millis() - saveBHoldStart > SAVE_RESET_HOLD_TIME_MS) {
                    saveScoresForPreset(1);
                    saveBHoldStart = 0;
                    waitForRelease = true;
                }
            } else {
                if (saveBHoldStart != 0) displayNeedsUpdate = true;
                saveBHoldStart = 0;
            }
            break;
        case INFO_SCREEN:
            break;
        case CONFIRMATION_SCREEN:
            break;
    }
}
