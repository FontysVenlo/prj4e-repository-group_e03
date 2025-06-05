#include <TMCStepper.h>

#define DIR_PIN     14
#define STEP_PIN    12
#define DRIVER_ADDRESS 0b00  // Adjust if using multiple drivers

#define stepDelay 1000

#define R_SENSE 0.11f  // TMC2209 typical sense resistor

TMC2209Stepper driver(&Serial2, R_SENSE, DRIVER_ADDRESS);

TaskHandle_t StepperTaskHandle = NULL;
TaskHandle_t StallGuardTaskHandle = NULL;

void TaskStepper(void *pvParameters) {
  for (;;) {
    digitalWrite(DIR_PIN, HIGH);
    delayMicroseconds(100);
    for (int i = 0; i < 1000; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    digitalWrite(DIR_PIN, LOW);
    delayMicroseconds(100);
    for (int i = 0; i < 1000; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void TaskStallGuard(void *pvParameters) {
  for (;;) {
    // Read all the important registers and print
    uint8_t toff = driver.toff();
    uint16_t SG_RESULT = driver.SG_RESULT();
    uint8_t sg_thrs = driver.SGTHRS();
    uint8_t stealthChop_enabled = driver.en_spreadCycle() ? 0 : 1; // stealthChop = !spreadCycle
    uint16_t coolthrs = driver.TCOOLTHRS();
    uint8_t pwm_autoscale = driver.pwm_autoscale();
    uint8_t pwm_reg = driver.pwm_reg();

    Serial.print("TOFF: "); Serial.println(toff);
    Serial.print("SG_RESULT: "); Serial.println(SG_RESULT);
    Serial.print("SG_THRS: "); Serial.println(sg_thrs);
    Serial.print("StealthChop enabled: "); Serial.println(stealthChop_enabled);
    Serial.print("CoolThrs: "); Serial.println(coolthrs);
    Serial.print("PWM Autoscale: "); Serial.println(pwm_autoscale);
    Serial.print("PWM Reg: "); Serial.println(pwm_reg);

    if (SG_RESULT < 50) { 
      Serial.println("Possible stall detected - mechanical resistance?");
    } else {
      Serial.println("Motor running normally.");
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing stepper and stallguard tasks...");

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  Serial2.begin(115200, SERIAL_8N1, 13, 15);  // RX, TX

  driver.begin();
  driver.toff(5);                // Must be >0 to enable motor
  driver.rms_current(600);
  driver.microsteps(16);

  driver.en_spreadCycle(true);  // IMPORTANT: SpreadCycle for StallGuard
  driver.pwm_autoscale(true);
  driver.SGTHRS(10);
  driver.TCOOLTHRS(0xFFFFF);

  // Start the motor stepper task
  xTaskCreate(
    TaskStepper,
    "StepperTask",
    2048,
    NULL,
    1,
    &StepperTaskHandle
  );

  // Start the stall guard monitoring task
  xTaskCreate(
    TaskStallGuard,
    "StallGuardTask",
    2048,
    NULL,
    1,
    &StallGuardTaskHandle
  );
}

void loop() {
  // Empty, multitasking handles everything
}
