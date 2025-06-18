#include "DisplayManager.h"
#include <DallasTemperature.h>


DisplayManager::DisplayManager(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display)
    : _display(display) {}

void DisplayManager::init() {
    _display.begin();
    drawStaticUI();
}

void DisplayManager::goToMenuScreen() {
    _currentScreen = MENU_SCREEN;
    _blinkVisible = true;
    _lastBlinkToggle = millis();
    drawMenu();
}

void DisplayManager::goToTempScreen() {
    _currentScreen = TEMP_SCREEN;
    _display.clearBuffer();
    drawStaticUI();
}

void DisplayManager::goToSetTempScreen() {
    _currentScreen = SET_TEMP_SCREEN;
    _blinkOn = true;
    _lastBlinkTime = millis();
    updateSetTempScreen(0);  // force initial draw
}

bool DisplayManager::isMenuScreen() const {
    return _currentScreen == MENU_SCREEN;
}

bool DisplayManager::isSetTempScreen() const {
    return _currentScreen == SET_TEMP_SCREEN;
}

int DisplayManager::getSelectedIndex() const {
    return _selectedIndex;
}

void DisplayManager::moveSelection(int direction) {
    _selectedIndex += direction;
    if (_selectedIndex < 0) _selectedIndex = _menuItemCount - 1;
    if (_selectedIndex >= _menuItemCount) _selectedIndex = 0;
    drawMenu();
}

void DisplayManager::updateTemperature(float tempC) {
    if (_currentScreen != TEMP_SCREEN) return;

    // Static variables to keep state between calls
    static float lastValidTemp = DEVICE_DISCONNECTED_C;
    static unsigned long lastValidTimestamp = 0;

    float displayTemp = tempC;

    if (tempC != DEVICE_DISCONNECTED_C) {
        lastValidTemp = tempC;
        lastValidTimestamp = millis();
    } else {
        if (millis() - lastValidTimestamp < 3000 && lastValidTemp != DEVICE_DISCONNECTED_C) {
            displayTemp = lastValidTemp;
        } else {
            displayTemp = DEVICE_DISCONNECTED_C;
        }
    }

    _display.clearBuffer();
    drawStaticUI();

    char tempBuf[8];
    snprintf(tempBuf, sizeof(tempBuf), (displayTemp == DEVICE_DISCONNECTED_C) ? "Err" : "%.1f", displayTemp);

    _display.setFont(u8g2_font_fub30_tr);
    int tempW = _display.getStrWidth(tempBuf);
    _display.drawStr((128 - tempW) / 2, 48, tempBuf);

    if (displayTemp != DEVICE_DISCONNECTED_C) {
        _display.setFont(u8g2_font_6x12_tr);
        _display.drawStr((128 - tempW) / 2 + tempW + 2, 48, "°C");
    }

    drawThermometer(displayTemp);
    _display.sendBuffer();
}


void DisplayManager::drawStaticUI() {
    _display.drawFrame(0, 0, 128, 64);
    _display.setFont(u8g2_font_6x12_tr);
    _display.drawStr((128 - _display.getStrWidth("Temp")) / 2, 12, "Temp");
}

void DisplayManager::drawMenu() {
    _display.clearBuffer();
    _display.drawFrame(0, 0, 128, 64);
    _display.setFont(u8g2_font_6x12_tr);
    _display.drawStr((128 - _display.getStrWidth("Menu")) / 2, 12, "Menu");

    const char* items[_menuItemCount] = {"Temp Monitor", "Set Temp", "Mode"};
    int baseY = 28;
    int spacing = 16;

    for (int i = 0; i < _menuItemCount; ++i) {
        if (i != _selectedIndex || _blinkVisible) {
            if (i == _selectedIndex) _display.drawStr(10, baseY + i * spacing, ">");
            _display.drawStr(20, baseY + i * spacing, items[i]);
        }
    }

    _display.sendBuffer();
}

void DisplayManager::drawThermometer(float tempC) {
    if (tempC == DEVICE_DISCONNECTED_C) return;

    const int totalSegments = 32;
    const float minTemp = 0.0f;
    const float maxTemp = 40.0f;

    int bulbX = 14, bulbY = 55, bulbR = 6;
    int stemW = 6, stemTop = 18, stemBottom = bulbY - bulbR;
    int stemX = bulbX - (stemW / 2) + 1;

    _display.drawCircle(bulbX, bulbY, bulbR);
    _display.drawFrame(stemX, stemTop, stemW, (stemBottom - stemTop));

    float clamped = constrain(tempC, minTemp, maxTemp);
    float pct = (clamped - minTemp) / (maxTemp - minTemp);
    int fillSegments = round(pct * totalSegments);
    int stemHeight = (stemBottom - stemTop - 2);
    int fillPx = round(fillSegments * (float)stemHeight / totalSegments);

    if (fillPx > 0) {
        int yStart = stemBottom - 1 - fillPx;
        _display.drawBox(stemX + 1, yStart, stemW - 2, fillPx);
    }

    _display.drawDisc(bulbX, bulbY, bulbR - 1);
}

