#define COBJMACROS
#include <windows.h>
#include <oleauto.h>
static void out(const char *s) { DWORD n; WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlenA(s),&n,0); }
static void check(HRESULT h,const char *where) { char b[256]; wsprintfA(b,"%s: 0x%08lx\r\n",where,h); out(b); if(FAILED(h)) ExitProcess(1); }
static VARIANT invoke(IDispatch *obj,WCHAR *name,WORD kind,UINT count,VARIANT *args) {
 DISPID id; VARIANT result; VariantInit(&result); check(IDispatch_GetIDsOfNames(obj,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"get member");
 DISPPARAMS p={args,0,count,0}; EXCEPINFO e={0}; UINT bad=0;
 HRESULT hr=IDispatch_Invoke(obj,id,&IID_NULL,LOCALE_USER_DEFAULT,kind,&p,&result,&e,&bad);
 check(hr,"invoke"); return result;
}
void mainCRTStartup(void) {
 CLSID cls; IUnknown *unknown; IDispatch *app;
 check(CoInitialize(0),"CoInitialize"); check(CLSIDFromProgID(L"Inventor.Application",&cls),"CLSID");
 check(GetActiveObject(&cls,0,&unknown),"GetActiveObject");
 check(IUnknown_QueryInterface(unknown,&IID_IDispatch,(void **)&app),"IDispatch");
 VARIANT docs=invoke(app,L"Documents",DISPATCH_PROPERTYGET,0,0); VARIANT count=invoke(docs.pdispVal,L"Count",DISPATCH_PROPERTYGET,0,0); char msg[100];wsprintfA(msg,"Open documents: %ld\r\n",count.lVal);out(msg); 
 VARIANT hw=invoke(app,L"HardwareOptions",DISPATCH_PROPERTYGET,0,0);
 WCHAR *name=L"EnableViewportGPURayTracing"; DISPID id,put=DISPID_PROPERTYPUT; VARIANT v; VariantInit(&v); v.vt=VT_BOOL; v.boolVal=VARIANT_TRUE; check(IDispatch_GetIDsOfNames(hw.pdispVal,&IID_NULL,&name,1,LOCALE_USER_DEFAULT,&id),"driver property"); DISPPARAMS dp={&v,&put,1,1}; EXCEPINFO e={0}; HRESULT hr=IDispatch_Invoke(hw.pdispVal,id,&IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYPUT,&dp,0,&e,0);if(e.pfnDeferredFillIn)e.pfnDeferredFillIn(&e);char buf[1800];wsprintfA(buf,"scode=%08lx source=%S description=%S\r\n",e.scode,e.bstrSource?e.bstrSource:L"",e.bstrDescription?e.bstrDescription:L"");out(buf);check(hr,"enable GPU ray tracing");
 VARIANT r=invoke(hw.pdispVal,L"EnableViewportGPURayTracing",DISPATCH_PROPERTYGET,0,0);char b[100];wsprintfA(b,"EnableViewportGPURayTracing=%d\r\n",r.boolVal);out(b);ExitProcess(0);
}
