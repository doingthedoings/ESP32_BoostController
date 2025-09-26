#include <Arduino.h>
#include "definitions.h"
#include "config.h"

//================================================================================
// FUNCTION PROTOTYPES
//================================================================================
void setup();
void loop();

//================================================================================
// SETUP (Core 1)
//================================================================================
void setup() {
    Serial.begin(115200);
    
    Wire.begin(OLED_SDA, OLED_SCK);
    Wire.setClock(400000);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(DISPLAY_BRIGHTNESS);
    
    calibrateTouchSensors();
    
    pinMode(TOUCH_PIN_6, INPUT);
    unsigned long startTime = millis();
    bool manualResetRequested = true; 

    while (millis() - startTime < 3000) {
        if (touchRead(TOUCH_PIN_6) < (touchCalibrationValues[5] + TOUCH_SENSITIVITY_OFFSET)) {
            manualResetRequested = false; 
            break;
        }
        delay(50); 
    }

    if (manualResetRequested) {
        drawCenteredString("FACTORY RESET...", SCREEN_HEIGHT / 2);
        display.display();
        EEPROM.begin(EEPROM_SIZE);
        initializeDefaultParameters();
        delay(2000); 
    }

    if (!manualResetRequested) { 
        EEPROM.begin(EEPROM_SIZE);
    }

    if (EEPROM.read(ADDR_INITIALIZED) != 'V') {
        initializeDefaultParameters();
    } else {
        loadAllParameters();
        int lastActivePreset = -1;
        EEPROM.get(ADDR_ACTIVE_PRESET, lastActivePreset);
        if (lastActivePreset >= 0 && lastActivePreset <= 1) {
            ControllerPreset presetToLoad;
            EEPROM.get(ADDR_PRESET_1 + (lastActivePreset * sizeof(ControllerPreset)), presetToLoad);
            if (isPresetDataValid(presetToLoad)) {
                copyPresetToGlobals(presetToLoad);
                activePresetIndex = lastActivePreset;
                activeProfile = (lastActivePreset == 0) ? 'A' : 'B';
            } else {
                activePresetIndex = -1;
            }
        } else {
            activePresetIndex = -1;
        }
    }
    
    // Load scores for display
    ControllerPreset tempPreset;
    EEPROM.get(ADDR_PRESET_1, tempPreset);
    if (isPresetDataValid(tempPreset)) {
        spoolScoreA = tempPreset.spoolScore;
        torqueScoreA = tempPreset.torqueScore;
    }
    EEPROM.get(ADDR_PRESET_2, tempPreset);
    if (isPresetDataValid(tempPreset)) {
        spoolScoreB = tempPreset.spoolScore;
        torqueScoreB = tempPreset.torqueScore;
    }
    
    peakHoldkPa = 100.0f;

    calculateScaledVoltages();
    pinMode(SOLENOID_PIN, OUTPUT);
    digitalWrite(SOLENOID_PIN, LOW);
    analogReadResolution(12);

    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == NULL) {
        Serial.println("Mutex creation failed!");
    }

    xTaskCreatePinnedToCore(pidControlTask, "PID Control", 4096, NULL, 2, &pidControlTaskHandle, 0);
    xTaskCreatePinnedToCore(displayAndInputTask, "Display & Input", 4096, NULL, 1, &displayAndInputTaskHandle, 1);

    vTaskDelete(NULL);
}

//================================================================================
// MAIN LOOP (Core 1)
//================================================================================
void loop() {
    // Intentionally left empty. All work is done in FreeRTOS tasks.
    vTaskDelay(portMAX_DELAY);
}
