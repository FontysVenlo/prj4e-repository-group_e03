#include "ValveController.h"
#include <algorithm>
#include <cmath>

ValveController::ValveController() : _valvePosition(0) {}

void ValveController::setValvePosition(int position) {
    if (position > MAX_VALVE) position = MAX_VALVE;
    else if (position < MIN_VALVE) position = MIN_VALVE;
    _valvePosition = position;
}

int ValveController::getValvePosition() {
    return _valvePosition;
}

void ValveController::recordTemperature(float temp) {
    _tempHistory.push_back(temp);
    if (_tempHistory.size() > MAX_HISTORY) {
        _tempHistory.erase(_tempHistory.begin());
    }
}

bool ValveController::isWithinDeadband(float current, float target) {
    return std::fabs(current - target) <= DEAD_BAND;
}

float ValveController::calculateTrend() {
    if (_tempHistory.size() < 2) return 0.0f;
    float delta = _tempHistory.back() - _tempHistory.front();
    return delta / (_tempHistory.size() - 1);
}

void ValveController::openValve(int percent) {
    setValvePosition(_valvePosition + percent);
}

void ValveController::closeValve(int percent) {
    setValvePosition(_valvePosition - percent);
}

void ValveController::update(float currentTemp, float targetTemp) {
    if (isWithinDeadband(currentTemp, targetTemp)) return;

    float trend = calculateTrend();

    if (targetTemp > currentTemp) {
        if (_valvePosition >= MAX_VALVE) return;
        if (_valvePosition == 0) openValve(SMALL_STEP);
        else if (trend < MIN_WARMING_RATE) openValve(SMALL_STEP);
        else if (trend > MAX_WARMING_RATE) closeValve(SMALL_STEP);
    } else {
        if (_valvePosition <= MIN_VALVE) return;
        if (_valvePosition == MAX_VALVE) closeValve(SMALL_STEP);
        else if (trend > MIN_COOLING_RATE) closeValve(SMALL_STEP);
        else if (trend < MAX_COOLING_RATE) openValve(SMALL_STEP);
    }
}
