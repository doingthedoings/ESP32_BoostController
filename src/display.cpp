#include "definitions.h"
#include "config.h"

//================================================================================
// DISPLAY UPDATE
//================================================================================
void drawMenuList(const char* title, const MenuItem menuItems[], int count, int selectedIndex) {
    display.setTextSize(1);
    drawCenteredString(title, 2);
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

    const int itemsOnScreen = 3;
    const int itemHeight = 10;
    if (selectedIndex < menuScrollOffset) { menuScrollOffset = selectedIndex; }
    if (selectedIndex >= menuScrollOffset + itemsOnScreen) { menuScrollOffset = selectedIndex - itemsOnScreen + 1;
    }

    for (int i = 0; i < itemsOnScreen; i++) {
        int itemIndex = menuScrollOffset + i;
        if (itemIndex >= count) break;
        int yPos = 16 + i * itemHeight;
        
        char valueStr[20];
        const MenuItem* item = &menuItems[itemIndex];
        if (item->type == P_FLOAT) { dtostrf(*(float*)item->valuePtr, 1, item->precision, valueStr);
        } 
        else if (item->type == P_INT) { itoa(*(int*)item->valuePtr, valueStr, 10);
        } 
        else { ultoa(*(unsigned long*)item->valuePtr, valueStr, 10);
        }

        if (itemIndex == selectedIndex) {
            display.setCursor(5, yPos);
            display.print(">");
            display.setCursor(12, yPos); display.print(item->label);
            drawRightAlignedString(valueStr, yPos);
        } else {
            display.setCursor(5, yPos);
            display.print(item->label);
            drawRightAlignedString(valueStr, yPos);
        }
    }
}


