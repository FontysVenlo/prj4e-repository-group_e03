#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// OLED display size and reset pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Task handle (optional, if you want to manage the task)
TaskHandle_t timeDisplayTaskHandle = NULL;

// Task function to display time
void timeDisplayTask(void *parameter) {
  struct tm timeinfo;
  while (true) {
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to get time");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.println("Failed to get time");
      display.display();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }

    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.println(timeStr);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(timeStr);
    display.display();

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 second
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C (ESP32 default SDA=21, SCL=22)
  Wire.begin(21, 22);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Connecting to WiFi...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi connected!");
  display.display();

  // Configure time (NTP servers)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Set timezone for Venlo (CET/CEST)
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();

  // Create FreeRTOS task for time display
  xTaskCreate(
    timeDisplayTask,     // Task function
    "Time Display",      // Task name
    4096,               // Stack size in bytes
    NULL,               // Parameter passed to task
    1,                  // Priority (1 is low, increase if needed)
    &timeDisplayTaskHandle  // Task handle (optional)
  );
}

void loop() {
  // Nothing here, task handles display update
}
