#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>
#include "DisplayManager.h"

// Pins
#define ONE_WIRE_BUS 13
#define BUTTON_MENU  12

// Globals
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
DisplayManager display(u8g2);

// Button Variables (Volatile for ISR)
volatile bool buttonPressed = false;
unsigned long lastDebounceTime = 0;

// FreeRTOS Task Handles
TaskHandle_t TaskTempHandle = NULL;
TaskHandle_t TaskButtonHandle = NULL;

// -------- ISR (Minimal!) --------
void IRAM_ATTR handleButtonMenu() {
  if (millis() - lastDebounceTime > 200) { // Debounce
    buttonPressed = true;
    lastDebounceTime = millis();
  }
}

// -------- Button Task (Processes Presses) --------
void TaskButton(void *pvParameters) {
  for (;;) {
    if (buttonPressed) {
      buttonPressed = false;
      display.toggleScreen(); // Safe to call here (not in ISR)
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
  }
}

// -------- Temperature Task --------
void TaskTemperatureDisplay(void *pvParameters) {
  for (;;) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    display.updateTemperature(tempC);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  display.init();

  // Button Setup
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_MENU), handleButtonMenu, FALLING);

  // Start FreeRTOS Tasks
  xTaskCreate(
    TaskTemperatureDisplay, 
    "TempTask", 
    4096, 
    NULL, 
    1, 
    &TaskTempHandle
  );

  xTaskCreate(
    TaskButton, 
    "ButtonTask", 
    2048, 
    NULL, 
    2,  // Higher priority than TempTask
    &TaskButtonHandle
  );
}

void loop() {} 