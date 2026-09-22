#include <windows.h>
static HWND app, browser;
static BOOL CALLBACK child(HWND h,LPARAM x){WCHAR cls[100];GetClassNameW(h,cls,100);if(!lstrcmpW(cls,L"Chrome_WidgetWin_0"))browser=h;return TRUE;}
static BOOL CALLBACK top(HWND h,LPARAM x){WCHAR t[200];GetWindowTextW(h,t,200);if(!lstrcmpW(t,L"Autodesk Inventor Professional 2027")){app=h;EnumChildWindows(h,child,0);}return TRUE;}
void mainCRTStartup(void){EnumWindows(top,0);if(!browser)ExitProcess(2);HWND parent=GetParent(browser);RECT r;GetWindowRect(browser,&r);POINT p={r.left,r.top};ScreenToClient(app,&p);SetParent(browser,app);SetWindowPos(browser,HWND_TOP,p.x,p.y,r.right-r.left,r.bottom-r.top,SWP_SHOWWINDOW);RedrawWindow(browser,0,0,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);Sleep(15000);p.x=r.left;p.y=r.top;ScreenToClient(parent,&p);SetParent(browser,parent);SetWindowPos(browser,HWND_TOP,p.x,p.y,r.right-r.left,r.bottom-r.top,SWP_SHOWWINDOW);ExitProcess(0);}
