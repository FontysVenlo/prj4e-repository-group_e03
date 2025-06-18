#pragma once
#include <vector>

class ValveController {
public:
    ValveController();

    void update(float currentTemp, float targetTemp);
    void setValvePosition(int position); // 0 - 100%
    int getValvePosition();
    void recordTemperature(float temp);

private:
    void openValve(int percent);
    void closeValve(int percent);
    float calculateTrend();
    bool isWithinDeadband(float current, float target);

    int _valvePosition;
    std::vector<float> _tempHistory;

    static constexpr float DEAD_BAND = 0.2f;
    static constexpr int SMALL_STEP = 5;
    static constexpr int MAX_VALVE = 100;
    static constexpr int MIN_VALVE = 0;
    static constexpr size_t MAX_HISTORY = 5;

    static constexpr float MIN_WARMING_RATE = 0.05f;
    static constexpr float MAX_WARMING_RATE = 0.3f;
    static constexpr float MIN_COOLING_RATE = -0.05f;
    static constexpr float MAX_COOLING_RATE = -0.3f;
};