void DisplayManager::tickBlink() {
    if (millis() - _lastBlinkToggle > 500) {
        _blinkVisible = !_blinkVisible;
        _lastBlinkToggle = millis();
        if (_currentScreen == MENU_SCREEN) drawMenu();
    }
}

void DisplayManager::updateSetTempScreen(float currentTemp) {
    if (_currentScreen != SET_TEMP_SCREEN) return;

    // --- Static buffer to avoid displaying "Err" too quickly
    static float lastValidTemp = DEVICE_DISCONNECTED_C;
    static unsigned long lastValidTimestamp = 0;

    float displayTemp = currentTemp;

    if (currentTemp != DEVICE_DISCONNECTED_C) {
        lastValidTemp = currentTemp;
        lastValidTimestamp = millis();
    } else {
        if (millis() - lastValidTimestamp < 3000 && lastValidTemp != DEVICE_DISCONNECTED_C) {
            displayTemp = lastValidTemp;
        } else {
            displayTemp = DEVICE_DISCONNECTED_C;
        }
    }

    // --- Now draw everything using displayTemp
    _display.clearBuffer();
    _display.drawFrame(0, 0, 128, 64);

    // Title
    _display.setFont(u8g2_font_6x12_tr);
    const char* header = "Set Temp";
    _display.drawStr((128 - _display.getStrWidth(header)) / 2, 12, header);

    // Initialize target temp if needed
    if (_targetTemp < 0.1f && displayTemp != DEVICE_DISCONNECTED_C) {
    _targetTemp = displayTemp;
    _editingTemp = displayTemp;
    }

    char currBuf[8], targetBuf[8];
    snprintf(currBuf, sizeof(currBuf), (displayTemp == DEVICE_DISCONNECTED_C) ? "Err" : "%.1f", displayTemp);
    snprintf(targetBuf, sizeof(targetBuf), "%.1f", _editingTemp);

    int middleY = 32;

    // Fonts and positions
    _display.setFont(u8g2_font_6x12_tr);
    int currW = _display.getStrWidth(currBuf);
    int targetW = _display.getStrWidth(targetBuf);

    int currX = 15;
    int arrowX = 64 - 6;
    int targetX = 128 - targetW - 15;

    // Draw temps
    _display.drawStr(currX, middleY, currBuf);
    _display.drawStr(arrowX, middleY, "→");
    _display.drawStr(targetX, middleY, targetBuf);

    // Circles for buttons
    int yBottom = 56, radius = 12;
    int spacing = (128 - 6 * radius) / 4;
    int x1 = spacing;
    int x2 = x1 + 2 * radius + spacing;
    int x3 = x2 + 2 * radius + spacing;

    _display.drawCircle(x1 + radius, yBottom, radius);
    _display.drawCircle(x2 + radius, yBottom, radius);
    _display.drawCircle(x3 + radius, yBottom, radius);

    // Button labels
    _display.setFont(u8g2_font_5x8_tr);
    int labelY = yBottom + 4;

    _display.drawStr(x1 + radius - (_display.getStrWidth("Down") / 2), labelY, "Down");
    _display.drawStr(x2 + radius - (_display.getStrWidth("Up") / 2), labelY, "Up");
    _display.drawStr(x3 + radius - (_display.getStrWidth("OK") / 2), labelY, "OK");

    _display.sendBuffer();
}


void DisplayManager::setTargetTemp(float temp) {
    _targetTemp = temp;
    _editingTemp = temp; 
}

float DisplayManager::getTargetTemp() {
    return _targetTemp;
}

void DisplayManager::increaseTargetTemp() {
    _editingTemp += 0.1f;
    if (_editingTemp > 40.0f) _editingTemp = 40.0f;
}

void DisplayManager::decreaseTargetTemp() {
    _editingTemp -= 0.1f;
    if (_editingTemp < 0.0f) _editingTemp = 0.0f;
}

bool DisplayManager::confirmSetTemp() {
    _targetTemp = _editingTemp;
    return true;
}
