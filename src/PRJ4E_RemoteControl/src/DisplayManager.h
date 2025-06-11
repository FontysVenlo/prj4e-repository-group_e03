#pragma once
#include <U8g2lib.h>
#include <DallasTemperature.h>

class DisplayManager {
public:
    enum Screen { TEMP_SCREEN, MENU_SCREEN }; // Add more screens as needed
    
    DisplayManager(U8G2_SSD1306_128X64_NONAME_F_HW_I2C& display);
    void init();
    void toggleScreen(); // Toggles between TEMP and MENU
    void updateTemperature(float tempC);
    void drawMenu();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C& _display;
    Screen _currentScreen = TEMP_SCREEN; // Start with temperature screen
    
    void drawStaticUI();
    void drawThermometer(float tempC);
};