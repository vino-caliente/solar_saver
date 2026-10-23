#include "MainWindow.h"
#include "Renderer.h"

// Создаем глобальное состояние
SimState g_sim;

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: 
            InvalidateRect(hwnd, NULL, TRUE); 
            return 0;

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // Очищаем фон
            FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

            // Вызываем всю отрисовку через Renderer
            Renderer::DrawAll(hdc, rc.right, rc.bottom, g_sim);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}