#include <windows.h>
static void out(const char *s){DWORD n;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0);}
void mainCRTStartup(void){
 const WCHAR *dir=L"C:\\Program Files\\Autodesk\\Inventor 2027\\Bin";
 SetCurrentDirectoryW(dir);SetDllDirectoryW(dir);
 HMODULE mod=LoadLibraryW(L"aurora.dll");char b[256];wsprintfA(b,"Load aurora=%p error=%lu\r\n",mod,GetLastError());out(b);if(!mod)ExitProcess(1);
 void *(*create)(void*,int,UINT)=(void*)GetProcAddress(mod,"?createRenderer@Aurora@@YA?AV?$shared_ptr@VIRenderer@Aurora@@@std@@W4Backend@IRenderer@1@I@Z");
 if(!create){out("Missing renderer export\r\n");ExitProcess(2);}void *renderer[2]={0};out("Creating DirectX ray renderer\r\n");create(renderer,1,1);wsprintfA(b,"Renderer=%p control=%p\r\n",renderer[0],renderer[1]);out(b);ExitProcess(renderer[0]?0:3);
}
