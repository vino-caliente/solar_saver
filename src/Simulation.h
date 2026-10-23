#pragma once
#include <string>

// Значения по умолчанию
struct SimState {
    float timeOfDay = 14.5f; // 14:30
    float energyIncomePercent = 0.6f; // 60% (для столбика)
    float rainProbability = 0.5f; // 50%
    float speedMultiplier = 0.25f; // x2 (для ползунка ускорения)
    
    // Статистика
    float totalConsumedKWh = 1.55f;
    float savedKWh = 1.24f;
    float savingPercent = 80.0f;
};

// Глобальное состояние (пока так, для простоты)
extern SimState g_sim;