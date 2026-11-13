#include "MainWindow.h"
#include "Renderer.h"
#include "Simulation.h"
#include "Resource.h"
#include <commctrl.h>
#include <cstdio>

// Глобальные хэндлы (только для правой панели и ползунков)
HWND hBtnStart, hBtnPause, hBtnReset;
HWND hSliderPanel, hSliderLamp, hSliderBatt, hSliderRain, hSliderSpeed;
HWND hTextTime, hTextWeather, hTextStats, hTextEnergy;
HWND hLabelRainVal, hLabelSpeedVal;
HWND hLabelRainName, hLabelSpeedName;

HBRUSH hDarkBgBrush = NULL;
HBRUSH hTextBgBrush = NULL;
HFONT hUiFont = NULL;

HWND CreateMySlider(HWND hwnd, HINSTANCE hInst, INT_PTR id, int minVal, int maxVal, int initVal) {
    HWND h = CreateWindowW(L"msctls_trackbar32", L"", WS_CHILD | WS_VISIBLE | TBS_HORZ, 
        0, 0, 100, 30, hwnd, (HMENU)id, hInst, NULL); 
    SendMessage(h, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
    SendMessage(h, TBM_SETPOS, TRUE, initVal);
    return h;
}

HWND CreateLabel(HWND hwnd, HINSTANCE hInst, const wchar_t* text, int x, int y, int w, int h) {
    HWND hLabel = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT, 
        x, y, w, h, hwnd, NULL, hInst, NULL);
    if (hUiFont) SendMessage(hLabel, WM_SETFONT, (WPARAM)hUiFont, TRUE);
    return hLabel;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            InitSimulation();

            hUiFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            hDarkBgBrush = CreateSolidBrush(RGB(30, 30, 30));
            hTextBgBrush = CreateSolidBrush(RGB(40, 40, 40));

            hBtnStart = CreateWindowW(L"BUTTON", L"Старт", WS_CHILD | WS_VISIBLE, 0, 0, 80, 30, hwnd, (HMENU)ID_BTN_START, hInst, NULL);
            hBtnPause = CreateWindowW(L"BUTTON", L"Пауза", WS_CHILD | WS_VISIBLE, 0, 0, 80, 30, hwnd, (HMENU)ID_BTN_PAUSE, hInst, NULL);
            hBtnReset = CreateWindowW(L"BUTTON", L"Сброс", WS_CHILD | WS_VISIBLE, 0, 0, 80, 30, hwnd, (HMENU)ID_BTN_RESET, hInst, NULL);
            
            SendMessage(hBtnStart, WM_SETFONT, (WPARAM)hUiFont, TRUE);
            SendMessage(hBtnPause, WM_SETFONT, (WPARAM)hUiFont, TRUE);
            SendMessage(hBtnReset, WM_SETFONT, (WPARAM)hUiFont, TRUE);

            hSliderPanel = CreateMySlider(hwnd, hInst, ID_SLIDER_PANEL, 0, 500, 300);
            hSliderLamp  = CreateMySlider(hwnd, hInst, ID_SLIDER_LAMP, 0, 100, 45);
            hSliderBatt  = CreateMySlider(hwnd, hInst, ID_SLIDER_BATT, 0, 1000, 300);
            hSliderRain  = CreateMySlider(hwnd, hInst, ID_SLIDER_RAIN, 0, 100, 0);
            hSliderSpeed = CreateMySlider(hwnd, hInst, ID_SLIDER_SPEED, 1, 4, 1);

            hTextTime = CreateLabel(hwnd, hInst, L"Время", 0, 0, 200, 20);
            hTextWeather = CreateLabel(hwnd, hInst, L"Погода", 0, 0, 200, 20);
            hTextStats = CreateLabel(hwnd, hInst, L"Статистика", 0, 0, 200, 80);
            hTextEnergy = CreateLabel(hwnd, hInst, L"Поступление", 0, 0, 200, 20);

            hLabelRainName  = CreateLabel(hwnd, hInst, L"Вероятность дождя:", 0, 0, 200, 20);
            hLabelSpeedName = CreateLabel(hwnd, hInst, L"Ускорение:", 0, 0, 200, 20);
            hLabelRainVal  = CreateLabel(hwnd, hInst, L"0%", 0, 0, 40, 20);
            hLabelSpeedVal = CreateLabel(hwnd, hInst, L"x1", 0, 0, 40, 20);

            SetTimer(hwnd, 1, 16, NULL);
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(240, 240, 240));
            SetBkColor(hdcStatic, RGB(40, 40, 40));
            return (LRESULT)hTextBgBrush;
        }

        case WM_CTLCOLORBTN: {
            return (LRESULT)hDarkBgBrush;
        }

        case WM_SIZE: {
            int W = LOWORD(lParam);
            int H = HIWORD(lParam);
            int simW = W * 0.7;
            int pad = 20;
            int x = simW + pad;
            int availW = W - simW - pad * 2;

            int panelX = simW * 0.1, panelY = H * 0.2;
            MoveWindow(hSliderPanel, panelX, panelY + 75, 120, 30, TRUE);

            int lampX = simW * 0.7, lampY = H * 0.2;
            MoveWindow(hSliderLamp, lampX, lampY + 95, 120, 30, TRUE);

            int battX = simW * 0.7, battY = H * 0.7;
            MoveWindow(hSliderBatt, battX, battY + 85, 120, 30, TRUE);

            int btnW = (availW - 20) / 3;
            MoveWindow(hBtnStart, x, 20, btnW, 30, TRUE);
            MoveWindow(hBtnPause, x + btnW + 10, 20, btnW, 30, TRUE);
            MoveWindow(hBtnReset, x + (btnW + 10) * 2, 20, btnW, 30, TRUE);

            MoveWindow(hTextTime, x, 70, availW, 20, TRUE);
            MoveWindow(hTextWeather, x, 95, availW, 20, TRUE);
            MoveWindow(hTextStats, x, 125, availW, 80, TRUE);
            MoveWindow(hTextEnergy, x, 200, availW, 20, TRUE);

            int y = 240;
            MoveWindow(hLabelRainName, x, y, 200, 20, TRUE);
            MoveWindow(hLabelRainVal, x + availW - 40, y, 40, 20, TRUE);
            MoveWindow(hSliderRain, x, y + 20, availW - 40, 30, TRUE);
            
            y += 60;
            MoveWindow(hLabelSpeedName, x, y, 200, 20, TRUE);
            MoveWindow(hLabelSpeedVal, x + availW - 40, y, 40, 20, TRUE);
            MoveWindow(hSliderSpeed, x, y + 20, availW - 40, 30, TRUE);

            return 0;
        }

        case WM_TIMER: {
            float dt = 0.016f; 
            UpdateSimulation(dt);

            wchar_t buf[128];
            int hour = (int)g_sim.timeOfDay;
            int minute = (int)((g_sim.timeOfDay - hour) * 60);
            swprintf_s(buf, L"Время: %02d:%02d (День %d)", hour, minute, g_sim.dayNumber);
            SetWindowTextW(hTextTime, buf);

            swprintf_s(buf, L"Погода: %s", g_sim.isRainingToday ? L"Дождливо" : L"Солнечно");
            SetWindowTextW(hTextWeather, buf);

            swprintf_s(buf, L"Потреблено: %.2f кВтч\nСэкономлено: %.2f кВтч\nПроцент: %.0f%%", 
                g_sim.totalConsumedKWh, g_sim.savedKWh, g_sim.savingPercent);
            SetWindowTextW(hTextStats, buf);

            swprintf_s(buf, L"Поступление: %.0f%%", g_sim.energyIncomePercent * 100);
            SetWindowTextW(hTextEnergy, buf);

            // Перерисовываем ВСЁ окно (спасибо WS_CLIPCHILDREN, мерцания не будет)
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_HSCROLL: {
            HWND hSlider = (HWND)lParam;
            int pos = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);

            if (hSlider == hSliderPanel) g_sim.panelMaxPowerW = (float)pos;
            else if (hSlider == hSliderLamp) g_sim.lampPowerW = (float)pos;
            else if (hSlider == hSliderBatt) {
                g_sim.batteryCapacityWh = (float)pos;
                if (g_sim.batteryStoredWh > g_sim.batteryCapacityWh) 
                    g_sim.batteryStoredWh = g_sim.batteryCapacityWh;
            }
            else if (hSlider == hSliderRain) {
                g_sim.rainProbability = pos / 100.0f;
                wchar_t buf[16]; swprintf_s(buf, L"%d%%", pos); SetWindowTextW(hLabelRainVal, buf);
            }
            else if (hSlider == hSliderSpeed) {
                if (pos == 1) g_sim.speedMultiplier = 1.0f;
                else if (pos == 2) g_sim.speedMultiplier = 2.0f;
                else if (pos == 3) g_sim.speedMultiplier = 4.0f;
                else if (pos == 4) g_sim.speedMultiplier = 8.0f;
                wchar_t buf[16]; swprintf_s(buf, L"x%.0f", g_sim.speedMultiplier); SetWindowTextW(hLabelSpeedVal, buf);
            }

            // Перерисовка всего окна
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_START: g_sim.isRunning = true; g_sim.isPaused = false; break;
                case ID_BTN_PAUSE: g_sim.isPaused = !g_sim.isPaused; break;
                case ID_BTN_RESET: 
                    ResetSimulation(); 
                    SendMessage(hSliderPanel, TBM_SETPOS, TRUE, (LPARAM)g_sim.panelMaxPowerW);
                    SendMessage(hSliderLamp, TBM_SETPOS, TRUE, (LPARAM)g_sim.lampPowerW);
                    SendMessage(hSliderBatt, TBM_SETPOS, TRUE, (LPARAM)g_sim.batteryCapacityWh);
                    SendMessage(hSliderRain, TBM_SETPOS, TRUE, (LPARAM)(g_sim.rainProbability * 100));
                    SendMessage(hSliderSpeed, TBM_SETPOS, TRUE, 1);
                    break;
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            FillRect(hdcMem, &rc, hDarkBgBrush);
            Renderer::DrawScene(hdcMem, rc.right, rc.bottom, g_sim);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            DeleteObject(hDarkBgBrush);
            DeleteObject(hTextBgBrush);
            DeleteObject(hUiFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}