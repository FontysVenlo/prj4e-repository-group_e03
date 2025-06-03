#include <Arduino.h>
#include "LoRaDevice.h"
#include "TemperatureManager.h"
#include <DallasTemperature.h>
#include <OneWire.h>

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
#define ONE_WIRE_BUS 4  // DS18B20 data connected here

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

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

  sensors.begin();

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
}

// --- Task Implementations ---

void TaskTemperatureReader(void *pvParameters) {
  Serial.println("TemperatureReader task started");

  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  Serial.print("DS18B20 devices found: ");
  Serial.println(deviceCount);

  if (deviceCount == 0) {
    Serial.println("No DS18B20 sensors detected! Check wiring.");
  }

  DeviceAddress deviceAddress;
  if (deviceCount > 0) {
    if (sensors.getAddress(deviceAddress, 0)) {
      Serial.print("Sensor address: ");
      for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
      }
      Serial.println();
    } else {
      Serial.println("Failed to read sensor address.");
    }
  }

  for (;;) {
    if (deviceCount > 0) {
      sensors.requestTemperatures();
      float temperature = sensors.getTempCByIndex(0);

      if (temperature == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: Could not read temperature data");
      } else {
        Serial.print("Temperature read: ");
        Serial.print(temperature);
        Serial.println(" °C");

        if (xSemaphoreTake(tempMutex, portMAX_DELAY)) {
          tempManager.updateTemperature(temperature);
          lora.sendCurrentTemperature(tempManager.getCurrentTemperature());
          xSemaphoreGive(tempMutex);
        }
      }
    } else {
      Serial.println("Skipping temperature read — no sensors detected.");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
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
