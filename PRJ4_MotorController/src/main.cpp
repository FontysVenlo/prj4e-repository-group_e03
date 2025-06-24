#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <TMCStepper.h>
#include <Adafruit_INA219.h>

// INA219 instance
Adafruit_INA219 ina219;

// Stall detection
volatile bool stallDetected = false;
const float stallCurrentThreshold = 1000.0f;  // in milliamps (adjust as needed)
const int stallSampleLimit = 5;              // number of samples over threshold to count as stall
int stallCounter = 0;


// LoRa Pins
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26

// Stepper Pins
#define STEP_PIN 12
#define DIR_PIN 14
#define EN_PIN 13   // Enable pin for driver, set as needed

// TMC2209 Settings
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

// UART for TMC2209
#define UART_RX_PIN 13
#define UART_TX_PIN 15

// Constants
const int MAX_POSITION = 100;  // Fully pressed position (forward)

// Global state
volatile int currentValvePosition = 0;   // Current step position (0-100)
volatile int targetValvePosition = 0;    // Target step position (0-100)
volatile bool commandReceived = false;   // Flag for first command received

// Initialize Serial2 for TMC2209
TMC2209Stepper driver(&Serial2, R_SENSE, DRIVER_ADDRESS);

void taskMonitorCurrent(void *pvParameters) {
  Serial.println("[Monitor] Current monitor started");

  for (;;) {
    float current_mA = ina219.getCurrent_mA();

    if (current_mA > stallCurrentThreshold) {
      stallCounter++;
    } else {
      stallCounter = 0;
    }

    if (stallCounter >= stallSampleLimit) {
      Serial.printf("[Monitor] Stall detected! Current: %.2f mA\n", current_mA);
      stallDetected = true;
      commandReceived = false;  // stop motor movement
      stallCounter = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(100));  // check every 100ms
  }
}


// --- LoRa receive task ---
void taskLoRaReceive(void *pvParameters) {
  Serial.println("[LoRaRecv] Task started");

  for (;;) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String incoming = "";
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }
      incoming.trim();

      Serial.print("[LoRaRecv] Received: ");
      Serial.println(incoming);

      String incomingUpper = incoming;
      incomingUpper.toUpperCase();
      if (incomingUpper.startsWith("VALVE:")) {
        String valStr = incoming.substring(6);
        float valvePercent = valStr.toFloat();

        if (valvePercent >= 0.0f && valvePercent <= 100.0f) {
          int newTarget = MAX_POSITION - round(valvePercent);

          if (newTarget < 0) newTarget = 0;
          if (newTarget > MAX_POSITION) newTarget = MAX_POSITION;

          targetValvePosition = newTarget;
          stallDetected = false;      
          commandReceived = true;

          Serial.printf("[LoRaRecv] valvePercent=%.2f, targetValvePosition=%d\n", valvePercent, targetValvePosition);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// --- Motor control task ---
void taskMotorControl(void *pvParameters) {
  Serial.println("[MotorTask] Started");

  for (;;) {

    if (!commandReceived || stallDetected) {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }


    if (targetValvePosition != currentValvePosition) {
      int direction = (targetValvePosition > currentValvePosition) ? HIGH : LOW;
      digitalWrite(DIR_PIN, direction);

      // Pulse step pin
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(1000);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(1000);

      if (direction == HIGH) {
        currentValvePosition = min(currentValvePosition + 1, MAX_POSITION);
      } else {
        currentValvePosition = max(currentValvePosition - 1, 0);
      }

      Serial.printf("[MotorTask] Moving step: %d / %d\n", currentValvePosition, targetValvePosition);

      vTaskDelay(pdMS_TO_TICKS(1));
    } else {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup started");

  // Initialize INA219
if (!ina219.begin()) {
  Serial.println("INA219 not found. Check wiring.");
  while (1);  // halt if INA not found
}



  // Setup pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // Enable driver
  digitalWrite(EN_PIN, LOW);  // LOW to enable (depends on your wiring)

  // Setup UART for TMC2209
  Serial2.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  driver.begin();
  driver.rms_current(600);       // Set motor current in mA
  driver.microsteps(16);         // Set microsteps (e.g. 16)

  // Setup LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (!LoRa.begin(868E6)) {
    Serial.println("LoRa init failed.");
    while (1);
  }
  Serial.println("LoRa init OK.");

  // Move motor fully forward 100 steps on startup
  Serial.println("Homing: moving fully forward 100 steps...");
  digitalWrite(DIR_PIN, HIGH);
  for (int i = 0; i < MAX_POSITION; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
  currentValvePosition = MAX_POSITION;
  targetValvePosition = MAX_POSITION;
  commandReceived = false;

  // Start FreeRTOS tasks
  xTaskCreate(taskLoRaReceive, "LoRaRecv", 2048, NULL, 1, NULL);
  xTaskCreate(taskMotorControl, "MotorCtrl", 2048, NULL, 1, NULL);
  xTaskCreate(taskMonitorCurrent, "MonitorCurrent", 2048, NULL, 1, NULL);

  Serial.println("Setup complete");
}

void loop() {
  // Empty, all work done in tasks
}
