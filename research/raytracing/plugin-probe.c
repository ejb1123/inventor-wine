#include <windows.h>
static void out(const char *s){DWORD n;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0);}
void mainCRTStartup(void){
 const WCHAR *dir=L"C:\\Program Files\\Autodesk\\Inventor 2027\\Bin";
 SetCurrentDirectoryW(dir);SetDllDirectoryW(dir);
 LoadLibraryW(L"C:\\Program Files\\Common Files\\Autodesk Shared\\Components\\2027\\2.0.3\\tbb12.dll");
 HMODULE mod=LoadLibraryW(L"usd\\hdAurora.dll");char b[256];wsprintfA(b,"Load hdAurora=%p error=%lu\r\n",mod,GetLastError());out(b);ExitProcess(mod?0:1);}
