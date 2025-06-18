#ifndef LORA_DEVICE_H
#define LORA_DEVICE_H

#include <Arduino.h>
#include <LoRa.h>

class LoRaDevice {
  public:
    LoRaDevice();
    bool begin(long frequency);
};

#endif