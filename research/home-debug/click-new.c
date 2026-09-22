#include <windows.h>
static HWND app, browser;
static BOOL CALLBACK child(HWND h,LPARAM x){WCHAR cls[100];GetClassNameW(h,cls,100);if(!lstrcmpW(cls,L"Chrome_RenderWidgetHostHWND"))browser=h;return TRUE;}
static BOOL CALLBACK top(HWND h,LPARAM x){WCHAR t[200];GetWindowTextW(h,t,200);if(!lstrcmpW(t,L"Autodesk Inventor Professional 2027")){app=h;EnumChildWindows(h,child,0);}return TRUE;}
void mainCRTStartup(void){EnumWindows(top,0);if(!browser)ExitProcess(2);PostMessageW(browser,WM_MOUSEMOVE,0,MAKELPARAM(80,262));PostMessageW(browser,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(80,262));PostMessageW(browser,WM_LBUTTONUP,0,MAKELPARAM(80,262));ExitProcess(0);}
