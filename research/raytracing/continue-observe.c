#define COBJMACROS
#include <windows.h>
#include <oleauto.h>
static void out(const char *s) { DWORD n; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0); }
static void check(HRESULT h,const char *where) { char b[256]; wsprintfA(b,"%s: 0x%08lx\r\n",where,h); out(b); if(FAILED(h)) ExitProcess(1); }
static VARIANT invoke(IDispatch *obj,WCHAR *name,WORD kind,UINT count,VARIANT *args) {
 if(!obj){out("No active view; stop.\r\n");ExitProcess(2);} DISPID id; VARIANT result; VariantInit(&result); check(IDispatch_GetIDsOfNames(obj,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"get member");
 DISPPARAMS p={args,0,count,0}; EXCEPINFO e={0}; UINT bad=0;
 HRESULT hr=IDispatch_Invoke(obj,id,&IID_NULL,LOCALE_USER_DEFAULT,kind,&p,&result,&e,&bad);
 check(hr,"invoke"); return result;
}
void mainCRTStartup(void) {
 CLSID cls; IUnknown *unknown; IDispatch *app;
 check(CoInitialize(0),"CoInitialize"); check(CLSIDFromProgID(L"Inventor.Application",&cls),"CLSID");
 check(GetActiveObject(&cls,0,&unknown),"GetActiveObject");
 check(IUnknown_QueryInterface(unknown,&IID_IDispatch,(void **)&app),"IDispatch");
 VARIANT hw=invoke(app,L"ActiveView",DISPATCH_PROPERTYGET,0,0);
 VARIANT hwopts=invoke(app,L"HardwareOptions",DISPATCH_PROPERTYGET,0,0);
 VARIANT gpu=invoke(hwopts.pdispVal,L"EnableViewportGPURayTracing",DISPATCH_PROPERTYGET,0,0);
 char b[1024];wsprintfA(b,"GPU preference = %d\r\n",gpu.boolVal);out(b);
 WCHAR *pause=L"IsRayTracingPaused";DISPID id,put=DISPID_PROPERTYPUT;
 check(IDispatch_GetIDsOfNames(hw.pdispVal,&IID_NULL,&pause,1,LOCALE_USER_DEFAULT,&id),"pause member");
 VARIANT val;VariantInit(&val);val.vt=VT_BOOL;val.boolVal=VARIANT_FALSE;
 DISPPARAMS dp={&val,&put,1,1};EXCEPINFO e={0};
 DWORD start=GetTickCount();
 check(IDispatch_Invoke(hw.pdispVal,id,&IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYPUT,&dp,0,&e,0),"Continue refinement");
 for(int i=0;i<35;i++){
  VARIANT progress=invoke(hw.pdispVal,L"RayTracingProgress",DISPATCH_PROPERTYGET,0,0),str;VariantInit(&str);
  check(VariantChangeType(&str,&progress,0,VT_BSTR),"format progress");
  VARIANT paused=invoke(hw.pdispVal,L"IsRayTracingPaused",DISPATCH_PROPERTYGET,0,0);
  wsprintfA(b,"elapsed_ms=%lu progress=%S paused=%d\r\n",GetTickCount()-start,str.bstrVal,paused.boolVal);out(b);
  VariantClear(&progress);VariantClear(&str);VariantClear(&paused);Sleep(100);
 }
 ExitProcess(0);}
