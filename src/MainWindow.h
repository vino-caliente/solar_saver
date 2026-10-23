#pragma once
#include <windows.h>

class MainWindow {
public:
    // Обработчик сообщений окна (WndProc)
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};