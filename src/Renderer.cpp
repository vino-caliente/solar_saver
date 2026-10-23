#include "Renderer.h"
#include <cstdio>

void Renderer::DrawTextAt(HDC hdc, int x, int y, const wchar_t* text, COLORREF color = RGB(0,0,0)) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, x, y, text, (int)wcslen(text));
}

void Renderer::DrawSlider(HDC hdc, int x, int y, int width, float percent) {
    RECT bg = { x, y, x + width, y + 6 };
    HBRUSH hBg = CreateSolidBrush(RGB(200, 200, 200));
    FillRect(hdc, &bg, hBg);
    DeleteObject(hBg);
    
    RECT fill = { x, y, x + (int)(width * percent), y + 6 };
    HBRUSH hFill = CreateSolidBrush(RGB(59, 130, 246));
    FillRect(hdc, &fill, hFill);
    DeleteObject(hFill);

    int knobX = x + (int)(width * percent) - 5;
    HBRUSH hKnob = CreateSolidBrush(RGB(50, 50, 50));
    HBRUSH old = (HBRUSH)SelectObject(hdc, hKnob);
    Ellipse(hdc, knobX, y - 4, knobX + 14, y + 10);
    SelectObject(hdc, old);
    DeleteObject(hKnob);
}

void Renderer::DrawEnergyBar(HDC hdc, int x, int y, int width, int height, float percent) {
    RECT bg = { x, y, x + width, y + height };
    HBRUSH hBg = CreateSolidBrush(RGB(230, 230, 230));
    FillRect(hdc, &bg, hBg);
    DeleteObject(hBg);

    FrameRect(hdc, &bg, (HBRUSH)GetStockObject(GRAY_BRUSH));

    int fillHeight = (int)(height * percent);
    if (fillHeight > 0) {
        RECT fill = { x + 2, y + height - fillHeight, x + width - 2, y + height - 2 };
        HBRUSH hFill = CreateSolidBrush(RGB(251, 191, 36));
        FillRect(hdc, &fill, hFill);
        DeleteObject(hFill);
    }
}

