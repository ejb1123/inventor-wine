#include <windows.h>
static HWND app, browser;
static BOOL CALLBACK child(HWND h,LPARAM x){WCHAR cls[100];GetClassNameW(h,cls,100);if(!lstrcmpW(cls,L"Chrome_WidgetWin_1"))browser=h;return TRUE;}
static BOOL CALLBACK top(HWND h,LPARAM x){WCHAR t[200];GetWindowTextW(h,t,200);if(!lstrcmpW(t,L"Autodesk Inventor Professional 2027")){app=h;EnumChildWindows(h,child,0);}return TRUE;}
void mainCRTStartup(void){EnumWindows(top,0);if(!browser)ExitProcess(2);HWND parent=GetParent(browser);LONG style=GetWindowLongW(browser,GWL_STYLE);RECT r;GetWindowRect(browser,&r);SetParent(browser,0);SetWindowLongW(browser,GWL_STYLE,(style&~WS_CHILD)|WS_POPUP|WS_CAPTION);SetWindowPos(browser,HWND_TOP,100,100,1000,700,SWP_SHOWWINDOW|SWP_FRAMECHANGED);Sleep(20000);SetWindowLongW(browser,GWL_STYLE,style);SetParent(browser,parent);POINT p={r.left,r.top};ScreenToClient(parent,&p);SetWindowPos(browser,HWND_TOP,p.x,p.y,r.right-r.left,r.bottom-r.top,SWP_SHOWWINDOW|SWP_FRAMECHANGED);ExitProcess(0);}
