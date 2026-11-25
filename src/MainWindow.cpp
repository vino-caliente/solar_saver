#include "MainWindow.h"
#include "Renderer.h"
#include "Simulation.h"
#include "Resource.h"
#include <commctrl.h>

HWND hBtnStart, hBtnPause, hBtnReset;
HWND hSliderPanel, hSliderLamp, hSliderBatt, hSliderRain, hSliderSpeed;
HBRUSH hDarkBgBrush = NULL;
HFONT hUiFont = NULL;

HWND CreateMySlider(HWND hwnd, HINSTANCE hInst, INT_PTR id, int minVal, int maxVal, int initVal) {
    HWND h = CreateWindowW(L"msctls_trackbar32", L"", WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS,
        0, 0, 100, 32, hwnd, (HMENU)id, hInst, NULL);
    SendMessage(h, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
    SendMessage(h, TBM_SETPOS, TRUE, initVal);
    SendMessage(h, WM_SETFONT, (WPARAM)hUiFont, TRUE);
    return h;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        InitSimulation();
        hUiFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        hDarkBgBrush = CreateSolidBrush(RGB(25, 29, 36));

        hBtnStart = CreateWindowW(L"BUTTON", L"▶  Старт", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 38, hwnd, (HMENU)ID_BTN_START, hInst, NULL);
        hBtnPause = CreateWindowW(L"BUTTON", L"Ⅱ  Пауза", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 38, hwnd, (HMENU)ID_BTN_PAUSE, hInst, NULL);
        hBtnReset = CreateWindowW(L"BUTTON", L"↻  Сброс", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 38, hwnd, (HMENU)ID_BTN_RESET, hInst, NULL);
        for (HWND b : {hBtnStart, hBtnPause, hBtnReset}) SendMessage(b, WM_SETFONT, (WPARAM)hUiFont, TRUE);

        hSliderRain  = CreateMySlider(hwnd, hInst, ID_SLIDER_RAIN,  0, 100, 0);
        hSliderSpeed = CreateMySlider(hwnd, hInst, ID_SLIDER_SPEED, 1, 4, 1);
        hSliderPanel = CreateMySlider(hwnd, hInst, ID_SLIDER_PANEL, 0, 500, 300);
        hSliderLamp  = CreateMySlider(hwnd, hInst, ID_SLIDER_LAMP,  0, 100, 45);
        hSliderBatt  = CreateMySlider(hwnd, hInst, ID_SLIDER_BATT,  0, 1000, 300);
        SetTimer(hwnd, 1, 16, NULL);
        return 0;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(25, 29, 36));
        SetTextColor(hdc, RGB(230, 235, 242));
        return (LRESULT)hDarkBgBrush;
    }

    case WM_SIZE: {
        int W = LOWORD(lParam), H = HIWORD(lParam);
        int simW = (int)(W * 0.68f);
        int panelX = simW, pad = 24, x = panelX + pad;
        int panelW = W - panelX, contentW = panelW - pad * 2;
        int buttonGap = 8, btnW = (contentW - buttonGap * 2) / 3;
        int top = 24;
        MoveWindow(hBtnStart, x, top, btnW, 38, TRUE);
        MoveWindow(hBtnPause, x + btnW + buttonGap, top, btnW, 38, TRUE);
        MoveWindow(hBtnReset, x + (btnW + buttonGap) * 2, top, btnW, 38, TRUE);
        int sliderW = contentW;
        // The renderer places labels/value rows at these coordinates; sliders sit directly beneath each row.
        int baseY = 374, step = (H < 760 ? 58 : 68);
        MoveWindow(hSliderRain,  x, baseY, sliderW, 32, TRUE);
        MoveWindow(hSliderSpeed, x, baseY + step, sliderW, 32, TRUE);
        MoveWindow(hSliderPanel, x, baseY + step * 2, sliderW, 32, TRUE);
        MoveWindow(hSliderLamp,  x, baseY + step * 3, sliderW, 32, TRUE);
        MoveWindow(hSliderBatt,  x, baseY + step * 4, sliderW, 32, TRUE);
        return 0;
    }
    case WM_TIMER:
        UpdateSimulation(0.016f);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_HSCROLL: {
        HWND hSlider = (HWND)lParam; int pos = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
        if (hSlider == hSliderPanel) g_sim.panelMaxPowerW = (float)pos;
        else if (hSlider == hSliderLamp) g_sim.lampPowerW = (float)pos;
        else if (hSlider == hSliderBatt) { g_sim.batteryCapacityWh = (float)pos; if (g_sim.batteryStoredWh > g_sim.batteryCapacityWh) g_sim.batteryStoredWh = g_sim.batteryCapacityWh; }
        else if (hSlider == hSliderRain) g_sim.rainProbability = pos / 100.0f;
        else if (hSlider == hSliderSpeed) g_sim.speedMultiplier = (pos == 1 ? 1.0f : pos == 2 ? 2.0f : pos == 3 ? 4.0f : 8.0f);
        InvalidateRect(hwnd, NULL, FALSE); return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_START: g_sim.isRunning = true; g_sim.isPaused = false; break;
        case ID_BTN_PAUSE: g_sim.isPaused = !g_sim.isPaused; break;
        case ID_BTN_RESET:
            ResetSimulation();
            SendMessage(hSliderPanel, TBM_SETPOS, TRUE, (LPARAM)g_sim.panelMaxPowerW);
            SendMessage(hSliderLamp, TBM_SETPOS, TRUE, (LPARAM)g_sim.lampPowerW);
            SendMessage(hSliderBatt, TBM_SETPOS, TRUE, (LPARAM)g_sim.batteryCapacityWh);
            SendMessage(hSliderRain, TBM_SETPOS, TRUE, (LPARAM)(g_sim.rainProbability * 100));
            SendMessage(hSliderSpeed, TBM_SETPOS, TRUE, 1); break;
        }
        InvalidateRect(hwnd, NULL, FALSE); return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps); RECT rc; GetClientRect(hwnd, &rc);
        HDC mem = CreateCompatibleDC(hdc); HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom); HBITMAP old = (HBITMAP)SelectObject(mem, bmp);
        FillRect(mem, &rc, hDarkBgBrush); Renderer::DrawScene(mem, rc.right, rc.bottom, g_sim); BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem); EndPaint(hwnd, &ps); return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1); if (hDarkBgBrush) DeleteObject(hDarkBgBrush); if (hUiFont) DeleteObject(hUiFont); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
