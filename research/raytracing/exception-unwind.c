#include <windows.h>
static void logline(const char*s){HANDLE h=CreateFileW(L"C:\\ResearchLicense\\ray-unwind.log",FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_ALWAYS,0,0);if(h!=INVALID_HANDLE_VALUE){DWORD n;WriteFile(h,s,lstrlenA(s),&n,0);CloseHandle(h);}}
static BOOL readmem(const void *p,void*out,SIZE_T size){SIZE_T n;return ReadProcessMemory(GetCurrentProcess(),p,out,size,&n)&&n==size;}
static LONG CALLBACK handler(EXCEPTION_POINTERS*p){
 if(p->ExceptionRecord->ExceptionCode!=0xe06d7363)return EXCEPTION_CONTINUE_SEARCH;
 CONTEXT c=*p->ContextRecord;HMODULE aurora=GetModuleHandleW(L"aurora.dll");if(!aurora)return EXCEPTION_CONTINUE_SEARCH;
 for(int i=0;i<40 && c.Rip;i++){
  if(c.Rip-(ULONG_PTR)aurora==0xdc142){
   char b[1000],file[MAX_PATH]={0};void*dev=0,*vt=0,*fn=0;HMODULE m=0;
   readmem((void*)(c.R15+0x18),&dev,8);readmem(dev,&vt,8);readmem((char*)vt+0x1f0,&fn,8);
   if(fn)GetModuleHandleExA(6,fn,&m);if(m)GetModuleFileNameA(m,file,MAX_PATH);
   wsprintfA(b,"Aurora frame R15=%p device=%p vtable=%p CreateStateObject=%p %s+%lx\r\n",(void*)c.R15,dev,vt,fn,file,(ULONG)((char*)fn-(char*)m));logline(b);break;
  }
  DWORD64 base=0,frame=0;void*data=0;PRUNTIME_FUNCTION f=RtlLookupFunctionEntry(c.Rip,&base,0);
  if(f)RtlVirtualUnwind(UNW_FLAG_NHANDLER,base,c.Rip,f,&c,&data,&frame,0);
  else {if(!readmem((void*)c.Rsp,&c.Rip,8))break;c.Rsp+=8;}
 }
 return EXCEPTION_CONTINUE_SEARCH;
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,void*p){if(reason==DLL_PROCESS_ATTACH)AddVectoredExceptionHandler(1,handler);return TRUE;}
