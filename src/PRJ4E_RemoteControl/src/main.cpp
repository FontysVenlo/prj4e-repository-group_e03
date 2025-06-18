#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>
#include <LoRa.h>
#include "DisplayManager.h"
#include "ValveController.h"
#include "TemperatureManager.h"
#include "LoRaDevice.h"

// Pin setup
#define ONE_WIRE_BUS 13
#define BUTTON_MENU  12
#define BUTTON_UP    17
#define BUTTON_DOWN  16

// Globals
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DisplayManager display(u8g2);
ValveController valveController;
TemperatureManager tempManager;
LoRaDevice loraDevice;

// FreeRTOS tasks
void TaskTemperatureDisplay(void* pvParameters) {
    for (;;) {
        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);
        display.updateTemperature(tempC);
        display.updateSetTempScreen(tempC);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskMenuNavigation(void* pvParameters) {
    bool lastMenu = HIGH, lastUp = HIGH, lastDown = HIGH;
    for (;;) {
        bool menu = digitalRead(BUTTON_MENU);
        bool up = digitalRead(BUTTON_UP);
        bool down = digitalRead(BUTTON_DOWN);

        if (menu == LOW && lastMenu == HIGH) {
            if (display.isMenuScreen()) {
                int index = display.getSelectedIndex();
                if (index == 0) display.goToTempScreen();
                else if (index == 1) display.goToSetTempScreen();
                // else Mode option
            } else if (display.isSetTempScreen()) {
                if (display.confirmSetTemp()) display.goToMenuScreen();
            } else {
                display.goToMenuScreen();
            }
        }

        if (up == LOW && lastUp == HIGH) {
            if (display.isMenuScreen()) display.moveSelection(-1);
            else if (display.isSetTempScreen()) display.increaseTargetTemp();
        }

        if (down == LOW && lastDown == HIGH) {
            if (display.isMenuScreen()) display.moveSelection(1);
            else if (display.isSetTempScreen()) display.decreaseTargetTemp();
        }

        lastMenu = menu;
        lastUp = up;
        lastDown = down;

        display.tickBlink();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void TaskValveControl(void* pvParameters) {
    for (;;) {
        sensors.requestTemperatures();
        vTaskDelay(pdMS_TO_TICKS(750));  // Wait for sensor to finish conversion

        float currentTemp = sensors.getTempCByIndex(0);

        if (currentTemp == DEVICE_DISCONNECTED_C) {
            Serial.println("Temperature sensor disconnected! Skipping valve update.");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        float targetTemp = display.getTargetTemp();

        valveController.recordTemperature(currentTemp);
        valveController.update(currentTemp, targetTemp);

        Serial.print("Current Temp: ");
        Serial.print(currentTemp, 1);
        Serial.print(" C, Target: ");
        Serial.print(targetTemp, 1);
        Serial.print(" C, Valve: ");

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void TaskLoRaSend(void *pvParameters) {
  float valvePosition = 0.0f;
  for (;;) {
    // Format payload
    valvePosition = valveController.getValvePosition();
    char payload[16];
    snprintf(payload, sizeof(payload), "VALVE:%.2f", valvePosition);

    LoRa.beginPacket();
    LoRa.print(payload);
    LoRa.endPacket();

    Serial.printf("Sent valve position: %.2f\n", valvePosition);

    vTaskDelay(pdMS_TO_TICKS(10000)); 
  }
}



void setup() {
    Serial.begin(115200);
    sensors.begin();
    display.init();

    pinMode(BUTTON_MENU, INPUT_PULLUP);
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    loraDevice.begin(868E6);


    xTaskCreate(TaskTemperatureDisplay, "TempTask", 4096, NULL, 1, NULL);
    xTaskCreate(TaskMenuNavigation, "MenuNav", 4096, NULL, 2, NULL);
    xTaskCreate(TaskValveControl, "ValveControl", 4096, NULL, 1, NULL);
    xTaskCreate(TaskLoRaSend, "LoRa Send Task", 2048, NULL, 1, NULL);
}

void loop() {}