void updateDisplay() {
    static bool isDisplayOn = true;
    static ScreenState lastDrawnScreen = (ScreenState)-1; 
    static float last_pressurekPa = -1.0;
    static float last_targetkPa = -1.0;
    static float last_controlPercent = -1.0;
    static float last_peakHoldkPa = -1.0;
    static float last_spoolScore = -1.0;
    static float last_torqueScore = -1.0;


    bool shouldDisplayBeOn = true;
    if(xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) { shouldDisplayBeOn = !isDisplayAsleep; xSemaphoreGive(dataMutex); }
    
    if (shouldDisplayBeOn != isDisplayOn) {
        if (shouldDisplayBeOn) display.ssd1306_command(SSD1306_DISPLAYON);
        else display.ssd1306_command(SSD1306_DISPLAYOFF);
        isDisplayOn = shouldDisplayBeOn;
    }

    if (!isDisplayOn) return;
    
    float local_targetkPa, local_pressurekPa, local_controlPercent, local_peakHoldkPa, local_spoolScore, local_torqueScore;
    if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
        local_targetkPa = targetkPa;
        local_pressurekPa = pressurekPa;
        local_peakHoldkPa = peakHoldkPa;
        local_controlPercent = controlPercent;
        local_spoolScore = spoolScore;
        local_torqueScore = torqueScore;
        xSemaphoreGive(dataMutex);
    } else { return; }
    
    char buffer[32];
    switch (currentScreen) {
        case MAIN_SCREEN:
            if (lastDrawnScreen != MAIN_SCREEN) { 
                display.clearDisplay();
                display.setTextSize(1);
                display.setCursor(2, 2); display.print("Target:");
                display.setCursor(2, 12); display.print("Actual:");
                display.setCursor(2, 22); display.print("Duty:");
                display.setCursor(2, 32); display.print("Peak-hold:");
                display.setCursor(2, 42); display.print("SS:");
                drawActionLabels();
                last_targetkPa = -1.0;
                last_pressurekPa = -1.0;
                last_controlPercent = -1.0;
                last_peakHoldkPa = -1.0;
                last_spoolScore = -1.0;
                last_torqueScore = -1.0;
            }

            if (local_targetkPa != last_targetkPa) {
                display.fillRect(68, 2, 60, 8, SSD1306_BLACK);
                dtostrf(local_targetkPa, 3, 0, buffer); strcat(buffer, " kPa");
                drawRightAlignedString(buffer, 2);
                last_targetkPa = local_targetkPa;
            }
            if (abs(local_pressurekPa - last_pressurekPa) > 0.05) { 
                display.fillRect(68, 12, 60, 8, SSD1306_BLACK);
                dtostrf(local_pressurekPa, 4, 1, buffer); strcat(buffer, " kPa");
                drawRightAlignedString(buffer, 12);
                last_pressurekPa = local_pressurekPa;
            }
            if (abs(local_controlPercent - last_controlPercent) > 0.05) { 
                display.fillRect(68, 22, 60, 8, SSD1306_BLACK);
                dtostrf(local_controlPercent, 3, 0, buffer); strcat(buffer, "%");
                drawRightAlignedString(buffer, 22);
                last_controlPercent = local_controlPercent;
            }
             if (abs(local_peakHoldkPa - last_peakHoldkPa) > 0.05) { 
                display.fillRect(68, 32, 60, 8, SSD1306_BLACK);
                dtostrf(local_peakHoldkPa, 4, 1, buffer); strcat(buffer, " kPa");
                drawRightAlignedString(buffer, 32);
                last_peakHoldkPa = local_peakHoldkPa;
            }
            if (abs(local_spoolScore - last_spoolScore) > 0.05) {
                display.fillRect(20, 42, 46, 8, SSD1306_BLACK);
                display.setCursor(20, 42);
                dtostrf(local_spoolScore, 4, 0, buffer);
                display.print(buffer);
                last_spoolScore = local_spoolScore;
            }
            if (abs(local_torqueScore - last_torqueScore) > 0.05) {
                display.fillRect(68, 42, 60, 8, SSD1306_BLACK);
                char ts_buffer[20];
                dtostrf(local_torqueScore, 4, 0, buffer);
                sprintf(ts_buffer, "TS:%s", buffer);
                drawRightAlignedString(ts_buffer, 42);
                last_torqueScore = local_torqueScore;
            }
            drawHoldIndicator();
            break;
        case EDIT_SETPOINT:
            display.clearDisplay();
            display.setTextSize(2); drawCenteredString("SET BOOST", 5);
            display.drawFastHLine(0, 22, 128, SSD1306_WHITE);
            dtostrf(local_targetkPa, 3, 0, buffer); strcat(buffer, "kPa");
            display.setTextSize(3); drawCenteredString(buffer, 27);
            drawActionLabels();
            drawHoldIndicator(); 
            break;
        case CONFIG_MENU:
            {
                display.clearDisplay();
                display.setTextSize(1); drawCenteredString("Configuration", 2);
                display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
                const char* titles[] = {"PID Tuning", "MAP Sensor", "Filtering/Misc"};
                for(int i=0; i<3; i++) {
                    int yPos = 16 + i * 10;
                    if (i == configMenuIndex) {
                        display.setCursor(5, yPos);
                        display.print(">");
                        display.setCursor(12, yPos); display.print(titles[i]);
                    } else {
                        display.setCursor(5, yPos);
                        display.print(titles[i]);
                    }
                }
                drawActionLabels();
            }
            break;
        case PID_TUNING_MENU:
            display.clearDisplay();
            drawMenuList("PID Tuning", pidMenuItems, pidMenuCount, pidMenuIndex);
            drawActionLabels();
            drawHoldIndicator(); 
            break;
        case MAP_SENSOR_MENU:
            display.clearDisplay();
            drawMenuList("MAP Sensor", mapMenuItems, mapMenuCount, mapMenuIndex);
            drawActionLabels();
            drawHoldIndicator(); 
            break;
        case FILTERING_MISC_MENU:
            display.clearDisplay();
            drawMenuList("Filtering/Misc", filterMenuItems, filterMenuCount, filterMenuIndex);
            drawActionLabels();
            drawHoldIndicator(); 
            break;
        case EDIT_PARAMETER:
            display.clearDisplay();
            display.setTextSize(1); drawCenteredString(currentParamName, 5);
            display.drawFastHLine(0, 16, 128, SSD1306_WHITE);
            display.setTextSize(2);
            dtostrf(tempEditValue, 1, currentParamPrecision, buffer);
            strcat(buffer, currentParamUnit);
            drawCenteredString(buffer, 28);
            drawActionLabels();
            break;
        case INFO_SCREEN:
            display.clearDisplay();
            wrapAndDrawText(currentInfoText, 2, 4, SCREEN_WIDTH - 4);
            drawActionLabels();
            break;
        case TUNE_SCORING_SCREEN:
            display.clearDisplay();
            display.setTextSize(1);

            // Column A Title
            display.setCursor(57, 1); display.print("A");
            // Column B Title
            display.setCursor(67, 1); display.print("B");

            display.drawFastVLine(64, 0, SCREEN_HEIGHT - 10, SSD1306_WHITE);

            // Column A content
            display.setCursor(1, 10); display.print("Spool-");
            display.setCursor(1, 20); display.print("Score:");
            dtostrf(spoolScoreA, 4, 0, buffer); display.print(buffer);

            display.setCursor(1, 35); display.print("Torque-");
            display.setCursor(1, 45); display.print("Score:");
            dtostrf(torqueScoreA, 4, 0, buffer); display.print(buffer);

            // Column B content
            display.setCursor(68, 10); display.print("Spool-");
            display.setCursor(68, 20); display.print("Score:");
            dtostrf(spoolScoreB, 4, 0, buffer); display.print(buffer);

            display.setCursor(68, 35); display.print("Torque-");
            display.setCursor(68, 45); display.print("Score:");
            dtostrf(torqueScoreB, 4, 0, buffer); display.print(buffer);

            drawActionLabels();
            drawTuneScoringHoldIndicator();
            break;
        case CONFIRMATION_SCREEN:
            display.clearDisplay();
            display.setTextSize(2);
            drawCenteredString(confirmationLine1, (SCREEN_HEIGHT / 2) - 16);
            drawCenteredString(confirmationLine2, (SCREEN_HEIGHT / 2));
            break;
    }
    
    display.display();
    displayNeedsUpdate = false;
    lastDrawnScreen = currentScreen;
}

