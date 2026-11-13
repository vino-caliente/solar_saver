#include "Renderer.h"
#include <cstdio>

void Renderer::DrawTextAt(HDC hdc, int x, int y, const wchar_t* text, COLORREF color) {
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, x, y, text, (int)wcslen(text));
}

void Renderer::DrawScene(HDC hdc, int W, int H, const SimState& sim) {
    int simW = W * 0.7;

    // ФОН ЗАВИСИТ ОТ ЧИСТОГО СОЛНЦА (инсоляции)
    float powerRatio = sim.currentSunlight; 
    
    int r = (int)(20 + powerRatio * 115);
    int g = (int)(30 + powerRatio * 176);
    int b = (int)(60 + powerRatio * 175);
    
    HBRUSH hBg = CreateSolidBrush(RGB(r, g, b));
    RECT rcScene = { 0, 0, simW, H };
    FillRect(hdc, &rcScene, hBg);
    DeleteObject(hBg);

    COLORREF textColor = (powerRatio < 0.3f) ? RGB(255, 255, 255) : RGB(30, 30, 30);

    // Разделитель
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, simW, 0, NULL); LineTo(hdc, simW, H);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    wchar_t buf[64]; // Буфер для текста

    // --- ПАНЕЛЬ ---
    int panelX = simW * 0.1, panelY = H * 0.2;
    // Рисуем текст прямо на сцене
    DrawTextAt(hdc, panelX, panelY + 55, L"Мощность панели:", textColor);
    swprintf_s(buf, L"%.0f Вт", sim.panelMaxPowerW);
    DrawTextAt(hdc, panelX + 130, panelY + 60, buf, textColor);

    RECT rcPanel = { panelX, panelY, panelX + 120, panelY + 60 };
    HBRUSH hPanelBrush = CreateSolidBrush(RGB(59, 130, 246));
    FillRect(hdc, &rcPanel, hPanelBrush);
    DeleteObject(hPanelBrush);
    DrawTextAt(hdc, panelX, panelY - 25, L"Солнечная батарея", textColor);

    // --- СЕТЬ ---
    int gridX = simW * 0.1, gridY = H * 0.7;
    RECT rcGrid = { gridX, gridY, gridX + 100, gridY + 70 };
    HBRUSH hGridBrush = CreateSolidBrush(RGB(156, 163, 175));
    FillRect(hdc, &rcGrid, hGridBrush);
    DeleteObject(hGridBrush);
    DrawTextAt(hdc, gridX, gridY - 25, L"Сеть", textColor);

    // --- ЛАМПОЧКА ---
    int lampX = simW * 0.7, lampY = H * 0.2;
    // Рисуем текст прямо на сцене
    DrawTextAt(hdc, lampX, lampY + 75, L"Мощность лампы:", textColor);
    swprintf_s(buf, L"%.0f Вт", sim.lampPowerW);
    DrawTextAt(hdc, lampX + 130, lampY + 80, buf, textColor);

    HBRUSH hLampBrush = CreateSolidBrush(RGB(251, 191, 36));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hLampBrush);
    Ellipse(hdc, lampX, lampY, lampX + 80, lampY + 80);
    SelectObject(hdc, oldBrush);
    DeleteObject(hLampBrush);
    DrawTextAt(hdc, lampX, lampY - 25, L"Лампочка", textColor);

    // --- АКБ ---
    int battX = simW * 0.7, battY = H * 0.7;
    // Рисуем текст прямо на сцене
    DrawTextAt(hdc, battX, battY + 65, L"Ёмкость АКБ:", textColor);
    swprintf_s(buf, L"%.0f Втч", sim.batteryCapacityWh);
    DrawTextAt(hdc, battX + 130, battY + 70, buf, textColor);

    RECT rcBatt = { battX, battY, battX + 100, battY + 70 };
    HBRUSH hBattBrush = CreateSolidBrush(RGB(16, 185, 129));
    FillRect(hdc, &rcBatt, hBattBrush);
    DeleteObject(hBattBrush);
    DrawTextAt(hdc, battX, battY - 25, L"Аккумулятор", textColor);

    // --- КОНТРОЛЛЕР ---
    int ctrlX = simW * 0.4, ctrlY = H * 0.45, ctrlSize = 80;
    RECT rcCtrl = { ctrlX, ctrlY, ctrlX + ctrlSize, ctrlY + ctrlSize };
    HBRUSH hCtrlBrush = CreateSolidBrush(RGB(230, 230, 230));
    FillRect(hdc, &rcCtrl, hCtrlBrush);
    DeleteObject(hCtrlBrush);
    FrameRect(hdc, &rcCtrl, (HBRUSH)GetStockObject(GRAY_BRUSH));
    DrawTextAt(hdc, ctrlX + 5, ctrlY + 30, L"Контроллер", RGB(30, 30, 30));

    // --- ПРОВОДА ---
    HPEN hWirePen = CreatePen(PS_SOLID, 3, RGB(150, 150, 150));
    HPEN hOldWire = (HPEN)SelectObject(hdc, hWirePen);
    MoveToEx(hdc, panelX + 120, panelY + 30, NULL); LineTo(hdc, ctrlX, ctrlY + 40);
    MoveToEx(hdc, gridX + 100, gridY + 35, NULL); LineTo(hdc, ctrlX, ctrlY + 40);
    MoveToEx(hdc, ctrlX + ctrlSize, ctrlY + 40, NULL); LineTo(hdc, lampX, lampY + 40);
    MoveToEx(hdc, ctrlX + ctrlSize, ctrlY + 40, NULL); LineTo(hdc, battX, battY + 35);
    SelectObject(hdc, hOldWire);
    DeleteObject(hWirePen);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}