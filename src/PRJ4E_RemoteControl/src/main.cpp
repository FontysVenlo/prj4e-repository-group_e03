#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// LoRa pin definitions for TTGO LoRa32 V1.0
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DI0 26

// --- Function declarations ---
void updateTemperature(float tempF);
void sendCurrentTemperature();
void sendTargetTemperature();
void TaskTemperatureReader(void *pvParameters);
void TaskButtonUp(void *pvParameters);
void TaskButtonDown(void *pvParameters);
void TaskSendTargetTemperature(void *pvParameters);

// --- Button GPIOs ---
const int buttonUpPin = 17;
const int buttonDownPin = 16;

// --- Shared values ---
volatile int targetTemperature = 0;                 // Desired temperature set by user
volatile int currentTemperature = 0;                // Actual temperature from sensor
volatile bool targetTemperatureInitialized = false; // Flag to initialize only once

// --- Mutex for shared data access ---
SemaphoreHandle_t valueMutex;

// --- Debounce settings ---
const unsigned long debounceDelay = 50; // ms

// --- Task handles ---
TaskHandle_t TaskButtonUpHandle = NULL;
TaskHandle_t TaskButtonDownHandle = NULL;
TaskHandle_t TaskTempHandle = NULL;
TaskHandle_t TaskSendHandle = NULL;

void setup() {
  Serial.begin(115200);
  delay(3000);  // Optional startup delay for easier serial debugging

  // Configure button pins
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);

  // Initialize mutex for shared variable protection
  valueMutex = xSemaphoreCreateMutex();

  // --- Initialize LoRa ---
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);

  if (!LoRa.begin(868E6)) {  // Use 915E6 for US, 433E6 for Asia
    Serial.println("LoRa init failed!");
    while (true); // Stop execution
  }

  Serial.println("LoRa init succeeded");

  // --- Start FreeRTOS tasks ---
  xTaskCreatePinnedToCore(TaskButtonUp, "ButtonUp", 2048, NULL, 1, &TaskButtonUpHandle, 1);
  xTaskCreatePinnedToCore(TaskButtonDown, "ButtonDown", 2048, NULL, 1, &TaskButtonDownHandle, 1);
  xTaskCreatePinnedToCore(TaskTemperatureReader, "TemperatureReader", 2048, NULL, 1, &TaskTempHandle, 1);
  xTaskCreatePinnedToCore(TaskSendTargetTemperature, "SendTargetTemp", 2048, NULL, 1, &TaskSendHandle, 1);
}

void loop() {
  // Not used. All logic is handled by tasks.
}

// --- Called every 30s to update temperature and send it via LoRa ---
void updateTemperature(float tempF) {
  currentTemperature = (int)round(tempF);

  // First-time initialization of target temperature
  if (!targetTemperatureInitialized) {
    targetTemperature = currentTemperature;
    targetTemperatureInitialized = true;
    Serial.println("targetTemperature initialized from currentTemperature.");
  }

  Serial.print("Updated currentTemperature: ");
  Serial.println(currentTemperature);

  sendCurrentTemperature(); // Send via LoRa
}

// --- Send current temperature over LoRa ---
void sendCurrentTemperature() {
  LoRa.beginPacket();
  LoRa.print("current:");
  LoRa.print(currentTemperature);
  LoRa.endPacket();

  Serial.print("[LoRa] Sent currentTemperature: ");
  Serial.println(currentTemperature);
}

// --- Send target temperature over LoRa ---
void sendTargetTemperature() {
  LoRa.beginPacket();
  LoRa.print("target:");
  LoRa.print(targetTemperature);
  LoRa.endPacket();

  Serial.print("[LoRa] Sent targetTemperature: ");
  Serial.println(targetTemperature);
}

// --- Task: Simulate sensor reading every 30 seconds ---
void TaskTemperatureReader(void *pvParameters) {
  for (;;) {
    float simulatedTemperature = 21.7; // Simulated value — replace with real sensor later
    updateTemperature(simulatedTemperature);
    vTaskDelay(30000 / portTICK_PERIOD_MS);  // Wait 30 seconds
  }
}

// --- Task: Handle "Up" button to increase targetTemperature ---
void TaskButtonUp(void *pvParameters) {
  bool lastState = LOW;
  bool pressed = false;
  unsigned long lastDebounceTime = 0;

  for (;;) {
    bool reading = digitalRead(buttonUpPin);

    if (reading != lastState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading == HIGH && !pressed) {
        // Wait briefly to check if both buttons are pressed (combo prevention)
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (digitalRead(buttonDownPin) == HIGH) {
          // Combo detected, skip adjustment
          pressed = true;
        } else if (targetTemperatureInitialized && xSemaphoreTake(valueMutex, portMAX_DELAY)) {
          targetTemperature++;
          Serial.print("targetTemperature: ");
          Serial.println(targetTemperature);
          xSemaphoreGive(valueMutex);
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

// --- Task: Handle "Down" button to decrease targetTemperature ---
void TaskButtonDown(void *pvParameters) {
  bool lastState = LOW;
  bool pressed = false;
  unsigned long lastDebounceTime = 0;

  for (;;) {
    bool reading = digitalRead(buttonDownPin);

    if (reading != lastState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading == HIGH && !pressed) {
        // Wait briefly to check if both buttons are pressed (combo prevention)
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (digitalRead(buttonUpPin) == HIGH) {
          // Combo detected, skip adjustment
          pressed = true;
        } else if (targetTemperatureInitialized && xSemaphoreTake(valueMutex, portMAX_DELAY)) {
          targetTemperature--;
          Serial.print("targetTemperature: ");
          Serial.println(targetTemperature);
          xSemaphoreGive(valueMutex);
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

// --- Task: Detect both buttons held together to send targetTemperature via LoRa ---
void TaskSendTargetTemperature(void *pvParameters) {
  unsigned long holdStart = 0;
  bool sent = false;

  for (;;) {
    bool upPressed = digitalRead(buttonUpPin) == HIGH;
    bool downPressed = digitalRead(buttonDownPin) == HIGH;

    if (upPressed && downPressed) {
      if (holdStart == 0) {
        holdStart = millis(); // Start hold timer
      } else if (!sent && millis() - holdStart >= 1000) {
        // Hold for at least 1 second → send targetTemperature
        if (targetTemperatureInitialized && xSemaphoreTake(valueMutex, portMAX_DELAY)) {
          sendTargetTemperature();
          xSemaphoreGive(valueMutex);
        }
        sent = true;
      }
    } else {
      // Reset when buttons released
      holdStart = 0;
      sent = false;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
