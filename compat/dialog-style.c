#include <windows.h>
typedef HRESULT (WINAPI *OpenThemeFn)(LPCWSTR,LPCWSTR,LPCWSTR,HANDLE*,DWORD);
typedef HRESULT (WINAPI *ApplyThemeFn)(HANDLE,void*,HWND);
typedef HRESULT (WINAPI *CloseThemeFn)(HANDLE);
static void regstr(HKEY root,LPCWSTR path,LPCWSTR name,LPCWSTR value) { HKEY k; if(RegCreateKeyExW(root,path,0,0,0,KEY_SET_VALUE,0,&k,0)) ExitProcess(2); RegSetValueExW(k,name,0,REG_SZ,(const BYTE*)value,(lstrlenW(value)+1)*2); RegCloseKey(k); }
void mainCRTStartup(void) {
 const WCHAR *style=L"C:\\windows\\resources\\themes\\research\\balanced.msstyles";
 HANDLE f=CreateFileW(L"C:\\ResearchUI\\theme.ini",GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0); if(f==INVALID_HANDLE_VALUE)ExitProcess(3);
 DWORD size=GetFileSize(f,0),read; void *data=HeapAlloc(GetProcessHeap(),0,size); if(!ReadFile(f,data,size,&read,0)||read!=size)ExitProcess(4);CloseHandle(f);
 HANDLE update=BeginUpdateResourceW(style,FALSE);if(!update)ExitProcess(5);
 if(!UpdateResourceW(update,L"TEXTFILE",L"BLUE_INI",0,data,size)||!EndUpdateResourceW(update,FALSE))ExitProcess(6);
 regstr(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",L"DejaVu Sans (TrueType)",L"DejaVuSans.ttf");
 const WCHAR *subs=L"Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes";
 regstr(HKEY_LOCAL_MACHINE,subs,L"MS Shell Dlg",L"DejaVu Sans"); regstr(HKEY_LOCAL_MACHINE,subs,L"MS Shell Dlg 2",L"DejaVu Sans");regstr(HKEY_LOCAL_MACHINE,subs,L"Segoe UI",L"DejaVu Sans");
 AddFontResourceW(L"C:\\windows\\Fonts\\DejaVuSans.ttf");
 HMODULE ux=LoadLibraryW(L"uxtheme.dll");HANDLE theme=0;
 OpenThemeFn open=(OpenThemeFn)GetProcAddress(ux,(LPCSTR)2);ApplyThemeFn apply=(ApplyThemeFn)GetProcAddress(ux,(LPCSTR)4);CloseThemeFn close=(CloseThemeFn)GetProcAddress(ux,(LPCSTR)3);
 if(!open||!apply||!close||FAILED(open(style,L"Blue",L"NormalSize",&theme,0)))ExitProcess(7);
 if(FAILED(apply(theme,0,0)))ExitProcess(8);
 close(theme);
 UINT type=FE_FONTSMOOTHINGCLEARTYPE; SystemParametersInfoW(SPI_SETFONTSMOOTHING,TRUE,0,SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);SystemParametersInfoW(SPI_SETFONTSMOOTHINGTYPE,0,(void*)(ULONG_PTR)type,SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);
 NONCLIENTMETRICSW metrics={0};metrics.cbSize=sizeof(metrics);
 if(!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0))ExitProcess(9);
 LOGFONTW *fonts[]={&metrics.lfCaptionFont,&metrics.lfSmCaptionFont,&metrics.lfMenuFont,&metrics.lfStatusFont,&metrics.lfMessageFont};
 for(int i=0;i<5;i++){fonts[i]->lfHeight=-14; fonts[i]->lfWidth=0;fonts[i]->lfQuality=CLEARTYPE_QUALITY;lstrcpyW(fonts[i]->lfFaceName,L"DejaVu Sans");}
 if(!SystemParametersInfoW(SPI_SETNONCLIENTMETRICS,sizeof(metrics),&metrics,SPIF_UPDATEINIFILE|SPIF_SENDCHANGE))ExitProcess(10);
 ExitProcess(0);
}