void drawActionLabels() {
    display.setTextSize(1);
    
    char label1[5] = "A", label2[5] = "B", label3[5] = "TS";
    if (activeProfile == 'A') sprintf(label1, "*A*");
    if (activeProfile == 'B') sprintf(label2, "*B*");
    
    const char* labels[6] = {"", "", "", "", "", ""};
    bool active[6] = {false, false, false, false, false, false};

    switch(currentScreen) {
        case MAIN_SCREEN:
            labels[0]="EDIT"; labels[1]=label1; labels[2]=label2; labels[3]=label3; labels[4]="CFG"; labels[5]="CLR";
            for(int i=0; i<6; i++) active[i] = true;
            break;
        case EDIT_SETPOINT:
            labels[0]="BACK"; labels[1]="RST"; labels[2]="-"; labels[3]="+"; labels[4]="SAVE";
            for(int i=0; i<5; i++) active[i] = true;
            break;
        case CONFIG_MENU:
            labels[0]="BACK"; labels[2]="-"; labels[3]="+"; labels[5]="SEL";
            active[0]=true; active[2]=true; active[3]=true; active[5]=true;
            break;
        case PID_TUNING_MENU:
        case MAP_SENSOR_MENU:
        case FILTERING_MISC_MENU:
            labels[0]="BACK"; labels[1]="SAVE"; labels[2]="-"; labels[3]="+"; labels[4]="INFO"; labels[5]="SEL";
            for(int i=0; i<6; i++) active[i] = true;
            break;
        case EDIT_PARAMETER:
            labels[0]="BACK"; labels[2]="-"; labels[3]="+"; labels[5]="OK";
            active[0]=true; active[2]=true; active[3]=true; active[5]=true;
            break;
        case INFO_SCREEN:
            labels[0]="BACK"; active[0]=true;
            break;
        case TUNE_SCORING_SCREEN:
            labels[0]="BACK"; labels[1]="SaveA"; labels[4]="SaveB";
            active[0]=true; active[1]=true; active[4]=true;
            break;
        case CONFIRMATION_SCREEN:
            break;
    }

    int16_t x1, y1; uint16_t w, h;
    int box_height = 9;
    int y_pos = SCREEN_HEIGHT - box_height;
    
    display.fillRect(0, y_pos, SCREEN_WIDTH, box_height, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);

    uint16_t widths[6] = {0};
    int total_text_w = 0;
    for (int i = 0; i < 6; i++) {
        if (active[i]) {
            display.getTextBounds(labels[i], 0, 0, &x1, &y1, &widths[i], &h);
            total_text_w += widths[i];
        }
    }

    int box_widths[6];
    int total_w = 0;
    for(int i=0; i<6; i++) {
        box_widths[i] = widths[i];
        total_w += box_widths[i];
    }

    int total_gaps_w = 5;
    int remaining_space = SCREEN_WIDTH - total_w - total_gaps_w;
    
    int padding_per_cell = remaining_space / 6;
    int extra_pixels = remaining_space % 6;

    for(int i=0; i<6; i++) {
        box_widths[i] += padding_per_cell;
        if(extra_pixels > 0) {
            box_widths[i]++;
            extra_pixels--;
        }
    }

    int current_x = 0;
    for (int i = 0; i < 6; i++) {
        int box_w = box_widths[i];

        if (active[i]) {
            int x_offset = (box_w - widths[i]) / 2;
            if (strcmp(labels[i], "+") == 0) {
                x_offset++;
            }
            display.setCursor(current_x + x_offset, y_pos + 1);
            display.print(labels[i]);
        }
        
        current_x += box_w;
        if (i < 5) {
            display.drawFastVLine(current_x, y_pos, box_height, SSD1306_BLACK);
            current_x++;
        }
    }
    display.setTextColor(SSD1306_WHITE);
}

