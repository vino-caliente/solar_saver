#include <windows.h>
#include "MainWindow.h"
#include "Resource.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SolarSaverWindowClass";

    WNDCLASSW wc = { };
    wc.lpfnWndProc   = MainWindow::WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCEW(ID_ICON_APP));

    if (!RegisterClassW(&wc)) {
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"SolarSaver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}