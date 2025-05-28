#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>

class TemperatureManager {
  public:
    TemperatureManager();
    void updateTemperature(float tempF);
    int getCurrentTemperature();
    int getTargetTemperature();
    void incrementTarget();
    void decrementTarget();
    bool isInitialized();
  private:
    int currentTemperature;
    int targetTemperature;
    bool initialized;
};

#endif