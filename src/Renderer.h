#pragma once
#include <windows.h>
#include "Simulation.h"

class Renderer {
public:
    static void DrawScene(HDC hdc, int width, int height, const SimState& sim);
    
private:
    static void DrawTextAt(HDC hdc, int x, int y, const wchar_t* text, COLORREF color);
};