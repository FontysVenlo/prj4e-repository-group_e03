#include <Arduino.h>
#include "LoRaDevice.h"
#include "TemperatureManager.h"

// --- Function Declarations ---
void TaskTemperatureReader(void *pvParameters);
void TaskButtonUp(void *pvParameters);
void TaskButtonDown(void *pvParameters);
void TaskSendTargetTemperature(void *pvParameters);

LoRaDevice lora;
TemperatureManager tempManager;

SemaphoreHandle_t tempMutex;

const int buttonUpPin = 17;
const int buttonDownPin = 16;
const unsigned long debounceDelay = 50;

TaskHandle_t TaskButtonUpHandle = NULL;
TaskHandle_t TaskButtonDownHandle = NULL;
TaskHandle_t TaskTempHandle = NULL;
TaskHandle_t TaskSendHandle = NULL;

void setup() {
  Serial.begin(115200);
  delay(3000);

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);

  tempMutex = xSemaphoreCreateMutex();

  if (!lora.begin(868E6)) {
    Serial.println("LoRa init failed!");
    while (true);
  }
  Serial.println("LoRa init succeeded");

  xTaskCreate(TaskButtonUp, "ButtonUp", 2048, NULL, 1, &TaskButtonUpHandle);
  xTaskCreate(TaskButtonDown, "ButtonDown", 2048, NULL, 1, &TaskButtonDownHandle);
  xTaskCreate(TaskTemperatureReader, "TemperatureReader", 2048, NULL, 1, &TaskTempHandle);
  xTaskCreate(TaskSendTargetTemperature, "SendTargetTemp", 2048, NULL, 1, &TaskSendHandle);
}

void loop() {
  // All logic handled by tasks
}

// --- Task Implementations ---

void TaskTemperatureReader(void *pvParameters) {
  for (;;) {
    float simulatedTemperature = 21.7;
    if (xSemaphoreTake(tempMutex, portMAX_DELAY)) {
      tempManager.updateTemperature(simulatedTemperature);
      lora.sendCurrentTemperature(tempManager.getCurrentTemperature());
      xSemaphoreGive(tempMutex);
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
}

void TaskButtonUp(void *pvParameters) {
  bool lastState = LOW;
  bool pressed = false;
  unsigned long lastDebounceTime = 0;

  for (;;) {
    bool reading = digitalRead(buttonUpPin);
    if (reading != lastState) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading == HIGH && !pressed) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (digitalRead(buttonDownPin) == HIGH) {
          pressed = true;
        } else if (tempManager.isInitialized() && xSemaphoreTake(tempMutex, portMAX_DELAY)) {
          tempManager.incrementTarget();
          xSemaphoreGive(tempMutex);
          pressed = true;
        }
      } else if (reading == LOW) {
        pressed = false;
      }
    }

    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void TaskButtonDown(void *pvParameters) {
  bool lastState = LOW;
  bool pressed = false;
  unsigned long lastDebounceTime = 0;

  for (;;) {
    bool reading = digitalRead(buttonDownPin);
    if (reading != lastState) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading == HIGH && !pressed) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (digitalRead(buttonUpPin) == HIGH) {
          pressed = true;
        } else if (tempManager.isInitialized() && xSemaphoreTake(tempMutex, portMAX_DELAY)) {
          tempManager.decrementTarget();
          xSemaphoreGive(tempMutex);
          pressed = true;
        }
      } else if (reading == LOW) {
        pressed = false;
      }
    }

    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void TaskSendTargetTemperature(void *pvParameters) {
  unsigned long holdStart = 0;
  bool sent = false;

  for (;;) {
    bool upPressed = digitalRead(buttonUpPin) == HIGH;
    bool downPressed = digitalRead(buttonDownPin) == HIGH;

    if (upPressed && downPressed) {
      if (holdStart == 0) {
        holdStart = millis();
      } else if (!sent && millis() - holdStart >= 1000) {
        if (tempManager.isInitialized() && xSemaphoreTake(tempMutex, portMAX_DELAY)) {
          lora.sendTargetTemperature(tempManager.getTargetTemperature());
          xSemaphoreGive(tempMutex);
        }
        sent = true;
      }
    } else {
      holdStart = 0;
      sent = false;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
