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
 WCHAR *props[]={L"RayTracing",L"IsRayTracingPaused",L"RayTracingProgress"};
 for(int i=0;i<3;i++){
  VARIANT r=invoke(hw.pdispVal,props[i],DISPATCH_PROPERTYGET,0,0),str;VariantInit(&str);
  check(VariantChangeType(&str,&r,0,VT_BSTR),"format property");
  char b[1024];wsprintfA(b,"%S = %S\r\n",props[i],str.bstrVal);out(b);VariantClear(&str);VariantClear(&r);
 }ExitProcess(0);}
