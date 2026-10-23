#pragma once
#include <windows.h>
#include "Simulation.h"

class Renderer {
public:
    static void DrawAll(HDC hdc, int width, int height, const SimState& sim);
    
private:
    static void DrawTextAt(HDC hdc, int x, int y, const wchar_t* text, COLORREF color);
    static void DrawSlider(HDC hdc, int x, int y, int width, float percent);
    static void DrawEnergyBar(HDC hdc, int x, int y, int width, int height, float percent);
    static void DrawButton(HDC hdc, int x, int y, int width, const wchar_t* text);
};