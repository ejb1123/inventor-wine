/* Inventor/Wine compatibility: retry its Hydra window-context format without
 * window alpha when Xwayland provides no matching RGBA visual. GPU render
 * targets retain their own formats. Only agp_hydra_bridge's WGL lookup is hooked.
 */
#include <windows.h>
static void logline(const char*s){HANDLE h=CreateFileW(L"C:\\ResearchLicense\\raytracing-compat.log",FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_ALWAYS,0,0);if(h!=INVALID_HANDLE_VALUE){DWORD n;WriteFile(h,s,lstrlenA(s),&n,0);CloseHandle(h);}else{char msg[100];DWORD n;wsprintfA(msg,"Hydra compatibility log open failed: %lu\r\n",GetLastError());WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),msg,lstrlenA(msg),&n,0);}}
static PROC (WINAPI *realProc)(LPCSTR);
static BOOL (WINAPI *realChoose)(HDC,const int*,const FLOAT*,UINT,int*,UINT*);
static BOOL WINAPI choose(HDC dc,const int*a,const FLOAT*b,UINT n,int*f,UINT*c){
 BOOL ok=realChoose(dc,a,b,n,f,c);if(!ok||!c||*c||!a)return ok;
 int attrs[64],i=0;BOOL rgba=FALSE,alpha=FALSE,window=FALSE;
 for(;i<60&&a[i];i+=2){attrs[i]=a[i];attrs[i+1]=a[i+1];if(a[i]==0x2014&&a[i+1]==32)rgba=TRUE;if(a[i]==0x201b&&a[i+1]==8)alpha=TRUE;if(a[i]==0x2001&&a[i+1]==1)window=TRUE;}
 if(i>=60||!rgba||!alpha||!window)return ok;attrs[i]=0;
 for(int j=0;j<i;j+=2){if(attrs[j]==0x2014)attrs[j+1]=24;if(attrs[j]==0x201b)attrs[j+1]=0;}
 BOOL retry=realChoose(dc,attrs,b,n,f,c);if(retry&&*c)logline("Hydra pixel format: RGBA32 unavailable; RGB24 fallback matched.\r\n");return retry;
}
static PROC WINAPI getProc(LPCSTR name){PROC p=realProc(name);if(p&&!lstrcmpA(name,"wglChoosePixelFormatARB")){realChoose=(void*)p;return (PROC)choose;}return p;}
static BOOL patch(HMODULE m){BYTE *base=(BYTE*)m;IMAGE_DOS_HEADER *dos=(void*)base;IMAGE_NT_HEADERS *nt=(void*)(base+dos->e_lfanew);IMAGE_IMPORT_DESCRIPTOR *d=(void*)(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
 for(;d->Name;d++){if(lstrcmpiA((char*)base+d->Name,"opengl32.dll"))continue;IMAGE_THUNK_DATA *names=(void*)(base+d->OriginalFirstThunk),*iat=(void*)(base+d->FirstThunk);for(;names->u1.AddressOfData;names++,iat++){if(IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))continue;IMAGE_IMPORT_BY_NAME *name=(void*)(base+names->u1.AddressOfData);if(lstrcmpA((char*)name->Name,"wglGetProcAddress"))continue;if((void*)iat->u1.Function!=(void*)GetProcAddress(GetModuleHandleW(L"opengl32.dll"),"wglGetProcAddress")){logline("Hydra WGL import already redirected; preserving existing hook.\r\n");return TRUE;}DWORD old;if(VirtualProtect(&iat->u1.Function,sizeof(void*),PAGE_READWRITE,&old)){realProc=(void*)iat->u1.Function;iat->u1.Function=(ULONG_PTR)getProc;logline("Hydra WGL compatibility hook installed.\r\n");VirtualProtect(&iat->u1.Function,sizeof(void*),old,&old);return TRUE;}}}
 return FALSE;
}
/* Hydra is loaded lazily when a document needs it. The user may stay on Home
 * for much longer than two minutes; keep waiting for the process lifetime. */
static DWORD WINAPI worker(void*p){logline("Hydra hook worker started.\r\n");for(;;){HMODULE m=GetModuleHandleW(L"agp_hydra_bridge.dll");if(m&&patch(m))return 0;Sleep(100);}}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,void*p){if(reason==DLL_PROCESS_ATTACH){logline("Hydra compatibility DLL attached.\r\n");DisableThreadLibraryCalls(h);HANDLE t=CreateThread(0,0,worker,0,0,0);if(t)CloseHandle(t);else logline("Hydra hook worker creation failed.\r\n");}return TRUE;}