void Renderer::DrawButton(HDC hdc, int x, int y, int w, const wchar_t* text) {
    RECT r = { x, y, x + w, y + 35 };
    HBRUSH hBtn = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &r, hBtn);
    DeleteObject(hBtn);
    FrameRect(hdc, &r, (HBRUSH)GetStockObject(GRAY_BRUSH));
    DrawTextW(hdc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void Renderer::DrawAll(HDC hdc, int W, int H, const SimState& sim) {
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    int simW = W * 0.7;

    // Разделитель
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, simW, 0, NULL); LineTo(hdc, simW, H);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // ЗОНА 1: СИМУЛЯЦИЯ
    int ctrlX = simW * 0.4, ctrlY = H * 0.45, ctrlSize = 80;
    RECT rcCtrl = { ctrlX, ctrlY, ctrlX + ctrlSize, ctrlY + ctrlSize };
    HBRUSH hCtrlBrush = CreateSolidBrush(RGB(230, 230, 230));
    FillRect(hdc, &rcCtrl, hCtrlBrush);
    DeleteObject(hCtrlBrush);
    FrameRect(hdc, &rcCtrl, (HBRUSH)GetStockObject(GRAY_BRUSH));
    DrawTextAt(hdc, ctrlX + 5, ctrlY + 30, L"Контроллер", RGB(0,0,0));

    int panelX = simW * 0.1, panelY = H * 0.2;
    RECT rcPanel = { panelX, panelY, panelX + 120, panelY + 60 };
    HBRUSH hPanelBrush = CreateSolidBrush(RGB(59, 130, 246));
    FillRect(hdc, &rcPanel, hPanelBrush);
    DeleteObject(hPanelBrush);
    DrawTextAt(hdc, panelX, panelY - 25, L"Солнечная батарея", RGB(0,0,0));
    DrawSlider(hdc, panelX, panelY + 75, 100, 0.6f);

    int gridX = simW * 0.1, gridY = H * 0.7;
    RECT rcGrid = { gridX, gridY, gridX + 100, gridY + 70 };
    HBRUSH hGridBrush = CreateSolidBrush(RGB(156, 163, 175));
    FillRect(hdc, &rcGrid, hGridBrush);
    DeleteObject(hGridBrush);
    DrawTextAt(hdc, gridX, gridY - 25, L"Сеть", RGB(0,0,0));

    int lampX = simW * 0.7, lampY = H * 0.2;
    HBRUSH hLampBrush = CreateSolidBrush(RGB(251, 191, 36));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hLampBrush);
    Ellipse(hdc, lampX, lampY, lampX + 80, lampY + 80);
    SelectObject(hdc, oldBrush);
    DeleteObject(hLampBrush);
    DrawTextAt(hdc, lampX, lampY - 25, L"Лампочка (потребитель)", RGB(0,0,0));
    DrawSlider(hdc, lampX, lampY + 95, 100, 0.45f);

    int battX = simW * 0.7, battY = H * 0.7;
    RECT rcBatt = { battX, battY, battX + 100, battY + 70 };
    HBRUSH hBattBrush = CreateSolidBrush(RGB(16, 185, 129));
    FillRect(hdc, &rcBatt, hBattBrush);
    DeleteObject(hBattBrush);
    DrawTextAt(hdc, battX, battY - 25, L"Аккумулятор", RGB(0,0,0));
    DrawSlider(hdc, battX, battY + 85, 100, 0.3f);

    // Провода
    HPEN hWirePen = CreatePen(PS_SOLID, 3, RGB(150, 150, 150));
    HPEN hOldWire = (HPEN)SelectObject(hdc, hWirePen);
    MoveToEx(hdc, panelX + 120, panelY + 30, NULL); LineTo(hdc, ctrlX, ctrlY + 40);
    MoveToEx(hdc, gridX + 100, gridY + 35, NULL); LineTo(hdc, ctrlX, ctrlY + 40);
    MoveToEx(hdc, ctrlX + ctrlSize, ctrlY + 40, NULL); LineTo(hdc, lampX, lampY + 40);
    MoveToEx(hdc, ctrlX + ctrlSize, ctrlY + 40, NULL); LineTo(hdc, battX, battY + 35);
    SelectObject(hdc, hOldWire);
    DeleteObject(hWirePen);

    // ЗОНА 2: ИНФОРМАЦИЯ
    int pad = 20, textX = simW + pad, textY = pad;
    wchar_t buf[64];

    DrawTextAt(hdc, textX, textY, L"ВРЕМЯ И СТАТИСТИКА", RGB(100,100,100));
    textY += 30;

    // Время
    int hour = (int)sim.timeOfDay;
    int minute = (int)((sim.timeOfDay - hour) * 60);
    swprintf_s(buf, L"Время: %02d:%02d", hour, minute);
    DrawTextAt(hdc, textX, textY, buf, RGB(0,0,0)); textY += 25;
    
    DrawTextAt(hdc, textX, textY, L"Время суток: День", RGB(0,0,0)); textY += 35;
    
    DrawTextAt(hdc, textX, textY, L"Всего потреблено:", RGB(100,100,100)); textY += 20;
    swprintf_s(buf, L"%.2f кВтч", sim.totalConsumedKWh);
    DrawTextAt(hdc, textX, textY, buf, RGB(0,0,0)); textY += 25;

    DrawTextAt(hdc, textX, textY, L"Сэкономлено:", RGB(16, 185, 129)); textY += 20;
    swprintf_s(buf, L"%.2f кВтч", sim.savedKWh);
    DrawTextAt(hdc, textX, textY, buf, RGB(0,0,0)); textY += 25;

    DrawTextAt(hdc, textX, textY, L"Процент экономии:", RGB(16, 185, 129)); textY += 20;
    swprintf_s(buf, L"%.0f %%", sim.savingPercent);
    DrawTextAt(hdc, textX, textY, buf, RGB(0,0,0)); textY += 35;

    // Кнопки
    int availableW = W - simW - pad * 2;
    int btnW = (availableW - 20) / 3; 
    DrawButton(hdc, textX, textY, btnW, L"Старт");
    DrawButton(hdc, textX + btnW + 10, textY, btnW, L"Пауза");
    DrawButton(hdc, textX + (btnW + 10) * 2, textY, btnW, L"Сброс");
    textY += 55;

    // Индикатор
    DrawTextAt(hdc, textX, textY, L"Поступление энергии:", RGB(0,0,0)); textY += 25;
    DrawEnergyBar(hdc, textX, textY, 30, 60, sim.energyIncomePercent);
    swprintf_s(buf, L"%.0f %%", sim.energyIncomePercent * 100);
    DrawTextAt(hdc, textX + 40, textY + 25, buf, RGB(100,100,100));
    textY += 75;

    // Ползунок дождя
    DrawTextAt(hdc, textX, textY, L"Вероятность дождя:", RGB(0,0,0)); textY += 25;
    DrawSlider(hdc, textX, textY, availableW - 40, sim.rainProbability);
    swprintf_s(buf, L"%.0f%%", sim.rainProbability * 100);
    DrawTextAt(hdc, textX + availableW - 35, textY - 5, buf, RGB(0,0,0));
    textY += 35;

    // Ползунок ускорения
    DrawTextAt(hdc, textX, textY, L"Ускорение времени:", RGB(0,0,0)); textY += 25;
    DrawSlider(hdc, textX, textY, availableW - 40, sim.speedMultiplier);
    DrawTextAt(hdc, textX + availableW - 35, textY - 5, L"x2", RGB(0,0,0));

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}