#include "Renderer.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

static HBITMAP g_sceneBitmap = NULL;
static int g_sceneW = 0, g_sceneH = 0;
// Все режимы потоков энергии заранее загружаются, затем переключаются без повторной загрузки.
static HBITMAP g_flowBitmaps[8] = {};
static int g_flowW[8] = {}, g_flowH[8] = {};
static const wchar_t* g_flowFiles[8] = {
    L"images\\scene_panel.bmp", L"images\\scene_panel_surplus_to_battery.bmp",
    L"images\\scene_panel+battery.bmp", L"images\\scene_panel+battery+grid.bmp",
    L"images\\scene_panel+grid.bmp", L"images\\scene_battery.bmp",
    L"images\\scene_battery+grid.bmp", L"images\\scene_grid.bmp"
};
static void EnsureSceneBitmap() {
    static bool tried = false; if (g_sceneBitmap || tried) return; tried = true;
    g_sceneBitmap = (HBITMAP)LoadImageW(GetModuleHandleW(NULL), L"images\\scene.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (g_sceneBitmap) { BITMAP bm; GetObject(g_sceneBitmap, sizeof(bm), &bm); g_sceneW = bm.bmWidth; g_sceneH = bm.bmHeight; }
    for (int i = 0; i < 8; ++i) {
        g_flowBitmaps[i] = (HBITMAP)LoadImageW(GetModuleHandleW(NULL), g_flowFiles[i], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
        if (g_flowBitmaps[i]) { BITMAP bm; GetObject(g_flowBitmaps[i], sizeof(bm), &bm); g_flowW[i] = bm.bmWidth; g_flowH[i] = bm.bmHeight; }
    }
}
static void Text(HDC hdc, int x, int y, const wchar_t* s, COLORREF c) { SetTextColor(hdc,c); SetBkMode(hdc,TRANSPARENT); TextOutW(hdc,x,y,s,(int)wcslen(s)); }
static HFONT MakeFont(int px, int weight=FW_NORMAL) { return CreateFontW(-px,0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI"); }
static void Fill(HDC hdc, int x,int y,int w,int h,COLORREF c) { HBRUSH b=CreateSolidBrush(c); RECT r{x,y,x+w,y+h}; FillRect(hdc,&r,b); DeleteObject(b); }
// Затемнение всей левой области, включая белые поля вокруг изображения.
static void DarkenArea(HDC hdc, int x, int y, int w, int h, BYTE alpha) {
    if (!alpha || w <= 0 || h <= 0) return;
    BITMAPINFO bi = {}; bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w; bi.bmiHeader.biHeight = -h; bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32; bi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr; HBITMAP overlay = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!overlay || !bits) { if (overlay) DeleteObject(overlay); return; }
    DWORD* pixels = (DWORD*)bits; DWORD value = ((DWORD)alpha << 24);
    for (int i = 0; i < w * h; ++i) pixels[i] = value;
    HDC temp = CreateCompatibleDC(hdc); HBITMAP old = (HBITMAP)SelectObject(temp, overlay);
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(hdc, x, y, w, h, temp, 0, 0, w, h, blend);
    SelectObject(temp, old); DeleteObject(overlay); DeleteDC(temp);
}
static void RoundBox(HDC hdc, int x,int y,int w,int h,int radius,COLORREF c) { HBRUSH b=CreateSolidBrush(c); HPEN p=CreatePen(PS_SOLID,1,c); auto ob=(HBRUSH)SelectObject(hdc,b); auto op=(HPEN)SelectObject(hdc,p); RoundRect(hdc,x,y,x+w,y+h,radius,radius); SelectObject(hdc,ob);SelectObject(hdc,op);DeleteObject(b);DeleteObject(p); }
static void LabelValue(HDC hdc,int x,int y,int w,const wchar_t* label,const wchar_t* value) { Text(hdc,x,y,label,RGB(158,170,185)); Text(hdc,x+w-100,y,value,RGB(232,239,247)); }

void Renderer::DrawTextAt(HDC hdc,int x,int y,const wchar_t* text,COLORREF color){ Text(hdc,x,y,text,color); }
// Определяем изображение потока по текущим мгновенным мощностям и состоянию АКБ.
static int SelectEnergyScene(const SimState& sim) {
    const float panel = sim.currentPanelPowerW;
    const float load = sim.lampPowerW;
    const float battery = sim.batteryStoredWh;
    const float grid = sim.currentGridPowerW;
    const float eps = 0.01f;
    if (panel > eps) {
        if (panel >= load - eps) {
            // Солнечная панель покрывает нагрузку; если есть излишек и место в АКБ — заряд.
            if (panel > load + eps && sim.batteryStoredWh < sim.batteryCapacityWh - eps) return 1;
            return 0;
        }
        if (grid > eps) return (battery > eps) ? 3 : 4;
        return 2;
    }
    if (battery > eps) return (grid > eps) ? 6 : 5;
    return 7;
}

void Renderer::DrawScene(HDC hdc,int W,int H,const SimState& sim) {
    int simW=(int)(W*0.68f), panelW=W-simW;
    EnsureSceneBitmap();
    // Белая сцена. Изображение целиком помещается в область (contain), без обрезки.
    Fill(hdc,0,0,simW,H,RGB(255,255,255));
    HBITMAP activeScene = g_sceneBitmap; int activeW = g_sceneW, activeH = g_sceneH;
    // Сценовые варианты переключаются только во время работающей симуляции.
    if (sim.isRunning && !sim.isPaused) {
        int flowIndex = SelectEnergyScene(sim);
        if (g_flowBitmaps[flowIndex]) { activeScene = g_flowBitmaps[flowIndex]; activeW = g_flowW[flowIndex]; activeH = g_flowH[flowIndex]; }
    }
    if(activeScene && activeW>0 && activeH>0) {
        HDC src=CreateCompatibleDC(hdc); HBITMAP old=(HBITMAP)SelectObject(src,activeScene);
        float scale=std::min((float)simW/activeW,(float)H/activeH); int dw=(int)(activeW*scale), dh=(int)(activeH*scale); int ox=(simW-dw)/2, oy=(H-dh)/2;
        SetStretchBltMode(hdc,HALFTONE); StretchBlt(hdc,ox,oy,dw,dh,src,0,0,activeW,activeH,SRCCOPY); SelectObject(src,old);DeleteDC(src);
    }
    // Освещенность меняет всю левую панель, а не только bitmap.
    BYTE darkness = (BYTE)((1.0f - sim.currentSunlight) * 140.0f);
    DarkenArea(hdc, 0, 0, simW, H, darkness);
    // Right-hand dashboard
    Fill(hdc,simW,0,panelW,H,RGB(25,29,36));
    Fill(hdc,simW,0,1,H,RGB(55,65,78));
    int pad=24,x=simW+pad,contentW=panelW-pad*2;
    HFONT font=MakeFont(16), bold=MakeFont(16,FW_SEMIBOLD), footerFont=MakeFont(16);
    auto old=(HFONT)SelectObject(hdc,font);
    Text(hdc,x,82,L"SOLARSAVER",RGB(235,241,248));
    SelectObject(hdc,bold); Text(hdc,x,108,L"Энергия дома",RGB(235,241,248)); SelectObject(hdc,font);
    // Stat tiles
    int tileY=140,tileH=82,gap=10,tileW=(contentW-gap)/2;
    RoundBox(hdc,x,tileY,tileW,tileH,12,RGB(38,45,55)); RoundBox(hdc,x+tileW+gap,tileY,tileW,tileH,12,RGB(38,45,55));
    wchar_t b[128]; swprintf_s(b,L"%.2f",sim.savedKWh); Text(hdc,x+14,tileY+13,L"СЭКОНОМЛЕНО",RGB(154,170,187)); SelectObject(hdc,bold); Text(hdc,x+14,tileY+39,b,RGB(91,210,164)); SelectObject(hdc,font); Text(hdc,x+tileW+gap+14,tileY+13,L"ПОТРЕБЛЕНО",RGB(154,170,187)); swprintf_s(b,L"%.2f",sim.totalConsumedKWh); SelectObject(hdc,bold);Text(hdc,x+tileW+gap+14,tileY+39,b,RGB(235,241,248));SelectObject(hdc,font);
    int y=tileY+tileH+20;
    swprintf_s(b,L"%02d:%02d   ·   День %d",(int)sim.timeOfDay,(int)((sim.timeOfDay-(int)sim.timeOfDay)*60),sim.dayNumber); Text(hdc,x,y,b,RGB(232,239,247)); y+=27;
    Text(hdc,x,y,sim.isRainingToday?L"Дождливая погода":L"Солнечная погода",RGB(165,177,190)); y+=26;
    swprintf_s(b,L"Солнечная энергия: %.0f%%",sim.energyIncomePercent*100); Text(hdc,x,y,b,RGB(232,239,247)); y+=23;
    swprintf_s(b,L"Экономия: %.0f%%",sim.savingPercent); Text(hdc,x,y,b,RGB(91,210,164)); y+=24;
    Text(hdc,x,y,L"Заряд аккумулятора",RGB(158,170,185)); y+=22;
    const int barW=contentW, barH=16;
    RoundBox(hdc,x,y,barW,barH,8,RGB(48,56,67));
    float chargeRatio = (sim.batteryCapacityWh > 0.0f) ? sim.batteryStoredWh / sim.batteryCapacityWh : 0.0f;
    chargeRatio = std::clamp(chargeRatio,0.0f,1.0f);
    int chargeW=(int)(barW*chargeRatio);
    if(chargeW>0) RoundBox(hdc,x,y,chargeW,barH,8,RGB(91,210,164));
    swprintf_s(b,L"%.0f / %.0f Вт·ч",sim.batteryStoredWh,sim.batteryCapacityWh);
    Text(hdc,x,y+22,b,RGB(232,239,247)); y+=48;
    // Controls captions are placed above their corresponding slider positions in MainWindow.
    int baseY=410, step=(H<760?58:68);
    LabelValue(hdc,x,baseY,contentW,L"Вероятность дождя",(swprintf_s(b,L"%d%%",(int)(sim.rainProbability*100)),b));
    LabelValue(hdc,x,baseY+step,contentW,L"Скорость симуляции",(swprintf_s(b,L"x%.0f",sim.speedMultiplier),b));
    LabelValue(hdc,x,baseY+step*2,contentW,L"Мощность панели",(swprintf_s(b,L"%.0f Вт",sim.panelMaxPowerW),b));
    LabelValue(hdc,x,baseY+step*3,contentW,L"Мощность лампы",(swprintf_s(b,L"%.0f Вт",sim.lampPowerW),b));
    LabelValue(hdc,x,baseY+step*4,contentW,L"Ёмкость аккумулятора",(swprintf_s(b,L"%.0f Вт·ч",sim.batteryCapacityWh),b));
    // Status footer
    SelectObject(hdc,footerFont); Text(hdc,x,H-28,sim.isPaused?L"Симуляция приостановлена":sim.isRunning?L"Симуляция запущена":L"Нажмите «Старт»",RGB(125,139,154));
    SelectObject(hdc,old);DeleteObject(font);DeleteObject(bold);DeleteObject(footerFont);
}
