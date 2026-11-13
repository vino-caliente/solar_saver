#pragma once
#include <vector>

struct SimState {
    float panelMaxPowerW = 300.0f;
    float batteryCapacityWh = 300.0f;
    float lampPowerW = 45.0f;
    float rainProbability = 0.5f;
    float speedMultiplier = 1.0f;

    float timeOfDay = 6.0f;
    int dayNumber = 1;
    float batteryStoredWh = 0.0f;

    float currentSunlight = 0.0f;     // 0.0 - 1.0 (чистое солнце, без учета панели)
    float currentPanelPowerW = 0.0f;  // Реальная выработка панели в Вт
    float currentGridPowerW = 0.0f;
    float energyIncomePercent = 0.0f; // Для столбика (равен currentSunlight)
    bool isRainingToday = false;

    float totalConsumedKWh = 0.0f;
    float savedKWh = 0.0f;
    float savingPercent = 0.0f;

    bool isRunning = false;
    bool isPaused = false;
};

extern SimState g_sim;

void InitSimulation();
void ResetSimulation();
void UpdateSimulation(float dt_seconds);