#include "Simulation.h"
#include <cmath>
#include <cstdlib>

SimState g_sim;

float GetSunlight(float time) {
    if (time < 5.0f || time > 19.0f) return 0.0f;
    if (time >= 7.0f && time <= 17.0f) return 1.0f;
    if (time > 5.0f && time < 7.0f) return (time - 5.0f) / 2.0f;
    if (time > 17.0f && time < 19.0f) return 1.0f - (time - 17.0f) / 2.0f;
    return 0.0f;
}

bool GenerateTodayWeather() {
    float r = (float)rand() / (float)RAND_MAX;
    return r < g_sim.rainProbability;
}

void ResetSimulation() {
    g_sim.timeOfDay = 6.0f;
    g_sim.dayNumber = 1;
    g_sim.batteryStoredWh = 0.0f;
    g_sim.totalConsumedKWh = 0.0f;
    g_sim.savedKWh = 0.0f;
    g_sim.savingPercent = 0.0f;
    g_sim.isRunning = false;
    g_sim.isPaused = false;
    
    g_sim.isRainingToday = GenerateTodayWeather();
}

void InitSimulation() {
    ResetSimulation();
}

void UpdateSimulation(float dt_real_seconds) {
    if (!g_sim.isRunning || g_sim.isPaused) return;

    float dt_game_hours = dt_real_seconds * g_sim.speedMultiplier * 0.2f;
    g_sim.timeOfDay += dt_game_hours;

    if (g_sim.timeOfDay >= 24.0f) {
        g_sim.timeOfDay -= 24.0f;
        g_sim.dayNumber++;
        g_sim.isRainingToday = GenerateTodayWeather();
    }

    // 1. Считаем чистое солнце (без учета мощности панели)
    float sunIntensity = GetSunlight(g_sim.timeOfDay);
    if (g_sim.isRainingToday) {
        sunIntensity *= 0.2f; // В дождь солнца в 5 раз меньше
    }
    g_sim.currentSunlight = sunIntensity;
    g_sim.energyIncomePercent = sunIntensity; // Для столбика и фона

    // 2. Считаем выработку панели (зависит от солнца и мощности панели)
    g_sim.currentPanelPowerW = g_sim.panelMaxPowerW * sunIntensity;

    // 3. Распределение энергии
    float dt_hours = dt_game_hours; 
    float loadEnergyWh = g_sim.lampPowerW * dt_hours;
    float panelEnergyWh = g_sim.currentPanelPowerW * dt_hours;
    float balanceWh = panelEnergyWh - loadEnergyWh;

    if (balanceWh > 0) {
        float freeSpace = g_sim.batteryCapacityWh - g_sim.batteryStoredWh;
        float chargeAmount = (balanceWh > freeSpace) ? freeSpace : balanceWh;
        g_sim.batteryStoredWh += chargeAmount;
        g_sim.currentGridPowerW = 0.0f;
    } else {
        float deficitWh = -balanceWh;
        float fromBattery = (deficitWh > g_sim.batteryStoredWh) ? g_sim.batteryStoredWh : deficitWh;
        g_sim.batteryStoredWh -= fromBattery;
        float fromGridWh = deficitWh - fromBattery;
        g_sim.currentGridPowerW = fromGridWh / dt_hours;
    }

    // 4. Статистика
    g_sim.totalConsumedKWh += loadEnergyWh / 1000.0f;
    float gridEnergyWh = g_sim.currentGridPowerW * dt_hours;
    g_sim.savedKWh += (loadEnergyWh - gridEnergyWh) / 1000.0f;

    if (g_sim.totalConsumedKWh > 0.0001f) {
        g_sim.savingPercent = (g_sim.savedKWh / g_sim.totalConsumedKWh) * 100.0f;
    } else {
        g_sim.savingPercent = 0.0f;
    }

    if (g_sim.batteryStoredWh < 0) g_sim.batteryStoredWh = 0;
    if (g_sim.batteryStoredWh > g_sim.batteryCapacityWh) g_sim.batteryStoredWh = g_sim.batteryCapacityWh;
}