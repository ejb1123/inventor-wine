#include <windows.h>
static void out(const char *s){DWORD n;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0);}
static BOOL (WINAPI *realSet)(HDC,int,const PIXELFORMATDESCRIPTOR*);
static BOOL WINAPI tracedSet(HDC dc,int f,const PIXELFORMATDESCRIPTOR *p){int old=GetPixelFormat(dc);BOOL r=realSet(dc,f,p);char b[256];wsprintfA(b,"SetPixelFormat old=%d requested=%d result=%d error=%lu\r\n",old,f,r,GetLastError());out(b);return r;}
static PROC (WINAPI *realProc)(LPCSTR);
static BOOL (WINAPI *realChoose)(HDC,const int*,const FLOAT*,UINT,int*,UINT*);
static BOOL WINAPI tracedChoose(HDC dc,const int*a,const FLOAT*b,UINT n,int*f,UINT*c){BOOL r=realChoose(dc,a,b,n,f,c);char msg[256];wsprintfA(msg,"ChooseARB result=%u count=%u fmt=%d error=%lu\r\n",r,*c,*f,GetLastError());out(msg);return r;}
static PROC WINAPI tracedProc(LPCSTR n){PROC r=realProc(n);char b[256];wsprintfA(b,"wglGetProcAddress %s=%p\r\n",n,r);out(b);if(!lstrcmpA(n,"wglChoosePixelFormatARB")){realChoose=(void*)r;return (PROC)tracedChoose;}return r;}
void mainCRTStartup(void){
 const WCHAR *dir=L"C:\\Program Files\\Autodesk\\Inventor 2027\\Bin";
 SetEnvironmentVariableW(L"PATH",L"C:\\Program Files\\Common Files\\Autodesk Shared\\Components\\2027\\2.0.3;C:\\windows\\system32");
 SetCurrentDirectoryW(dir);SetDllDirectoryW(dir);
 LoadLibraryW(L"C:\\Program Files\\Common Files\\Autodesk Shared\\Components\\2027\\2.0.3\\tbb12.dll");
 HMODULE mod=LoadLibraryW(L"agp_hydra_bridge.dll");char b[256];wsprintfA(b,"Load hdAurora=%p error=%lu\r\n",mod,GetLastError());out(b);if(!mod)ExitProcess(1);
 DWORD prot;void **slot=(void**)((char*)mod+0x119010);realSet=*slot;VirtualProtect(slot,8,PAGE_READWRITE,&prot);*slot=tracedSet;VirtualProtect(slot,8,prot,&prot);
 slot=(void**)((char*)mod+0x11a0c8);realProc=*slot;VirtualProtect(slot,8,PAGE_READWRITE,&prot);*slot=tracedProc;VirtualProtect(slot,8,prot,&prot);
 HWND w=CreateWindowExA(0,"STATIC","Eligibility probe",WS_POPUP,0,0,32,32,0,0,0,0);
 BOOL (*check)(HWND)=(void*)((char*)mod+0x10aa60);BOOL supported=check(w);wsprintfA(b,"Actual Hydra Aurora eligibility=%u\r\n",supported);out(b);
 HMODULE glf=GetModuleHandleW(L"usd_glf.dll");const int*(*caps)(void)=(void*)GetProcAddress(glf,"?GetInstance@GlfContextCaps@pxrInternal_adsk_v0_25_8__pxrReserved__@@SAAEBV12@XZ");if(caps){wsprintfA(b,"Glf version=%d\r\n",*caps());out(b);}DestroyWindow(w);ExitProcess(supported?0:2);}