void drawHoldIndicator() {
    static bool wasHolding = false;
    unsigned long startTime = 0;
    unsigned long holdTime = 0;
    const int box_height = 9;
    const int y_pos = SCREEN_HEIGHT - box_height;
    
    if (currentScreen == MAIN_SCREEN) {
        if (editHoldStart != 0) { startTime = editHoldStart; holdTime = EDIT_HOLD_TIME_MS; }
        else if (cfgHoldStart != 0) { startTime = cfgHoldStart; holdTime = EDIT_HOLD_TIME_MS; }
    } else if (currentScreen == EDIT_SETPOINT) {
        if (saveHoldStart != 0) { startTime = saveHoldStart; holdTime = SAVE_RESET_HOLD_TIME_MS; }
        else if (resetHoldStart != 0) { startTime = resetHoldStart; holdTime = SAVE_RESET_HOLD_TIME_MS; }
    } else if (currentScreen >= PID_TUNING_MENU && currentScreen <= FILTERING_MISC_MENU) {
        if (cfgSaveHoldStart != 0) { startTime = cfgSaveHoldStart; holdTime = SAVE_RESET_HOLD_TIME_MS; }
    }

    if (startTime != 0) {
        long elapsed = millis() - startTime;
        if (elapsed < holdTime) {
            int maxBarHeight = y_pos - 2;
            int barHeight = map(elapsed, 0, holdTime, 0, maxBarHeight);
            display.drawLine(0, y_pos - 1, 0, y_pos - 1 - barHeight, SSD1306_WHITE);
            display.drawLine(SCREEN_WIDTH - 1, y_pos - 1, SCREEN_WIDTH - 1, y_pos - 1 - barHeight, SSD1306_WHITE);
        }
        wasHolding = true;
    } else if (wasHolding) {
        int maxBarHeight = y_pos;
        display.drawFastVLine(0, 0, maxBarHeight, SSD1306_BLACK);
        display.drawFastVLine(SCREEN_WIDTH - 1, 0, maxBarHeight, SSD1306_BLACK);
        wasHolding = false;
    }
}

void drawTuneScoringHoldIndicator() {
    static bool wasHoldingA = false;
    static bool wasHoldingB = false;
    unsigned long startTimeA = saveAHoldStart;
    unsigned long startTimeB = saveBHoldStart;
    const int box_height = 9;
    const int y_pos = SCREEN_HEIGHT - box_height;

    // Handle bar A
    if (startTimeA != 0) {
        long elapsed = millis() - startTimeA;
        if (elapsed < SAVE_RESET_HOLD_TIME_MS) {
            int maxBarHeight = y_pos - 2;
            int barHeight = map(elapsed, 0, SAVE_RESET_HOLD_TIME_MS, 0, maxBarHeight);
            display.drawLine(0, y_pos - 1, 0, y_pos - 1 - barHeight, SSD1306_WHITE);
        }
        wasHoldingA = true;
    } else if (wasHoldingA) {
        int maxBarHeight = y_pos;
        display.drawFastVLine(0, 0, maxBarHeight, SSD1306_BLACK);
        wasHoldingA = false;
    }

    // Handle bar B
    if (startTimeB != 0) {
        long elapsed = millis() - startTimeB;
        if (elapsed < SAVE_RESET_HOLD_TIME_MS) {
            int maxBarHeight = y_pos - 2;
            int barHeight = map(elapsed, 0, SAVE_RESET_HOLD_TIME_MS, 0, maxBarHeight);
            display.drawLine(SCREEN_WIDTH - 1, y_pos - 1, SCREEN_WIDTH - 1, y_pos - 1 - barHeight, SSD1306_WHITE);
        }
        wasHoldingB = true;
    } else if (wasHoldingB) {
        int maxBarHeight = y_pos;
        display.drawFastVLine(SCREEN_WIDTH - 1, 0, maxBarHeight, SSD1306_BLACK);
        wasHoldingB = false;
    }
}

void drawCenteredString(const String &text, int y) {
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, y);
    display.print(text);
}

void drawRightAlignedString(const String &text, int y, int maxX) {
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(maxX - w, y);
    display.print(text);
}

void wrapAndDrawText(const String& text, int x, int y, int maxWidth) {
    display.setTextSize(1);
    String currentLine = "";
    String currentWord = "";
    int line_height = 8;

    for (unsigned int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        if (c == ' ' || i == text.length() - 1) {
            if (c != ' ') currentWord += c;
            int16_t x1, y1; uint16_t w, h;
            String testLine = currentLine + (currentLine.length() > 0 ? " " : "") + currentWord;
            display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);

            if (w > maxWidth && currentLine.length() > 0) {
                display.setCursor(x, y);
                display.print(currentLine);
                y += line_height;
                if (y > SCREEN_HEIGHT - line_height - 10) { 
                    currentLine = currentWord + "...";
                    break;
                }
                currentLine = currentWord;
            } else {
                currentLine = testLine;
            }
            currentWord = "";
        } else {
            currentWord += c;
        }
    }
    display.setCursor(x, y);
    display.print(currentLine);
}