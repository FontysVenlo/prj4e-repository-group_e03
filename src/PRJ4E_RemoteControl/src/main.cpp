#include <Arduino.h>

// Button GPIOs
const int buttonUpPin = 17;
const int buttonDownPin = 16;

// Shared value
volatile int value = 0;

// Mutex
SemaphoreHandle_t valueMutex;

// Debounce config
const unsigned long debounceDelay = 50;

// Task handles (optional)
TaskHandle_t TaskButtonUpHandle = NULL;
TaskHandle_t TaskButtonDownHandle = NULL;

// --- Task: Increase Value ---
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
        if (xSemaphoreTake(valueMutex, portMAX_DELAY)) {
          value++;
          Serial.print("Value: ");
          Serial.println(value);
          xSemaphoreGive(valueMutex);
        }
        pressed = true;
      } else if (reading == LOW) {
        pressed = false;
      }
    }

    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);  // small delay to reduce CPU usage
  }
}

// --- Task: Decrease Value ---
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
        if (xSemaphoreTake(valueMutex, portMAX_DELAY)) {
          value--;
          Serial.print("Value: ");
          Serial.println(value);
          xSemaphoreGive(valueMutex);
        }
        pressed = true;
      } else if (reading == LOW) {
        pressed = false;
      }
    }

    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);  // startup delay

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);

  valueMutex = xSemaphoreCreateMutex();

  // Start tasks
  xTaskCreatePinnedToCore(
    TaskButtonUp,
    "ButtonUp",
    2048,
    NULL,
    1,
    &TaskButtonUpHandle,
    1
  );

  xTaskCreatePinnedToCore(
    TaskButtonDown,
    "ButtonDown",
    2048,
    NULL,
    1,
    &TaskButtonDownHandle,
    1
  );
}

void loop() {
  // Empty — logic handled in tasks
}
