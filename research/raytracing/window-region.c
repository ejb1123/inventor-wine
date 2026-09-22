#include <windows.h>
static BOOL CALLBACK top(HWND h, LPARAM unused) {
    WCHAR title[200]; GetWindowTextW(h,title,200);
    if(lstrcmpW(title,L"Autodesk Inventor Professional 2027"))return TRUE;
    HRGN region=CreateRectRgn(0,0,0,0); RECT box={0},window,client;
    int kind=GetWindowRgn(h,region); GetRgnBox(region,&box);GetWindowRect(h,&window);GetClientRect(h,&client);
    char line[400];DWORD n;
    wsprintfA(line,"hwnd=%p region_kind=%d region=%ld,%ld,%ld,%ld window=%ld,%ld,%ld,%ld client=%ld,%ld,%ld,%ld\r\n",h,kind,box.left,box.top,box.right,box.bottom,window.left,window.top,window.right,window.bottom,client.left,client.top,client.right,client.bottom);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),line,lstrlenA(line),&n,0);
#ifdef CLEAR_REGION
    if(kind==SIMPLEREGION || kind==COMPLEXREGION) {
        DWORD size=GetRegionData(region,0,0); RGNDATA *data=HeapAlloc(GetProcessHeap(),0,size);
        if(data && GetRegionData(region,size,data)) {
            HANDLE f=CreateFileW(L"C:\\ResearchUI\\inventor-window-region-backup.bin",GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
            if(f!=INVALID_HANDLE_VALUE){BOOL saved=WriteFile(f,data,size,&n,0)&&n==size;CloseHandle(f);if(saved){int result=SetWindowRgn(h,0,TRUE);wsprintfA(line,"Clear region result=%d\r\n",result);WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),line,lstrlenA(line),&n,0);}}
        }
        if(data)HeapFree(GetProcessHeap(),0,data);
    }
#endif
    DeleteObject(region);return TRUE;
}
void mainCRTStartup(void){EnumWindows(top,0);ExitProcess(0);}
