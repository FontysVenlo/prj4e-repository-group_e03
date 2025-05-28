#include "TemperatureManager.h"

TemperatureManager::TemperatureManager() : currentTemperature(0), targetTemperature(0), initialized(false) {}

void TemperatureManager::updateTemperature(float tempF) {
  currentTemperature = (int)round(tempF);
  if (!initialized) {
    targetTemperature = currentTemperature;
    initialized = true;
    Serial.println("targetTemperature initialized from currentTemperature.");
  }
  Serial.print("Updated currentTemperature: ");
  Serial.println(currentTemperature);
}

int TemperatureManager::getCurrentTemperature() {
  return currentTemperature;
}

int TemperatureManager::getTargetTemperature() {
  return targetTemperature;
}

void TemperatureManager::incrementTarget() {
  targetTemperature++;
  Serial.print("targetTemperature: ");
  Serial.println(targetTemperature);
}

void TemperatureManager::decrementTarget() {
  targetTemperature--;
  Serial.print("targetTemperature: ");
  Serial.println(targetTemperature);
}

bool TemperatureManager::isInitialized() {
  return initialized;
}