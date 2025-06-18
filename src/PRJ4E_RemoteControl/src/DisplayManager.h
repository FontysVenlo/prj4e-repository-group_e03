#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

class DisplayManager {
public:
    enum Screen { TEMP_SCREEN, MENU_SCREEN, SET_TEMP_SCREEN };

    DisplayManager(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display);
    void init();

    void updateTemperature(float tempC);
    void moveSelection(int direction);
    void tickBlink();
    void updateSetTempScreen(float currentTemp);

    void setTargetTemp(float temp);
    void increaseTargetTemp();
    void decreaseTargetTemp();
    bool confirmSetTemp();

    // Navigation
    void goToMenuScreen();
    void goToTempScreen();
    void goToSetTempScreen();

    // State helpers
    bool isMenuScreen() const;
    bool isSetTempScreen() const;
    int getSelectedIndex() const;

    float getTargetTemp();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C& _display;

    Screen _currentScreen = TEMP_SCREEN;

    float _targetTemp = 0;
    float _editingTemp = 0.0f;
    unsigned long _lastBlinkTime = 0;
    bool _blinkOn = true;

    int _selectedIndex = 0;
    const int _menuItemCount = 3;

    bool _blinkVisible = true;
    unsigned long _lastBlinkToggle = 0;

    void drawStaticUI();
    void drawMenu();
    void drawThermometer(float tempC);
    void drawSetTempUI(float currentTemp);
    
};