#pragma once
#include "../global/vars.h"

void inject_winmain(bool replace);

bool WinDXInit(DEVICEINFO* device, DXCONFIG* config, bool createNew);
void WinAppExit();
LRESULT CALLBACK WinAppProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long WinRegisterWindow(HINSTANCE hinstance);
HWND WinCreateWindow(HINSTANCE hinstance, long nCmdShow);
float WinFrameRate();
void WinFreeDX(bool free_dd);
void S_ExitSystem(const char* msg);
void Log(const char* s, ...);		//NOT present in original code

#ifdef DO_LOG
extern FILE* logF;
#endif