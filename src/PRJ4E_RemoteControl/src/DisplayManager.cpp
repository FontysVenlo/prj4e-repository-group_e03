#include "DisplayManager.h"

DisplayManager::DisplayManager(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display) 
    : _display(display) {}

void DisplayManager::init() {
    _display.begin();
    drawStaticUI(); // Draw static elements once
}

void DisplayManager::toggleScreen() {
    _currentScreen = (_currentScreen == TEMP_SCREEN) ? MENU_SCREEN : TEMP_SCREEN;
    if (_currentScreen == TEMP_SCREEN) {
        _display.clearBuffer();
        drawStaticUI(); // Redraw static elements for temp screen
    } else {
        drawMenu();
    }
}

void DisplayManager::updateTemperature(float tempC) {
    if (_currentScreen != TEMP_SCREEN) return; // Only update if on temp screen
    
    _display.clearBuffer();
    drawStaticUI();
    
    // Temperature value
    char tempBuf[8];
    if (tempC == DEVICE_DISCONNECTED_C) {
        strcpy(tempBuf, "Err");
    } else {
        snprintf(tempBuf, sizeof(tempBuf), "%.1f", tempC);
    }

    _display.setFont(u8g2_font_fub30_tr);
    int16_t tempW = _display.getStrWidth(tempBuf);
    _display.drawStr((128 - tempW) / 2, 48, tempBuf);

    if (tempC != DEVICE_DISCONNECTED_C) {
        _display.setFont(u8g2_font_6x12_tr);
        _display.drawStr((128 - tempW) / 2 + tempW + 2, 48, "°C");
    }

    drawThermometer(tempC);
    _display.sendBuffer();
}

void DisplayManager::drawMenu() {
    _display.clearBuffer();
    _display.drawFrame(0, 0, 128, 64);
    _display.setFont(u8g2_font_6x12_tr);
    _display.drawStr(10, 20, "MENU");
    _display.drawStr(10, 40, "> Temp Monitor");
    _display.drawStr(10, 60, "  Settings (TODO)");
    _display.sendBuffer();
}

void DisplayManager::drawStaticUI() {
    _display.drawFrame(0, 0, 128, 64);
    _display.setFont(u8g2_font_6x12_tr);
    const char *header = "Temp";
    _display.drawStr((128 - _display.getStrWidth(header)) / 2, 12, header);
}

void DisplayManager::drawThermometer(float tempC) {
    if (tempC == DEVICE_DISCONNECTED_C) return;

    const int totalSegments = 32;
    const float minTemp = 0.0f;
    const float maxTemp = 40.0f;

    int16_t bulbX = 8 + 6;
    int16_t bulbY = 55;
    int16_t bulbR = 6;

    int16_t stemW = 5;
    int16_t stemTop = 18;
    int16_t stemBottom = bulbY - bulbR;
    int16_t stemX = bulbX - (stemW / 2);

    _display.drawCircle(bulbX, bulbY, bulbR);
    _display.drawFrame(stemX, stemTop, stemW, (stemBottom - stemTop));

    float clamped = tempC;
    if (clamped < minTemp) clamped = minTemp;
    if (clamped > maxTemp) clamped = maxTemp;

    float pct = (clamped - minTemp) / (maxTemp - minTemp);
    int fillSegments = (int)round(pct * totalSegments);
    fillSegments = max(0, min(totalSegments, fillSegments));

    int16_t stemHeight = (stemBottom - stemTop - 2);
    float pixelsPerStep = (float)stemHeight / (float)totalSegments;
    int16_t fillPx = (int16_t)round(fillSegments * pixelsPerStep);

    if (fillPx > 0) {
        int16_t yStart = stemBottom - 1 - fillPx;
        int16_t boxX = stemX + 1;
        int16_t boxW = stemW - 2;
        _display.drawBox(boxX, yStart, boxW, fillPx);
    }

    _display.drawDisc(bulbX, bulbY, bulbR - 1);
}