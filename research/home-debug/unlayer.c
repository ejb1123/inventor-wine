#include <windows.h>
static HWND target;
static BOOL CALLBACK child(HWND h,LPARAM x){WCHAR cls[100];GetClassNameW(h,cls,100);if(!lstrcmpW(cls,L"Intermediate D3D Window"))target=h;return TRUE;}
static BOOL CALLBACK top(HWND h,LPARAM x){WCHAR t[200];GetWindowTextW(h,t,200);if(!lstrcmpW(t,L"Autodesk Inventor Professional 2027"))EnumChildWindows(h,child,0);return TRUE;}
void mainCRTStartup(void){EnumWindows(top,0);if(!target)ExitProcess(2);LONG style=GetWindowLongW(target,GWL_EXSTYLE);SetWindowLongW(target,GWL_EXSTYLE,style & ~(WS_EX_LAYERED|WS_EX_NOREDIRECTIONBITMAP));SetWindowPos(target,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);RedrawWindow(target,0,0,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);Sleep(25000);SetWindowLongW(target,GWL_EXSTYLE,style);ExitProcess(0);}
